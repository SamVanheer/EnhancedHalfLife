using System.Collections.Generic;

namespace CodeGenerator.CodeGen
{
    public class GeneratedCode
    {
        private readonly Dictionary<string, GeneratedClass> _classes = new();

        public IEnumerable<GeneratedClass> Classes => _classes.Values;

        public void Add(GeneratedClass data)
        {
            _classes.Add(data.FullyQualifiedName, data);
        }
    }
}
