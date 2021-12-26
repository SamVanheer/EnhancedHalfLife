using System.Text;

namespace CodeGenerator.CodeGen
{
    public sealed class GeneratedClass
    {
        public string Scope { get; }

        public string Name { get; }

        public string FileName { get; }

        public string FullyQualifiedName => Scope.Length > 0 ? $"{Scope}::{Name}" : Name;

        public ClassDeclaration Declaration { get; } = new();
        public ClassDefinition Definition { get; } = new();

        public GeneratedClass(string scope, string name, string fileName)
        {
            Scope = scope;
            Name = name;
            FileName = fileName;
        }

        public string GetGeneratedHeaderContents()
        {
            var builder = new StringBuilder();

            builder.AppendLine("#pragma once");

            builder.AppendLine(Declaration.ToString());

            return builder.ToString();
        }

        public string GetGeneratedSourceContents()
        {
            return Definition.ToString();
        }
    }
}
