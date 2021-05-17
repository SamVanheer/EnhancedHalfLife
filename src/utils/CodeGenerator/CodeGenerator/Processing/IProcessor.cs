using ClangSharp;

namespace CodeGenerator.Processing
{
    /// <summary>
    /// Anything that processes translation units should implement this interface
    /// </summary>
    public interface IProcessor
    {
        void ProcessTranslationUnit(TranslationUnit translationUnit);
    }
}
