using System;

namespace CodeGenerator.CodeGen
{
    /// <summary>
    /// Provides methods to add generated class definition data
    /// </summary>
    public sealed class ClassDefinition : GeneratedCodeBase
    {
        public void AddHeaderInclude(string relativePath, bool isSystemHeader = false)
        {
            if (isSystemHeader)
            {
                _code.Insert(0, $"#include <{relativePath}>{Environment.NewLine}");
            }
            else
            {
                _code.Insert(0, $"#include \"{relativePath}\"{Environment.NewLine}");
            }
        }

        public void AddMethodDefinition(string definition)
        {
            _code.AppendLine(definition);
        }

        public void AddVariableDefinition(string definition)
        {
            _code.AppendLine(definition);
        }

        public void AddStaticAssertDefinition(string condition, string? message = null)
        {
            var assert = message is not null ? $"static_assert({condition}, \"{message}\");" : $"static_assert({condition});";

            _code.AppendLine(assert);
        }

        public void AddDefinitionMacro(string macro)
        {
            _code.AppendLine(macro);
        }

        public void AddGlobalVariable(string variable)
        {
            _code.Append(variable).AppendLine(";");
        }
    }
}
