using System.Collections.Generic;

namespace CodeGenerator.Configuration
{
    public class ConfigurationFile
    {
        public string? SourceDirectory { get; set; }

        public string? BinaryDirectory { get; set; }

        public List<string>? Definitions { get; set; }

        public List<string>? IncludePaths { get; set; }

        public List<string>? Headers { get; set; }

        public List<string>? PrecompiledHeaders { get; set; }
    }
}
