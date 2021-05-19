using System;

namespace CodeGenerator.Configuration
{
    public class CachedFileInfo
    {
        public string FileName { get; set; } = string.Empty;

        public DateTimeOffset LastProcessed { get; set; }

        /// <summary>
        /// Whether this file has a generated source file associated with it
        /// </summary>
        public bool HasGeneratedSourceFile { get; set; }
    }
}
