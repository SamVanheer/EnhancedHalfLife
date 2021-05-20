using System;
using System.Collections.Generic;

namespace CodeGenerator.Diagnostics
{
    public class FileDiagnostics
    {
        public string FileName { get; }

        private List<Diagnostic>? _diagnostics;

        public IReadOnlyList<Diagnostic> Diagnostics => _diagnostics ?? (IReadOnlyList<Diagnostic>)Array.Empty<Diagnostic>();

        public FileDiagnostics(string fileName)
        {
            FileName = fileName;
        }

        public void AddDiagnostic(Diagnostic diagnostic)
        {
            _diagnostics ??= new();
            _diagnostics.Add(diagnostic);
        }
    }
}
