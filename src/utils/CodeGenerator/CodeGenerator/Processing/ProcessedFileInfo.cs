using System;

namespace CodeGenerator.Processing
{
    public class ProcessedFileInfo
    {
        public string FileName { get; }

        public DateTimeOffset LastProcessed { get; }

        /// <summary>
        /// true if this file has been processed during this run
        /// </summary>
        public bool FileProcessed { get; }

        /// <summary>
        /// Whether this file has a generated source file associated with it
        /// </summary>
        public bool HasGeneratedSourceFile { get; set; }

        public ProcessedFileInfo(string fileName, DateTimeOffset lastProcessed, bool fileProcessed, bool hasGeneratedSourceFile)
        {
            FileName = fileName;
            LastProcessed = lastProcessed;
            FileProcessed = fileProcessed;
            HasGeneratedSourceFile = hasGeneratedSourceFile;
        }
    }
}
