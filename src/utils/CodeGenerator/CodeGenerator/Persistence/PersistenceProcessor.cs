﻿using ClangSharp;
using ClangSharp.Interop;
using CodeGenerator.CodeGen;
using CodeGenerator.Diagnostics;
using CodeGenerator.Parsing;
using CodeGenerator.Processing;
using CodeGenerator.Utility;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace CodeGenerator.Persistence
{
    public class PersistenceProcessor : IProcessor
    {
        private const string ClassPrefix = "Class=";
        private const string EntityNameKey = "EntityName";
        private const string EntityNameAliasesKey = "EntityNameAliases";

        private const string FieldPrefix = "Field=";
        private const string TypeKey = "Type";

        private const string EntityBaseClassName = "CBaseEntity";

        private readonly HashSet<string> _visitedTypes = new();
        private readonly HashSet<string> _warnedDuplicateClassesInHeader = new();

        private readonly DiagnosticEngine _diagnosticEngine;
        private readonly GeneratedCode _generatedCode;

        public PersistenceProcessor(DiagnosticEngine diagnosticEngine, GeneratedCode generatedCode)
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
                || record.IsAnonymousStructOrUnion
                || !record.IsCompleteDefinition)
            {
                return;
            }

            var (scope, fqName) = GetFullyQualifiedName(record);

            if (_visitedTypes.Contains(fqName))
            {
                return;
            }

            _visitedTypes.Add(fqName);

            bool isEntityClass = IsEntityClass(record);

            record.Location.GetSpellingLocation(out var file, out _, out _, out _);

            var headerFileName = Path.GetFullPath(file.Name.CString.NormalizeSlashes());

            if (isEntityClass
                && _generatedCode.Classes.Any(c => c.FileName == headerFileName)
                && !_warnedDuplicateClassesInHeader.Contains(headerFileName))
            {
                _warnedDuplicateClassesInHeader.Add(headerFileName);
                LogError(record, "Multiple classes declared in header. Move each class to its own file to avoid codegen errors");
            }

            var (classAttributes, hasEHLClassMacro) = GetAttributes<ClassAttributes>(record, ClassPrefix);

            if (classAttributes is null)
            {
                return;
            }

            //Not marked for processing, ignore
            if (!hasEHLClassMacro)
            {
                if (isEntityClass)
                {
                    LogError(record, $"Class {fqName} is an entity class but has no EHL_CLASS macro");
                }

                return;
            }

            //Not currently allowed
            if (!isEntityClass)
            {
                LogError(record, $"Class {fqName} is marked for codegen but is not an entity class");
                return;
            }

            bool isBaseEntity = record.Name == EntityBaseClassName && scope.Length == 0;

            if (!isBaseEntity && record.Bases.Count == 0)
            {
                LogError(record, $"Class {fqName} is not CBaseEntity yet has no base class!");
                return;
            }

            var fields = record.Fields
                .Select(f => new { Field = f, GetAttributes<FieldAttributes>(f, FieldPrefix).Attributes })
                .ToList();

            if (fields.Any(f => f.Attributes is null))
            {
                return;
            }

            var persistedFields = fields
                .Where(f => f.Attributes?.Persisted == true)
                .Select(f => FormatField(fqName, f.Field, f.Attributes!))
                .ToList();

            if (persistedFields.Any(f => f is null))
            {
                return;
            }

            var baseClass = !isBaseEntity ? record.Bases[0].Referenced.Name : string.Empty;

            var data = new GeneratedClassData(scope, record.Name, headerFileName);
            _generatedCode.Add(data);

            data.AddVisibilityDeclaration("public");
            //Add type aliases
            data.AddTypeAlias("ThisClass", record.Name);

            if (baseClass.Length > 0)
            {
                data.AddTypeAlias("BaseClass", baseClass);
            }

            data.AddHeaderInclude(headerFileName);

            if (classAttributes.EntityName is not null)
            {
                data.AddDefinitionMacro($"LINK_ENTITY_TO_CLASS({classAttributes.EntityName}, {fqName});");

                //TODO: need to validate the name so it doesn't contain invalid characters or that it's empty
                foreach (var alias in classAttributes.EntityNameAliases)
                {
                    data.AddDefinitionMacro($"LINK_ALIAS_ENTITY_TO_CLASS({alias}, {classAttributes.EntityName}, {fqName});");
                }
            }
            else if (!classAttributes.EntityNameAliases.IsEmpty)
            {
                LogError(record, $"Class {fqName} has entity alias names but no entity name");
                return;
            }

            data.AddStaticAssertDefinition(
                $"std::is_same_v<{fqName}, {fqName}::ThisClass>",
                "Did you forget to add EHL_GENERATED_BODY() or the generated header include?");

            //Generate a declaration and definition for persistence if the class has any fields that need it
            if (persistedFields.Count > 0)
            {
                data.AddVisibilityDeclaration("public");
                data.AddMethodDeclaration("bool Save(CSave& save)", isBaseEntity, true);
                data.AddMethodDeclaration("bool Restore(CRestore& restore)", isBaseEntity, true);
                data.AddVariableDeclaration("static TYPEDESCRIPTION m_SaveData[]");

                var saveData = $"TYPEDESCRIPTION {record.Name}::m_SaveData[] =\n{{\n{string.Join(",\n", persistedFields)}\n}};\n";

                data.AddVariableDefinition(saveData);

                data.AddMethodDefinition(FormatSaveRestoreMethod(fqName, isBaseEntity, baseClass, "Save", "CSave", "save", "WriteFields", "WriteEntVars"));
                data.AddMethodDefinition(FormatSaveRestoreMethod(fqName, isBaseEntity, baseClass, "Restore", "CRestore", "restore", "ReadFields", "ReadEntVars"));
            }
        }

        private void LogError(Cursor cursor, string message)
        {
            cursor.Location.GetSpellingLocation(out var file, out var line, out var column, out _);
            _diagnosticEngine.AddDiagnostic(Path.GetFullPath(file.Name.CString.NormalizeSlashes()),
                line, column, message, Severity.Error);
        }

        private static (string Scope, string FullyQualifiedName) GetFullyQualifiedName(NamedDecl decl)
        {
            string scope = string.Empty;

            for (Decl context = (Decl)decl.DeclContext; context?.IsTranslationUnit == false; context = (Decl)context.DeclContext)
            {
                scope = scope.Length == 0 ? context.Spelling : $"{context.Spelling}::{scope}";
            }

            return (scope, scope.Length > 0 ? $"{scope}::{decl.Name}" : decl.Name);
        }

        private static bool IsEntityClass(CXXRecordDecl record)
        {
            if (record.DeclContext is TranslationUnitDecl && record.Name == EntityBaseClassName)
            {
                return true;
            }

            if (record.Bases.Count == 0)
            {
                return false;
            }

            if (record.Bases[0].Referenced is not CXXRecordDecl baseClass)
            {
                return false;
            }

            return IsEntityClass(baseClass);
        }

        private (T? Attributes, bool HasAnyAttributes) GetAttributes<T>(
            NamedDecl decl, string prefix)
            where T : class, new()
        {
            var rawAttributes = decl.Attrs
                    .Where(attr => attr.Spelling.StartsWith(prefix))
                    .ToList();

            var text = string.Join(",", rawAttributes.Select(attr => attr.Spelling.AsSpan()[prefix.Length..].ToString()));

            var attributes = AttributeSerializer.Deserialize<T>(decl, _diagnosticEngine, text);

            return (attributes, rawAttributes.Count > 0);
        }

        private string? DeduceFloatType(FieldDecl field, FieldType? fieldType)
        {
            if (fieldType is null || fieldType == FieldType.Float)
            {
                return "FIELD_FLOAT";
            }
            else if (fieldType == FieldType.Time)
            {
                return "FIELD_TIME";
            }
            else
            {
                LogError(field, $"Unknown float type \"{fieldType}\"");
                return null;
            }
        }

        private string? DeduceVectorType(FieldDecl field, FieldType? fieldType)
        {
            if (fieldType is null || fieldType == FieldType.Vector)
            {
                return "FIELD_VECTOR";
            }
            else if (fieldType == FieldType.Position)
            {
                return "FIELD_POSITION_VECTOR";
            }
            else
            {
                LogError(field, $"Unknown Vector type \"{fieldType}\"");
                return null;
            }
        }

        private string? DeduceStringType(FieldDecl field, FieldType? fieldType)
        {
            if (fieldType is null || fieldType == FieldType.String)
            {
                return "FIELD_STRING";
            }
            else if (fieldType == FieldType.ModelName)
            {
                return "FIELD_MODELNAME";
            }
            else if (fieldType == FieldType.SoundName)
            {
                return "FIELD_SOUNDNAME";
            }
            else
            {
                LogError(field, $"Unknown String type \"{fieldType}\"");
                return null;
            }
        }

        private string? DeduceFieldType(FieldDecl field, FieldAttributes attributes)
        {
            var fieldType = attributes.Type;

            var type = field.Type;

            //Strip array type information so we can get the actual type
            if (type is ArrayType array)
            {
                type = array.ElementType;
            }

            //See if it's a pointer to member function
            if (type.CanonicalType is MemberPointerType && type.PointeeType is FunctionProtoType)
            {
                return "FIELD_FUNCTION";
            }

            switch (type)
            {
                case EnumType:
                    {
                        //For now all enums are 32 bit signed ints
                        return "FIELD_INTEGER";
                    }

                case BuiltinType builtin:
                    {
                        var result = builtin.Kind switch
                        {
                            CXTypeKind.CXType_Int => "FIELD_INTEGER",
                            CXTypeKind.CXType_UInt => "FIELD_INTEGER",
                            CXTypeKind.CXType_Float => DeduceFloatType(field, fieldType),
                            CXTypeKind.CXType_Short => "FIELD_SHORT",
                            CXTypeKind.CXType_UChar => "FIELD_CHARACTER",
                            CXTypeKind.CXType_Char_S => "FIELD_CHARACTER",
                            CXTypeKind.CXType_Char_U => "FIELD_CHARACTER",
                            CXTypeKind.CXType_Bool => "FIELD_BOOLEAN",
                            _ => null
                        };

                        if (result is null)
                        {
                            //TODO: float types will log on invalid types, so this is duplicated for that
                            LogError(field, $"Builtin type \"{builtin.AsString}\" not supported");
                        }

                        return result;
                    }

                default:
                    var typeString = type.AsString;

                    if (typeString.StartsWith("EHandle", StringComparison.InvariantCultureIgnoreCase))
                    {
                        return "FIELD_EHANDLE";
                    }

                    return typeString switch
                    {
                        "Vector" => DeduceVectorType(field, fieldType),
                        "string_t" => DeduceStringType(field, fieldType),
                        "byte" => "FIELD_CHARACTER",
                        _ => "FIELD_CHARACTER" //All other types are treated as byte arrays
                    };
            }
        }

        private static string DetermineFieldSize(FieldDecl field, bool isByteArray)
        {
            if (field.Type is ConstantArrayType array)
            {
                //TODO: if this is a constant array of some non-POD type this will be wrong
                return array.Size.ToString();
            }

            if (isByteArray)
            {
                return $"sizeof({field.Type.AsString})";
            }

            return 1.ToString();
        }

        private string? FormatField(string fqName, FieldDecl field, FieldAttributes attributes)
        {
            var type = DeduceFieldType(field, attributes);

            if (type is null)
            {
                return null;
            }

            var isByteArray = type == "FIELD_CHARACTER";

            return $"\t{{ {type}, \"{field.Name}\", static_cast<int>(offsetof({fqName}, {field.Name})), {DetermineFieldSize(field, isByteArray)}, 0 }}";
        }

        private static string FormatSaveRestoreMethod(string typeName, bool isBaseEntity, string baseClass,
            string methodName, string storageType, string storageParameter, string ioMethodName, string entvarsIOMethodName)
        {
            var saveContents = $"\treturn {storageParameter}.{ioMethodName}(\"{typeName}\", this, m_SaveData, ArraySize(m_SaveData));";

            if (isBaseEntity)
            {
                saveContents = $"\tif (!{storageParameter}.{entvarsIOMethodName}(\"ENTVARS\", pev))\n\t{{\n\t\treturn false;\n\t}}\n\n" + saveContents;
            }
            else
            {
                saveContents = $"\tif (!{baseClass}::{methodName}({storageParameter}))\n\t{{\n\t\treturn false;\n\t}}\n\n" + saveContents;
            }

            var saveMethod = $"bool {typeName}::{methodName}({storageType}& {storageParameter})\n{{\n{saveContents}\n}}\n";

            return saveMethod;
        }
    }
}
