using System.Collections.Generic;

namespace CodeGenerator.CodeGen
{
    public class GeneratedCode
    {
        private readonly Dictionary<string, GeneratedClassData> _classes = new();

        public IEnumerable<GeneratedClassData> Classes => _classes.Values;

        public void Add(GeneratedClassData data)
        {
            _classes.Add(data.FullyQualifiedName, data);
        }
    }
}
