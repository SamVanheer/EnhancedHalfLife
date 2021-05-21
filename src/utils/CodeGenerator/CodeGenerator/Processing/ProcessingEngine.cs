using ClangSharp;
using ClangSharp.Interop;
using CodeGenerator.Configuration;
using CodeGenerator.Utility;
using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.IO;
using System.Linq;

namespace CodeGenerator.Processing
{
    /// <summary>
    /// Given a set of headers, creates a translation unit for each file and invokes a set of processors on them
    /// </summary>
    public class ProcessingEngine : IDisposable
    {
        private readonly ImmutableHashSet<string> _definitions;

        private readonly ImmutableList<string> _includePaths;

        private readonly ImmutableList<string> _headers;

        private readonly ImmutableDictionary<string, CachedFileInfo> _cachedFileInfo;

        private readonly ClangSharp.Index _index;

        public List<IProcessor> Processors { get; } = new();

        public ProcessingEngine(ImmutableHashSet<string> definitions,
            ImmutableList<string> includePaths,
            IEnumerable<string> headers,
            ImmutableDictionary<string, CachedFileInfo> cachedFileInfo,
            bool displayDiagnostics)
        {
            _definitions = definitions;
            _includePaths = includePaths;
            _headers = headers.Select(h => Path.GetFullPath(h.NormalizeSlashes())).ToImmutableList();
            _cachedFileInfo = cachedFileInfo;

            _index = ClangSharp.Index.Create(displayDiagnostics: displayDiagnostics);
        }

        /// <summary>
        /// Processes the list of header files
        /// </summary>
        /// <returns>Updated list of file change info</returns>
        public ImmutableDictionary<string, ProcessedFileInfo> Process()
        {
            var newFileInfo = new List<ProcessedFileInfo>();

            var commandLineArgs = CreateCommandLineArguments();

            foreach (var file in _headers)
            {
                var lastChange = File.GetLastWriteTimeUtc(file);

                _cachedFileInfo.TryGetValue(file, out var fileChangeInfo);

                bool processFile = fileChangeInfo is null || lastChange > fileChangeInfo.LastProcessed;

                //if we've processed this file after the last change time we can skip doing it again
                if (processFile)
                {
                    var parseError = CXTranslationUnit.TryParse(_index.Handle, file, commandLineArgs, ReadOnlySpan<CXUnsavedFile>.Empty,
                        CXTranslationUnit_Flags.CXTranslationUnit_None
                        | CXTranslationUnit_Flags.CXTranslationUnit_SkipFunctionBodies,
                        out var tunit);

                    if (parseError != CXErrorCode.CXError_Success)
                    {
                        throw new ProcessingException($"Error parsing translation unit \"{file}\": {parseError}");
                    }

                    using var translationUnit = TranslationUnit.GetOrCreate(tunit);

                    ProcessTranslationUnit(translationUnit);
                }

                //Only carry over generated source file info if we didn't process the header so the generator can provide an up-to-date value later on
                newFileInfo.Add(new ProcessedFileInfo(file, DateTimeOffset.UtcNow, processFile, !processFile && fileChangeInfo?.HasGeneratedSourceFile == true));
            }

            return newFileInfo.ToImmutableDictionary(i => i.FileName);
        }

        private string[] CreateCommandLineArguments()
        {
            var args = new List<string>
            {
                "-xc++",
                "-std=c++17",
                "-DEHL_CODEGEN_PROCESSING", //Tells the code that we're processing it for codegen
                "-Wno-pragma-once-outside-header",
            };

            args.AddRange(_definitions.Select(d => $"-D{d}"));
            args.AddRange(_includePaths.Select(i => $"-I{i}"));

            return args.ToArray();
        }

        private void ProcessTranslationUnit(TranslationUnit translationUnit)
        {
            foreach (var processor in Processors)
            {
                processor.ProcessTranslationUnit(translationUnit);
            }
        }

        private bool disposedValue;

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                _index.Dispose();
                disposedValue = true;
            }
        }

        ~ProcessingEngine()
        {
            Dispose(disposing: false);
        }

        public void Dispose()
        {
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }
}
