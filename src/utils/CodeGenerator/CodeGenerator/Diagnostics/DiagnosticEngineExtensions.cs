using ClangSharp;
using CodeGenerator.Utility;
using System.IO;

namespace CodeGenerator.Diagnostics
{
    public static class DiagnosticEngineExtensions
    {
        public static void LogError(this DiagnosticEngine diagnosticEngine, Cursor cursor, string message)
        {
            cursor.Location.GetSpellingLocation(out var file, out var line, out var column, out _);
            diagnosticEngine.AddDiagnostic(Path.GetFullPath(file.Name.CString.NormalizeSlashes()),
                line, column, message, Severity.Error);
        }
    }
}
