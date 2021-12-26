namespace CodeGenerator.Reflection
{
    internal static class ReflectionConstants
    {
        public const string ClassPrefix = "Class=";
        public const string FieldPrefix = "Field=";

        public const string InitializationClassTemplatePrefix = "template<typename T>";
        public const string InitializationClassName = "TypeReflectionInitializer";
        public const string InitializationClassDeclaration = "class " + InitializationClassName;

        //These keys must match the names used by the attributes
        public const string EntityNameKey = nameof(ClassAttributes.EntityName);
        public const string EntityNameAliasesKey = nameof(ClassAttributes.EntityNameAliases);

        public const string Persisted = nameof(Persisted);

        public const string IsGlobal = nameof(IsGlobal);

        public const string PersistenceNamespace = "persistence::";
        public const string Type = nameof(Type);
        public const string TypeEnumName = "FieldType";

        public const string TrueString = "true";
        public const string FalseString = "false";
    }
}
