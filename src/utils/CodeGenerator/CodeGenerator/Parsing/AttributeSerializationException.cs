using System;

namespace CodeGenerator.Parsing
{
    public sealed class AttributeSerializationException : Exception
    {
        public AttributeSerializationException()
        {
        }

        public AttributeSerializationException(string? message)
            : base(message)
        {
        }

        public AttributeSerializationException(string? message, Exception? innerException)
            : base(message, innerException)
        {
        }
    }
}
