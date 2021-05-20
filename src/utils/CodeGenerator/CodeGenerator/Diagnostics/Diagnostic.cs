namespace CodeGenerator.Diagnostics
{
    public class Diagnostic
    {
        public uint Line { get; }

        public uint Column { get; }

        public string Message { get; }

        public Severity Severity { get; }

        public Diagnostic(uint line, uint column, string message, Severity severity)
        {
            Line = line;
            Column = column;
            Message = message;
            Severity = severity;
        }
    }
}
