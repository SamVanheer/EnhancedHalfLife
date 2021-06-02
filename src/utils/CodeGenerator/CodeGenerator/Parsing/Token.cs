using System;

namespace CodeGenerator.Parsing
{
    internal readonly ref struct Token
    {
        public readonly ReadOnlySpan<char> Value;

        public Token(ReadOnlySpan<char> value)
        {
            Value = value;
        }

        public override string ToString()
        {
            return Value.ToString();
        }
    }
}
