namespace CodeGenerator.Reflection
{
    public class FieldAttributes
    {
        public bool Persisted { get; set; }

        public bool IsGlobal { get; set; }

        public FieldType? Type { get; set; }
    }
}
