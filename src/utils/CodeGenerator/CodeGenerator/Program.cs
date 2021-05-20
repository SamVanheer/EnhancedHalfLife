using CodeGenerator.CodeGen;
using CodeGenerator.Configuration;
using CodeGenerator.Diagnostics;
using CodeGenerator.Persistence;
using CodeGenerator.Processing;
using System;
using System.Collections.Immutable;
using System.CommandLine;
using System.CommandLine.Invocation;
using System.IO;
using System.Linq;
using System.Text.Json;

namespace CodeGenerator
{
    public static class Program
    {
        private const string EHLCodeGeneratorCacheFileName = "EHLCodeGeneratorCache.json";

        private const int ErrorSuccess = 0;
        private const int ErrorNoConfigFile = 1;
        private const int ErrorInvalidConfigFile = 2;
        private const int ErrorProcessingFailed = 3;
        private const int ErrorErrorFoundInCodegen = 4;

        public static int Main(string[] args)
        {
            var rootCommand = new RootCommand
            {
                Description = "Enhanced Half-Life Code Generator"
            };

            var generateCommand = new Command("generate", "Generates source code for the target");

            rootCommand.AddCommand(generateCommand);

            generateCommand.AddOption(new Option<string>("--config-file",
                    description: "Specifies the path to the configuration file"));

            generateCommand.AddOption(new Option<bool>("--dry-run",
                description: "Process the target but don't generate any code"));

            generateCommand.Handler = CommandHandler.Create<string, bool>(GenerateCode);

            var cleanCommand = new Command("clean", "Cleans up the generated code and caches");

            rootCommand.AddCommand(cleanCommand);

            cleanCommand.AddOption(new Option<string>("--config-file",
                    description: "Specifies the path to the configuration file"));

            cleanCommand.Handler = CommandHandler.Create<string>(CleanGeneratedCode);

            return rootCommand.InvokeAsync(args).Result;
        }

        private static int GenerateCode(string configFile, bool dryRun)
        {
            if (string.IsNullOrWhiteSpace(configFile))
            {
                Console.WriteLine("No configuration file specified, aborting");
                return ErrorNoConfigFile;
            }

            var config = LoadConfigurationFile(configFile);

            if (config is null)
            {
                return ErrorInvalidConfigFile;
            }

            var generatedCode = new GeneratedCode();

            ImmutableDictionary<string, ProcessedFileInfo>? newFileInfo = null;

            try
            {
                var cachedFileInfo = LoadCache(config.BinaryDirectory!);

                Console.WriteLine("Processing code...");

                var diagnosticEngine = new DiagnosticEngine();

                using var engine = new ProcessingEngine(
                    config.Definitions!.ToImmutableHashSet(),
                    config.IncludePaths!.ToImmutableList(),
                    config.Headers!.Select(h => Path.GetFullPath(h, config.SourceDirectory!)),
                    cachedFileInfo);

                engine.Processors.Add(new PersistenceProcessor(diagnosticEngine, generatedCode));

                newFileInfo = engine.Process();

                //diagnosticEngine.PrintDiagnostics();

                diagnosticEngine.GetDiagnosticCounts(out var infoCount, out var warningCount, out var errorCount);

                Console.WriteLine("{0} Messages, {1} Warnings, {2} Errors", infoCount, warningCount, errorCount);

                if (errorCount == 0)
                {
                    if (!dryRun)
                    {
                        Console.WriteLine("Generating code...");
                        var codeGenerator = new Generator(config.BinaryDirectory!, newFileInfo, generatedCode);

                        codeGenerator.Generate();
                    }
                }
                else
                {
                    Console.WriteLine("One or more errors found; no code generated");
                    //The cache is not updated so a second build still shows the same errors
                    return ErrorErrorFoundInCodegen;
                }
            }
            catch (ProcessingException e)
            {
                Console.Error.WriteLine("An error occurred while processing: {0}", e.Message);
                return ErrorProcessingFailed;
            }

            //Update the list on disk
            SaveCache(config.BinaryDirectory!, newFileInfo);

            return ErrorSuccess;
        }

        private static int CleanGeneratedCode(string configFile)
        {
            var config = LoadConfigurationFile(configFile);

            if (config is null)
            {
                return ErrorInvalidConfigFile;
            }

            Console.WriteLine("Cleaning codegen in {0}...", config.BinaryDirectory);

            //Delete all generated files and the cache
            var generatedDirectory = Path.Combine(config.BinaryDirectory!, Generator.GeneratedDirectory);

            if (Directory.Exists(generatedDirectory))
            {
                Directory.Delete(Path.Combine(config.BinaryDirectory!, Generator.GeneratedDirectory), true);
            }

            File.Delete(Path.Combine(config.BinaryDirectory!, EHLCodeGeneratorCacheFileName));

            return ErrorSuccess;
        }

        private static ConfigurationFile? LoadConfigurationFile(string configFile)
        {
            var configFileContents = File.ReadAllText(configFile);

            var config = JsonSerializer.Deserialize<ConfigurationFile>(configFileContents);

            if (config is null
                || config.SourceDirectory is null
                || config.BinaryDirectory is null
                || config.Definitions is null
                || config.IncludePaths is null
                || config.Headers is null)
            {
                Console.WriteLine("Configuration file is invalid");
                return null;
            }

            return config;
        }

        private static ImmutableDictionary<string, CachedFileInfo> LoadCache(string binaryDirectory)
        {
            try
            {
                var changeListContents = File.ReadAllText(Path.Combine(binaryDirectory, EHLCodeGeneratorCacheFileName));
                var changeList = JsonSerializer.Deserialize<EHLCodeGeneratorCache>(changeListContents);

                if (changeList is null
                    || changeList.Files is null)
                {
                    throw new ProcessingException("File cache is invalid");
                }

                return changeList.Files.ToImmutableDictionary(f => f.FileName);
            }
            catch (FileNotFoundException)
            {
                return ImmutableDictionary<string, CachedFileInfo>.Empty;
            }
        }

        private static void SaveCache(string binaryDirectory, ImmutableDictionary<string, ProcessedFileInfo> newFileChangeInfo)
        {
            var list = new EHLCodeGeneratorCache
            {
                Files = newFileChangeInfo.Values
                    .Select(i => new CachedFileInfo
                    {
                        FileName = i.FileName,
                        LastProcessed = i.LastProcessed,
                        HasGeneratedSourceFile = i.HasGeneratedSourceFile
                    })
                    .ToList()
            };

            var serialized = JsonSerializer.Serialize(list, options: new JsonSerializerOptions
            {
                WriteIndented = true
            });

            File.WriteAllText(Path.Combine(binaryDirectory, EHLCodeGeneratorCacheFileName), serialized);
        }
    }
}
