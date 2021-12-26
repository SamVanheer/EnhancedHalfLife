using System;
using System.IO;
using System.Text.RegularExpressions;

namespace CodeGenerator.Utility
{
    public static class StringUtils
    {
        public static string NormalizeSlashes(this string path)
        {
            path = path.Replace('/', Path.DirectorySeparatorChar);
            path = path.Replace('\\', Path.DirectorySeparatorChar);

            return path;
        }

        public static string NormalizeNewlines(this string text, string newLineFormat)
        {
            //Regex from: https://stackoverflow.com/a/141069
            return Regex.Replace(text, @"\r\n|\n\r|\n|\r", newLineFormat);
        }

        public static string NormalizeNewlines(this string text)
        {
            return text.NormalizeNewlines(Environment.NewLine);
        }

        public static string WrapWithDoubleQuotes(this string text)
        {
            return $"\"{text}\"";
        }

        public static string ConvertTypeNameToIdentifierName(this string name)
        {
            return name.Replace("::", "_");
        }
    }
}
