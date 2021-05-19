using System.IO;

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
    }
}
