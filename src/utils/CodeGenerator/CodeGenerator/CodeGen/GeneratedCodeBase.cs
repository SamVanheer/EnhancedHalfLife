using System.Text;

namespace CodeGenerator.CodeGen
{
    /// <summary>
    /// Base class for generated code
    /// </summary>
    public abstract class GeneratedCodeBase
    {
        protected readonly StringBuilder _code = new();

        public override string ToString()
        {
            return _code.ToString();
        }
    }
}
