using System.Text;

namespace CodeGenerator.CodeGen
{
    /// <summary>
    /// Provides methods to add generated class declaration data
    /// </summary>
    public sealed class ClassDeclaration : GeneratedCodeBase
    {
        public ClassDeclaration()
        {
            _code.AppendFormat("#undef {0}", CodeGenConstants.EHLGeneratedBodyMacroName).AppendLine();
            _code.AppendFormat("#define {0}()", CodeGenConstants.EHLGeneratedBodyMacroName);
        }

        private StringBuilder AddNewLine()
        {
            return _code.AppendLine("\\");
        }

        public void AddAccessDeclaration(ClassAccess access)
        {
            AddNewLine().AppendFormat("{0}:", access.ToString().ToLower());
        }

        public void AddMacroInstantiation(string macro)
        {
            AddNewLine().Append(macro);
        }

        public void AddFriend(string functionDeclaration, string? templatePrefix)
        {
            if (templatePrefix is not null)
            {
                AddNewLine().Append(templatePrefix);
            }

            AddNewLine().AppendFormat("friend {0};", functionDeclaration);
        }

        public void AddMethodDeclaration(string declaration, bool isBase, bool isVirtual = false)
        {
            if (!isBase)
            {
                declaration += " override";
            }
            else if (isVirtual)
            {
                declaration = "virtual " + declaration;
            }

            AddNewLine().AppendFormat("{0};", declaration);
        }

        public void AddVariableDeclaration(string declaration)
        {
            AddNewLine().AppendFormat("{0};", declaration);
        }

        public void AddTypeAlias(string aliasName, string originalName)
        {
            AddNewLine().AppendFormat("using {0} = {1};", aliasName, originalName);
        }
    }
}
