using ClangSharp;
using CodeGenerator.Diagnostics;
using CodeGenerator.Parsing;
using System;
using System.Linq;

namespace CodeGenerator.Utility
{
    public static class NamedDeclExtensions
    {
        public static (string Scope, string FullyQualifiedName) GetFullyQualifiedName(this NamedDecl decl)
        {
            string scope = string.Empty;

            for (Decl context = (Decl)decl.DeclContext; context?.IsTranslationUnit == false; context = (Decl)context.DeclContext)
            {
                scope = scope.Length == 0 ? context.Spelling : $"{context.Spelling}::{scope}";
            }

            return (scope, scope.Length > 0 ? $"{scope}::{decl.Name}" : decl.Name);
        }

        public static (T? Attributes, bool HasAnyAttributes) GetAttributes<T>(
            this NamedDecl decl, DiagnosticEngine diagnosticEngine, string prefix)
            where T : class, new()
        {
            var rawAttributes = decl.Attrs
                    .Where(attr => attr.Spelling.StartsWith(prefix))
                    .ToList();

            var text = string.Join(",", rawAttributes.Select(attr => attr.Spelling.AsSpan()[prefix.Length..].ToString()));

            var attributes = AttributeSerializer.Deserialize<T>(decl, diagnosticEngine, text);

            return (attributes, rawAttributes.Count > 0);
        }
    }
}
