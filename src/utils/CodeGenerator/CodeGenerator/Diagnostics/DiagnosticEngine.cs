using System;
using System.Collections.Generic;
using System.Linq;

namespace CodeGenerator.Diagnostics
{
    public class DiagnosticEngine
    {
        private readonly Dictionary<string, FileDiagnostics> _diagnostics = new();

        public IReadOnlyDictionary<string, FileDiagnostics> Diagnostics => _diagnostics;

        public bool HasErrorDiagnostics => _diagnostics.Values.Any(f => f.Diagnostics.Any(d => d.Severity == Severity.Error));

        public void AddDiagnostic(string fileName, uint line, uint column, string message, Severity severity)
        {
            if (!_diagnostics.TryGetValue(fileName, out var fileDiagnostics))
            {
                fileDiagnostics = new(fileName);
                _diagnostics.Add(fileName, fileDiagnostics);
            }

            var diagnostic = new Diagnostic(line, column, message.Trim(), severity);

            fileDiagnostics.AddDiagnostic(diagnostic);

            PrintDiagnostic(fileName, diagnostic);
        }

        public void PrintDiagnostics()
        {
            foreach (var file in _diagnostics.Values.OrderBy(f => f.FileName))
            {
                foreach (var diagnostic in file.Diagnostics)
                {
                    PrintDiagnostic(file.FileName, diagnostic);
                }
            }
        }

        private static void PrintDiagnostic(string fileName, Diagnostic diagnostic)
        {
            var outputStream = diagnostic.Severity switch
            {
                Severity.Error => Console.Error,
                _ => Console.Out
            };

            outputStream.WriteLine("{0}({1},{2}): {3}: {4}",
                fileName, diagnostic.Line, diagnostic.Column, diagnostic.Severity, diagnostic.Message);
        }

        public void GetDiagnosticCounts(out int infoCount, out int warningCount, out int errorCount)
        {
            infoCount = 0;
            warningCount = 0;
            errorCount = 0;

            foreach (var file in _diagnostics.Values)
            {
                foreach (var diagnostic in file.Diagnostics)
                {
                    switch (diagnostic.Severity)
                    {
                        case Severity.Information:
                            ++infoCount;
                            break;
                        case Severity.Warning:
                            ++warningCount;
                            break;
                        case Severity.Error:
                            ++errorCount;
                            break;
                    }
                }
            }
        }
    }
}
