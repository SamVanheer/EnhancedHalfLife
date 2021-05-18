﻿using ClangSharp;
using ClangSharp.Interop;
using CodeGenerator.CodeGen;
using CodeGenerator.Processing;
using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Linq;
using System.Text.RegularExpressions;

namespace CodeGenerator.Persistence
{
    public class PersistenceProcessor : IProcessor
    {
        private const string ClassPrefix = "Class=";
        private const string FieldPrefix = "Field=";
        private const string TypeKey = "Type";

        private const string EntityBaseClassName = "CBaseEntity";

        private readonly HashSet<string> _visitedTypes = new();

        private readonly GeneratedCode _generatedCode;

        public PersistenceProcessor(GeneratedCode generatedCode)
        {
            _generatedCode = generatedCode;
        }

        public void ProcessTranslationUnit(TranslationUnit translationUnit)
        {
            Visit(translationUnit.TranslationUnitDecl);
        }

        private void Visit(Cursor cursor)
        {
            //Ignore system header files
            if (cursor.Location.IsInSystemHeader)
            {
                return;
            }

            //Recurse into child scopes first to define all nested types
            switch (cursor)
            {
                case TranslationUnitDecl:
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

            var classAttributes = GetRelevantAttributes(record, ClassPrefix);

            //Not marked for processing, ignore
            if (classAttributes.IsEmpty)
            {
                return;
            }

            //Generate a declaration and definition for persistence if the class has any fields that need it
            var persistedFields = record.Fields
                .Select(f => new { Field = f, Attributes = GetRelevantAttributes(f, FieldPrefix) })
                .Where(f => f.Attributes.ContainsKey("Persisted"))
                .ToList();

            if (persistedFields.Count > 0)
            {
                bool isBaseEntity = record.Name == EntityBaseClassName && scope.Length == 0;

                var baseClass = !isBaseEntity ? record.Bases[0].Referenced.Name : string.Empty;

                var data = _generatedCode.Get(scope, record.Name);

                record.Location.GetSpellingLocation(out var file, out _, out _, out _);

                data.AddHeaderInclude(file.Name.CString);

                data.AddVisibilityDeclaration("public");
                data.AddMethodDeclaration("bool Save(CSave& save)", isBaseEntity, true);
                data.AddMethodDeclaration("bool Restore(CRestore& restore)", isBaseEntity, true);
                data.AddVariableDeclaration("static TYPEDESCRIPTION m_SaveData[]");

                var savedFields = persistedFields
                    .Select(f => FormatField(fqName, f.Field, f.Attributes));

                var saveData = $"TYPEDESCRIPTION {record.Name}::m_SaveData[] =\n{{\n{string.Join(",\n", savedFields)}\n}};\n";

                data.AddVariableDefinition(saveData);

                data.AddMethodDefinition(FormatSaveRestoreMethod(fqName, isBaseEntity, baseClass, "Save", "CSave", "save", "WriteFields", "WriteEntVars"));
                data.AddMethodDefinition(FormatSaveRestoreMethod(fqName, isBaseEntity, baseClass, "Restore", "CRestore", "restore", "ReadFields", "ReadEntVars"));
            }
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

        private static (string Key, string Value) ConvertAttributeToKeyValue(string attribute)
        {
            //if an attribute has a =, it will be split on that character
            var index = attribute.IndexOf('=');

            if (index != -1)
            {
                var value = attribute[(index + 1)..].Trim();

                //Values that are quoted will be transformed to have no quotes
                if (value.StartsWith('\"') && value.EndsWith('\"'))
                {
                    value = value[1..^1];
                }

                return (attribute.Substring(0, index).Trim(), value);
            }

            return (attribute.Trim(), string.Empty);
        }

        private static ImmutableDictionary<string, string> GetRelevantAttributes(NamedDecl decl, string prefix)
        {
            //Regex from: https://stackoverflow.com/a/3147901
            try
            {
                return decl.Attrs
                    .Where(attr => attr.Spelling.StartsWith(prefix))
                    .SelectMany(attr => Regex.Split(attr.Spelling[prefix.Length..], ",(?=(?:[^']*'[^']*')*[^']*$)"))
                    .Select(ConvertAttributeToKeyValue)
                    .ToImmutableDictionary(attr => attr.Key, attr => attr.Value);
            }
            catch (ArgumentException e)
            {
                var (scope, fqName) = GetFullyQualifiedName(decl);

                throw new ProcessingException($"Duplicate attribute encountered on declaration {fqName}", e);
            }
        }

        private static string DeduceFloatType(string? fieldType)
        {
            if (fieldType is null || fieldType == "Float")
            {
                return "FIELD_FLOAT";
            }
            else if (fieldType == "Time")
            {
                return "FIELD_TIME";
            }
            else
            {
                throw new ProcessingException($"Unknown float type \"{fieldType}\"");
            }
        }

        private static string DeduceVectorType(string? fieldType)
        {
            if (fieldType is null || fieldType == "Vector")
            {
                return "FIELD_VECTOR";
            }
            else if (fieldType == "Position")
            {
                return "FIELD_POSITION_VECTOR";
            }
            else
            {
                throw new ProcessingException($"Unknown Vector type \"{fieldType}\"");
            }
        }

        private static string DeduceStringType(string? fieldType)
        {
            if (fieldType is null || fieldType == "String")
            {
                return "FIELD_STRING";
            }
            else if (fieldType == "ModelName")
            {
                return "FIELD_MODELNAME";
            }
            else if (fieldType == "SoundName")
            {
                return "FIELD_SOUNDNAME";
            }
            else
            {
                throw new ProcessingException($"Unknown String type \"{fieldType}\"");
            }
        }

        private static string DeduceFieldType(FieldDecl field, ImmutableDictionary<string, string> attributes)
        {
            attributes.TryGetValue(TypeKey, out var fieldType);

            /*
            var fieldTypes = GetTypeAttributes(attributes);

            if (fieldTypes.Count > 1)
            {
                throw new ProcessingException("More than one Type attribute was specified");
            }

            var fieldType = fieldTypes.SingleOrDefault();
            */
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
                    return builtin.Kind switch
                    {
                        CXTypeKind.CXType_Int => "FIELD_INTEGER",
                        CXTypeKind.CXType_UInt => "FIELD_INTEGER",
                        CXTypeKind.CXType_Float => DeduceFloatType(fieldType),
                        CXTypeKind.CXType_Short => "FIELD_SHORT",
                        CXTypeKind.CXType_UChar => "FIELD_CHARACTER",
                        CXTypeKind.CXType_Char_S => "FIELD_CHARACTER",
                        CXTypeKind.CXType_Char_U => "FIELD_CHARACTER",
                        CXTypeKind.CXType_Bool => "FIELD_BOOLEAN",
                        _ => throw new ProcessingException($"Builtin type \"{builtin.AsString}\" not supported")
                    };

                default:
                    var typeString = type.AsString;

                    if (typeString.StartsWith("EHandle", StringComparison.InvariantCultureIgnoreCase))
                    {
                        return "FIELD_EHANDLE";
                    }

                    return typeString switch
                    {
                        "Vector" => DeduceVectorType(fieldType),
                        "string_t" => DeduceStringType(fieldType),
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

        private static string FormatField(string fqName, FieldDecl field, ImmutableDictionary<string, string> attributes)
        {
            var type = DeduceFieldType(field, attributes);
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