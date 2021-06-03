using System.Collections.Immutable;

namespace CodeGenerator.Persistence
{
    public class ClassAttributes
    {
        public string? EntityName { get; set; }

        public ImmutableList<string> EntityNameAliases { get; set; } = ImmutableList<string>.Empty;
    }
}
