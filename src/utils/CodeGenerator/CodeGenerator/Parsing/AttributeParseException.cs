using System;

namespace CodeGenerator.Parsing
{
    public sealed class AttributeParseException : Exception
    {
        public AttributeParseException()
        {
        }

        public AttributeParseException(string? message)
            : base(message)
        {
        }

        public AttributeParseException(string? message, Exception? innerException)
            : base(message, innerException)
        {
        }
    }
}
