using System.Collections.Generic;

namespace CodeGenerator.CodeGen
{
    public class GeneratedCode
    {
        private readonly Dictionary<string, GeneratedClassData> _classes = new();

        public IEnumerable<GeneratedClassData> Classes => _classes.Values;

        public GeneratedClassData Get(string scope, string name)
        {
            var fullyQualifiedName = scope.Length > 0 ? $"{scope}::{name}" : name;

            if (!_classes.TryGetValue(fullyQualifiedName, out var data))
            {
                data = new GeneratedClassData(scope, name);
                _classes.Add(fullyQualifiedName, data);
            }

            return data;
        }
    }
}
