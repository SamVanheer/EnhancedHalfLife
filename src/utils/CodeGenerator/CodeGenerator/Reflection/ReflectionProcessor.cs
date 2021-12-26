using ClangSharp;
using CodeGenerator.CodeGen;
using CodeGenerator.Diagnostics;
using CodeGenerator.Processing;
using CodeGenerator.Utility;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace CodeGenerator.Reflection
{
    public sealed class ReflectionProcessor : IProcessor
    {
        private readonly HashSet<string> _visitedTypes = new();
        private readonly HashSet<string> _warnedDuplicateClassesInHeader = new();

        private readonly DiagnosticEngine _diagnosticEngine;
        private readonly GeneratedCode _generatedCode;

        public ReflectionProcessor(DiagnosticEngine diagnosticEngine, GeneratedCode generatedCode)
        {
            _diagnosticEngine = diagnosticEngine;
            _generatedCode = generatedCode;
        }

        public void ProcessTranslationUnit(TranslationUnit translationUnit)
        {
            //Do this here so Visit doesn't have to filter out TU cursors
            foreach (var child in translationUnit.TranslationUnitDecl.CursorChildren)
            {
                Visit(child);
            }
        }

        private void Visit(Cursor cursor)
        {
            //Ignore system header files
            if (cursor.Location.IsInSystemHeader)
            {
                return;
            }

            //Only process types from the main file
            if (!cursor.Location.IsFromMainFile)
            {
                return;
            }

            //Recurse into child scopes first to define all nested types
            switch (cursor)
            {
                case NamespaceDecl:
                case CXXRecordDecl:
                    foreach (var child in cursor.CursorChildren)
                    {
                        Visit(child);
                    }
                    break;
            }

            //Now gather type information
            switch (cursor)
            {
                case CXXRecordDecl record:
                    ProcessRecord(record);
                    break;
            }
        }

        private void ProcessRecord(CXXRecordDecl record)
        {
            if (record.IsInStdNamespace
                || (!record.IsClass && !record.IsStruct)
                || record.IsAnonymousStructOrUnion //TODO treat marked anonymous structs as errors
                || !record.IsCompleteDefinition)
            {
                return;
            }

            var (scope, fqName) = record.GetFullyQualifiedName();

            if (_visitedTypes.Contains(fqName))
            {
                return;
            }

            _visitedTypes.Add(fqName);

            var (classAttributes, hasEHLClassMacro) = record.GetAttributes<ClassAttributes>(_diagnosticEngine, ReflectionConstants.ClassPrefix);

            if (classAttributes is null)
            {
                return;
            }

            //Not marked for processing, ignore
            if (!hasEHLClassMacro)
            {
                //TODO: add this as an attribute
                /*
                if (isEntityClass)
                {
                    _diagnosticEngine.LogError(record, $"Class {fqName} is an entity class but has no EHL_CLASS macro");
                }
                */

                return;
            }

            record.Location.GetSpellingLocation(out var file, out _, out _, out _);

            var headerFileName = Path.GetFullPath(file.Name.CString.NormalizeSlashes());

            if (_generatedCode.Classes.Any(c => c.FileName == headerFileName)
                && !_warnedDuplicateClassesInHeader.Contains(headerFileName))
            {
                _warnedDuplicateClassesInHeader.Add(headerFileName);
                _diagnosticEngine.LogError(record, "Multiple classes declared in header. Move each class to its own file to avoid codegen errors");
            }

            var fields = record.Fields
                .Select(f => (Field: f, AttributeData: f.GetAttributes<FieldAttributes>(_diagnosticEngine, ReflectionConstants.FieldPrefix)))
                .ToList();

            if (fields.Any(f => f.AttributeData.Attributes is null))
            {
                return;
            }

            var baseClass = record.Bases.Count > 0 ? record.Bases[0].Referenced.Name : string.Empty;

            var data = new GeneratedClass(scope, record.Name, headerFileName);
            _generatedCode.Add(data);

            var baseClassName = baseClass.Length > 0 ? baseClass : string.Empty;

            //Emit this only if it's a polymorphic type
            if (record.Destructor?.IsVirtual == true)
            {
                data.Declaration.AddMacroInstantiation($"RTTR_ENABLE({string.Join(", ", record.Bases.Select(b => b.Referenced.Name))})");
            }

            data.Declaration.AddAccessDeclaration(ClassAccess.Public);
            //Add type aliases
            data.Declaration.AddTypeAlias("ThisClass", record.Name);

            if (baseClass.Length > 0)
            {
                data.Declaration.AddTypeAlias("BaseClass", baseClass);
            }

            data.Declaration.AddAccessDeclaration(ClassAccess.Private);
            data.Declaration.AddFriend(ReflectionConstants.InitializationClassDeclaration, ReflectionConstants.InitializationClassTemplatePrefix);

            data.Definition.AddHeaderInclude("array", true);
            data.Definition.AddHeaderInclude(headerFileName);

            //TODO: validate that there is no entityname for abstract & template classes
            if (classAttributes.EntityName is not null)
            {
                if (!ValidateEntityName(record, fqName, classAttributes.EntityName))
                {
                    return;
                }

                foreach (var alias in classAttributes.EntityNameAliases)
                {
                    if (!ValidateEntityName(record, fqName, alias))
                    {
                        return;
                    }
                }
            }
            else if (!classAttributes.EntityNameAliases.IsEmpty)
            {
                _diagnosticEngine.LogError(record, $"Class {fqName} has entity alias names but no entity name");
                return;
            }

            data.Definition.AddStaticAssertDefinition(
                $"std::is_same_v<{fqName}, {fqName}::ThisClass>",
                "Did you forget to add EHL_GENERATED_BODY() or the generated header include?");

            //Filter out fields not marked with EHL_FIELD
            var reflectionFields = fields.Where(f => f.AttributeData.HasAnyAttributes)
                .Select(f => (f.Field, Attributes: f.AttributeData.Attributes!))
                .ToList();

            var reflectionCode = CreateReflectionInitializationClass(record, fqName, classAttributes, reflectionFields);

            data.Definition.AddGlobalVariable(reflectionCode);
        }

        private bool ValidateEntityName(Cursor cursor, string fqName, string name)
        {
            if (string.IsNullOrWhiteSpace(name))
            {
                _diagnosticEngine.LogError(cursor, $"Class {fqName} has empty entity name");
                return false;
            }

            if (Encoding.UTF8.GetByteCount(name) != name.Length)
            {
                _diagnosticEngine.LogError(cursor, $"Class {fqName} has entity name \"{name}\" with non-ASCII characters");
                return false;
            }

            if (char.IsDigit(name[0]))
            {
                _diagnosticEngine.LogError(cursor, $"Class {fqName} has entity name \"{name}\" that starts with digit");
                return false;
            }

            if (Regex.IsMatch(name, "^[^a-zA-Z0-9_]+$"))
            {
                _diagnosticEngine.LogError(cursor, $"Class {fqName} has entity name \"{name}\" that contains invalid characters");
                return false;
            }

            return true;
        }

        private static string CreateReflectionInitializationClass(CXXRecordDecl record, string className, ClassAttributes classAttributes,
            List<(FieldDecl Field, FieldAttributes Attributes)> fields)
        {
            var builder = new StringBuilder();

            builder.AppendLine("template<>");
            builder.AppendFormat("class {0}<{1}>", ReflectionConstants.InitializationClassName, className).AppendLine();
            builder.AppendLine("{");

            builder.AppendLine("public:");
            builder.AppendFormat("\t{0}()", ReflectionConstants.InitializationClassName).AppendLine();
            builder.AppendLine("\t{");

            builder.AppendFormat("\t\trttr::registration::class_<{0}>(\"{0}\")", className).AppendLine();

            {
                var metaDataBuilder = new StringBuilder();

                if (classAttributes.EntityName is not null)
                {
                    AddMetadata(metaDataBuilder, ReflectionConstants.EntityNameKey, classAttributes.EntityName.WrapWithDoubleQuotes());
                }

                if (!classAttributes.EntityNameAliases.IsEmpty)
                {
                    var namesList = string.Join(", ", classAttributes.EntityNameAliases.Select(StringUtils.WrapWithDoubleQuotes));

                    AddMetadata(metaDataBuilder, ReflectionConstants.EntityNameAliasesKey,
                        $"std::array<std::string, {classAttributes.EntityNameAliases.Count}>{{{namesList}}}");
                }

                if (metaDataBuilder.Length > 0)
                {
                    AddMetadata(builder, metaDataBuilder.ToString());
                }
            }

            if (classAttributes.EntityName is not null
                && !record.IsAbstract && !record.IsTemplated)
            {
                builder.AppendLine("\t\t\t.constructor<>()");
            }

            foreach (var (field, attributes) in fields)
            {
                builder.AppendFormat("\t\t\t.property(\"{0}\", &{1}::{0})", field.Name, className).AppendLine();

                var metaDataBuilder = new StringBuilder();

                if (attributes.Persisted)
                {
                    AddMetadata(metaDataBuilder, ReflectionConstants.Persisted, attributes.Persisted);
                }

                if (attributes.IsGlobal)
                {
                    AddMetadata(metaDataBuilder, ReflectionConstants.IsGlobal, attributes.IsGlobal);
                }

                if (attributes.Type is { } type)
                {
                    AddMetadata(metaDataBuilder,
                        ReflectionConstants.Type, $"{ReflectionConstants.PersistenceNamespace}{ReflectionConstants.TypeEnumName}::{type}");
                }

                if (metaDataBuilder.Length > 0)
                {
                    AddMetadata(builder, metaDataBuilder.ToString());
                }
            }

            builder.AppendLine("\t\t\t;");

            builder.AppendLine("\t}");

            builder.AppendLine("};");

            builder.AppendFormat("static {0}<{1}> {1}TypeReflectionInitializer",
                ReflectionConstants.InitializationClassName, className.ConvertTypeNameToIdentifierName());

            return builder.ToString();
        }

        private static StringBuilder AddMetadata(StringBuilder builder, string metadata)
        {
            return builder.AppendFormat("\t\t\t\t({0})", metadata).AppendLine();
        }

        private static StringBuilder AddMetadata(StringBuilder builder, string key, string value)
        {
            if (builder.Length > 0)
            {
                builder.Append(", ");
            }

            return builder.AppendFormat("rttr::metadata(\"{0}\", {1})", key, value);
        }

        private static StringBuilder AddMetadata(StringBuilder builder, string key, bool value)
        {
            return AddMetadata(builder, key, value ? ReflectionConstants.TrueString : ReflectionConstants.FalseString);
        }
    }
}
