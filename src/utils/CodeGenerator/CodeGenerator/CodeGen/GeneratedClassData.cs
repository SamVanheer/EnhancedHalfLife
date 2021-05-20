using System.Collections.Generic;
using System.IO;

namespace CodeGenerator.CodeGen
{
    /// <summary>
    /// Stores the generated data for a class
    /// </summary>
    public class GeneratedClassData
    {
        private const string EHLGeneratedBodyMacroName = "EHL_GENERATED_BODY";

        public string Scope { get; }

        public string Name { get; }

        public string FileName { get; }

        public string FullyQualifiedName => Scope.Length > 0 ? $"{Scope}::{Name}" : Name;

        public List<string> GeneratedDeclaration { get; } = new();

        public List<string> GeneratedDefinition { get; } = new();

        public GeneratedClassData(string scope, string name, string fileName)
        {
            Scope = scope;
            Name = name;
            FileName = fileName;
        }

        public void AddVisibilityDeclaration(string visibility)
        {
            visibility += ":";

            GeneratedDeclaration.Add(visibility);
        }

        public void AddMethodDeclaration(string declaration, bool isBase, bool isVirtual = false)
        {
            if (!isBase)
            {
                declaration += " override";
            }
            else if (isVirtual)
            {
                declaration = "virtual " + declaration;
            }

            declaration += ";";

            GeneratedDeclaration.Add(declaration);
        }

        public void AddVariableDeclaration(string declaration)
        {
            declaration += ";";

            GeneratedDeclaration.Add(declaration);
        }

        public void AddAlias(string aliasName, string originalName)
        {
            var alias = $"using {aliasName} = {originalName};";

            GeneratedDeclaration.Add(alias);
        }

        public void AddHeaderInclude(string relativePath)
        {
            GeneratedDefinition.Insert(0, $"#include \"{relativePath}\"");
        }

        public void AddMethodDefinition(string definition)
        {
            GeneratedDefinition.Add(definition);
        }

        public void AddVariableDefinition(string definition)
        {
            GeneratedDefinition.Add(definition);
        }

        public void AddStaticAssertDefinition(string condition, string? message = null)
        {
            var assert = message is not null ? $"static_assert({condition}, \"{message}\");" : $"static_assert({condition});";

            GeneratedDefinition.Add(assert);
        }

        public string GenerateFullDeclaration()
        {
            //Generated code is wrapped by a macro that will be used by the class
            return @$"#pragma once

#undef {EHLGeneratedBodyMacroName}
#define {EHLGeneratedBodyMacroName}() " + string.Join("\\\n", GeneratedDeclaration) + '\n';
        }

        public string GenerateFullDefinition()
        {
            return string.Join("\n", GeneratedDefinition);
        }

        public override string ToString()
        {
            return $"{FullyQualifiedName}:{Path.GetFileName(FileName)}";
        }
    }
}
