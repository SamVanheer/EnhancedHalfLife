using System.Collections.Generic;

namespace CodeGenerator.Configuration
{
    /// <summary>
    /// Contains a list of files with relevant information
    /// </summary>
    public class EHLCodeGeneratorCache
    {
        public List<CachedFileInfo>? Files { get; set; }
    }
}
