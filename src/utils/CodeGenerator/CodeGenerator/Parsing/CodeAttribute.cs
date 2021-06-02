using System.Collections.Immutable;

namespace CodeGenerator.Parsing
{
    public sealed class CodeAttribute
    {
        public string Name { get; }

        public ImmutableList<CodeAttribute> Values { get; }

        public CodeAttribute(string name, ImmutableList<CodeAttribute> values)
        {
            Name = name;
            Values = values;
        }

        public CodeAttribute(string name)
            : this(name, ImmutableList<CodeAttribute>.Empty)
        {
        }

        public override string ToString()
        {
            return $"{Name}:{Values.Count}";
        }
    }
}
