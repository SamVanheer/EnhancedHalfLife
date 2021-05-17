using System.Collections.Generic;

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

        public string FullyQualifiedName => Scope.Length > 0 ? $"{Scope}::{Name}" : Name;

        public string BaseFileName => FullyQualifiedName.Replace("::", "_") + ".generated";

        public List<string> GeneratedDeclaration { get; } = new();

        public List<string> GeneratedDefinition { get; } = new();

        public GeneratedClassData(string scope, string name)
        {
            Scope = scope;
            Name = name;
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
    }
}
