using CodeGenerator.CodeGen;
using CodeGenerator.Configuration;
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
        private const int ErrorSuccess = 0;
        private const int ErrorNoConfigFile = 1;
        private const int ErrorInvalidConfigFile = 2;
        private const int ErrorProcessingFailed = 3;

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

            return rootCommand.InvokeAsync(args).Result;
        }

        private static int GenerateCode(string configFile, bool dryRun)
        {
            if (string.IsNullOrWhiteSpace(configFile))
            {
                Console.WriteLine("No configuration file specified, aborting");
                return ErrorNoConfigFile;
            }

            var configFileContents = File.ReadAllText(configFile);

            var config = JsonSerializer.Deserialize<ConfigurationFile>(configFileContents);

            if (config is null
                || config.SourceDirectory is null
                || config.Definitions is null
                || config.IncludePaths is null
                || config.Headers is null
                || config.PrecompiledHeaders is null)
            {
                Console.WriteLine("Configuration file is invalid");
                return ErrorInvalidConfigFile;
            }

            Console.WriteLine("Processing code...");

            using var engine = new ProcessingEngine(
                config.Definitions.ToImmutableHashSet(),
                config.IncludePaths.ToImmutableList(),
                config.Headers.Select(h => Path.GetFullPath(h, config.SourceDirectory)).ToImmutableList(),
                config.PrecompiledHeaders.ToImmutableList());

            var generatedCode = new GeneratedCode();

            engine.Processors.Add(new PersistenceProcessor(generatedCode));

            try
            {
                engine.Process();
            }
            catch (ProcessingException e)
            {
                Console.Error.WriteLine("An error occurred while processing: {0}", e.Message);
                return ErrorProcessingFailed;
            }

            Console.WriteLine("Generating code...");

            if (!dryRun)
            {
                var codeGenerator = new Generator(generatedCode);

                codeGenerator.Generate();
            }

            return ErrorSuccess;
        }
    }
}
