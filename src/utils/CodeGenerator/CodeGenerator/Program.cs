using CodeGenerator.CodeGen;
using CodeGenerator.Configuration;
using CodeGenerator.Diagnostics;
using CodeGenerator.Processing;
using CodeGenerator.Reflection;
using Newtonsoft.Json;
using System;
using System.Collections.Immutable;
using System.CommandLine;
using System.CommandLine.Invocation;
using System.IO;
using System.Linq;

namespace CodeGenerator
{
    public static class Program
    {
        private const string EHLCodeGeneratorCacheFileName = "EHLCodeGeneratorCache.json";
        private const string EHLCodeGeneratorNewCacheFileName = "EHLCodeGeneratorNewCache.json";

        private const int ErrorSuccess = 0;
        private const int ErrorNoConfigFile = 1;
        private const int ErrorInvalidConfigFile = 2;
        private const int ErrorProcessingFailed = 3;
        private const int ErrorErrorFoundInCodegen = 4;
        private const int ErrorNoNewCacheFile = 5;

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

            generateCommand.AddOption(new Option<bool>("--display-diagnostics",
                description: "Display Clang diagnostics during processing"));

            generateCommand.Handler = CommandHandler.Create<string, bool, bool>(GenerateCode);

            var updateCacheFileCommand = new Command("update-cache-file", "Updates the cache file to include changes from the last run");

            rootCommand.AddCommand(updateCacheFileCommand);

            updateCacheFileCommand.AddOption(new Option<string>("--config-file",
                    description: "Specifies the path to the configuration file"));

            updateCacheFileCommand.Handler = CommandHandler.Create<string>(UpdateCacheFile);

            var cleanCommand = new Command("clean", "Cleans up the generated code and caches");

            rootCommand.AddCommand(cleanCommand);

            cleanCommand.AddOption(new Option<string>("--config-file",
                    description: "Specifies the path to the configuration file"));

            cleanCommand.Handler = CommandHandler.Create<string>(CleanGeneratedCode);

            return rootCommand.InvokeAsync(args).Result;
        }

        private static int GenerateCode(string configFile, bool dryRun, bool displayDiagnostics)
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

            try
            {
                //The timestamp can be off by a second if it gets queried multiple times, so make sure to use the same value for both read & write
                var currentBuildTimestamp = GetCodeGeneratorBuildTimeStamp();

                var generatedCode = new GeneratedCode();

                var cachedFileInfo = LoadCache(config.BinaryDirectory!, currentBuildTimestamp);

                Console.WriteLine("Processing code...");

                var diagnosticEngine = new DiagnosticEngine();

                using var engine = new ProcessingEngine(
                    config.Definitions!.ToImmutableHashSet(),
                    config.IncludePaths!.ToImmutableList(),
                    config.Headers!.Select(h => Path.GetFullPath(h, config.SourceDirectory!)),
                    cachedFileInfo,
                    displayDiagnostics);

                engine.Processors.Add(new ReflectionProcessor(diagnosticEngine, generatedCode));

                var newFileInfo = engine.Process();

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

                        //Write the new cache file
                        SaveCache(config.BinaryDirectory!, currentBuildTimestamp, EHLCodeGeneratorNewCacheFileName, newFileInfo);
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

            return ErrorSuccess;
        }

        private static int UpdateCacheFile(string configFile)
        {
            var config = LoadConfigurationFile(configFile);

            if (config is null)
            {
                return ErrorInvalidConfigFile;
            }

            var newCacheFileName = Path.Combine(config.BinaryDirectory!, EHLCodeGeneratorNewCacheFileName);

            if (!File.Exists(newCacheFileName))
            {
                Console.Error.WriteLine("New cache file not found");
                return ErrorNoNewCacheFile;
            }

            Console.WriteLine("Updating cache file...");

            var cacheFileName = Path.Combine(config.BinaryDirectory!, EHLCodeGeneratorCacheFileName);

            File.Move(newCacheFileName, cacheFileName, true);

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
            File.Delete(Path.Combine(config.BinaryDirectory!, EHLCodeGeneratorNewCacheFileName));

            return ErrorSuccess;
        }

        private static ConfigurationFile? LoadConfigurationFile(string configFile)
        {
            var configFileContents = File.ReadAllText(configFile);

            var config = JsonConvert.DeserializeObject<ConfigurationFile>(configFileContents);

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

        private static DateTimeOffset GetCodeGeneratorBuildTimeStamp()
        {
            var fileInfo = new FileInfo(typeof(Program).Assembly.Location);

            fileInfo.Refresh();

            return fileInfo.LastWriteTimeUtc;
        }

        private static ImmutableDictionary<string, CachedFileInfo> LoadCache(string binaryDirectory, DateTimeOffset currentBuildTimestamp)
        {
            try
            {
                var changeListContents = File.ReadAllText(Path.Combine(binaryDirectory, EHLCodeGeneratorCacheFileName));
                var changeList = JsonConvert.DeserializeObject<EHLCodeGeneratorCache>(changeListContents);

                if (changeList is null
                    || changeList.Files is null)
                {
                    throw new ProcessingException("File cache is invalid");
                }

                //Only use the cache if it was generated by this version of the code generator
                if (changeList.CodeGeneratorBuildTimestamp >= currentBuildTimestamp)
                {
                    return changeList.Files.ToImmutableDictionary(f => f.FileName);
                }
                else
                {
                    Console.WriteLine("Ignoring cache; code generator has been updated");
                }
            }
            catch (FileNotFoundException)
            {
            }

            return ImmutableDictionary<string, CachedFileInfo>.Empty;
        }

        private static void SaveCache(string binaryDirectory, DateTimeOffset currentBuildTimestamp,
            string cacheFileName, ImmutableDictionary<string, ProcessedFileInfo> newFileChangeInfo)
        {
            var list = new EHLCodeGeneratorCache
            {
                CodeGeneratorBuildTimestamp = currentBuildTimestamp,
                Files = newFileChangeInfo.Values
                    .Select(i => new CachedFileInfo
                    {
                        FileName = i.FileName,
                        LastProcessed = i.LastProcessed,
                        HasGeneratedSourceFile = i.HasGeneratedSourceFile
                    })
                    .ToList()
            };

            var serialized = JsonConvert.SerializeObject(list, settings: new JsonSerializerSettings
            {
                Formatting = Formatting.Indented
            });

            File.WriteAllText(Path.Combine(binaryDirectory, cacheFileName), serialized);
        }
    }
}
