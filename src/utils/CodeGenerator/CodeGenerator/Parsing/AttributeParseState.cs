using System;

namespace CodeGenerator.Parsing
{
    internal readonly ref struct AttributeParseState
    {
        public readonly ReadOnlySpan<char> Text { get; }

        public readonly int Index { get; }

        public AttributeParseState(ref AttributeParser parser)
        {
            Text = parser._text;
            Index = parser._index;
        }

        public override string ToString()
        {
            return $"[{Index}..{Text.Length}]";
        }
    }
}
