using ClangSharp;
using ClangSharp.Interop;
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
        private const string PrecompiledHeaderName = "Precompiled.pch";

        private readonly ImmutableHashSet<string> _definitions;

        private readonly ImmutableList<string> _includePaths;

        private readonly ImmutableList<string> _headers;

        private readonly ImmutableList<string> _precompiledHeaders;

        private readonly ClangSharp.Index _index;

        public List<IProcessor> Processors { get; } = new();

        public ProcessingEngine(ImmutableHashSet<string> definitions,
            ImmutableList<string> includePaths,
            ImmutableList<string> headers,
            ImmutableList<string> precompiledHeaders)
        {
            _definitions = definitions;
            _includePaths = includePaths;
            _headers = headers;
            _precompiledHeaders = precompiledHeaders;

            _index = ClangSharp.Index.Create(displayDiagnostics: false);
        }

        public void Process()
        {
            var sharedArguments = CreateSharedCommandLineArguments();

            try
            {
                (bool hasPrecompiledHeader, ImmutableHashSet<string> filesToSkip) = MaybeGeneratePrecompiledHeader(sharedArguments);

                var commandLineArgs = CreateCommandLineArguments(sharedArguments, hasPrecompiledHeader);

                var files = _headers.Except(filesToSkip).ToList();

                foreach (var file in files)
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
            }
            finally
            {
                //Delete the temporary precompiled header
                File.Delete(PrecompiledHeaderName);
            }
        }

        private string[] CreateSharedCommandLineArguments()
        {
            var args = new List<string>
            {
                "-xc++",
                "-std=c++17",
                "-DEHL_CODEGEN_PROCESSING" //Tells the code that we're processing it for codegen
            };

            args.AddRange(_definitions.Select(d => $"-D{d}"));
            args.AddRange(_includePaths.Select(i => $"-I{i}"));

            return args.ToArray();
        }

        private static string[] CreateCommandLineArguments(string[] sharedArguments, bool hasPrecompiledHeader)
        {
            var list = sharedArguments.ToList();

            if (hasPrecompiledHeader)
            {
                list.Add("-include-pch");
                list.Add(PrecompiledHeaderName);
            }

            return list.ToArray();
        }

        private unsafe (bool, ImmutableHashSet<string>) MaybeGeneratePrecompiledHeader(string[] sharedArguments)
        {
            if (_precompiledHeaders.IsEmpty)
            {
                return (false, ImmutableHashSet<string>.Empty);
            }

            var parseError = CXTranslationUnit.TryParse(
                    _index.Handle,
                    null,
                    sharedArguments
                        .Concat(_precompiledHeaders)
                        .ToArray(),
                    ReadOnlySpan<CXUnsavedFile>.Empty,
                    CXTranslationUnit_Flags.CXTranslationUnit_ForSerialization
                    | CXTranslationUnit_Flags.CXTranslationUnit_Incomplete,
                    out var precompiledTU);

            if (parseError != CXErrorCode.CXError_Success)
            {
                throw new ProcessingException($"Error parsing precompiled headers: {parseError}");
            }

            var saveError = precompiledTU.Save(PrecompiledHeaderName, (CXSaveTranslationUnit_Flags)clang.defaultSaveOptions(precompiledTU));

            if (saveError != CXSaveError.CXSaveError_None)
            {
                throw new ProcessingException($"Error saving precompiled headers: {saveError}");
            }

            using var translationUnit = TranslationUnit.GetOrCreate(precompiledTU);

            ProcessTranslationUnit(translationUnit);

            var filesToSkip = ImmutableHashSet.CreateBuilder<string>();

            //Find all files that were included and add them for exclusion
            foreach (var child in translationUnit.TranslationUnitDecl.CursorChildren)
            {
                child.Location.GetSpellingLocation(out var file, out _, out _, out _);

                var path = file.Name.CString;

                //Normalize the paths
                path = path.Replace('/', Path.DirectorySeparatorChar);
                path = path.Replace('\\', Path.DirectorySeparatorChar);
                path = Path.GetFullPath(path);

                filesToSkip.Add(path);
            }

            return (true, filesToSkip.ToImmutable());
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
