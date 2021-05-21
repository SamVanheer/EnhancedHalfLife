using System;
using System.Collections.Generic;

namespace CodeGenerator.Configuration
{
    /// <summary>
    /// Contains a list of files with relevant information
    /// </summary>
    public class EHLCodeGeneratorCache
    {
        public DateTimeOffset CodeGeneratorBuildTimestamp { get; set; }

        public List<CachedFileInfo>? Files { get; set; }
    }
}
