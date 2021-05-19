using CodeGenerator.CodeGen;
using System;
using System.IO;
using System.Linq;

namespace CodeGenerator.ErrorDetection
{
    public static class ErrorChecker
    {
        /// <summary>
        /// Checks to see if there are any errors in the generated code
        /// </summary>
        /// <param name="sourceDirectory"></param>
        /// <param name="generatedCode"></param>
        public static bool CheckForErrors(string sourceDirectory, GeneratedCode generatedCode)
        {
            var headersWithMultipleClasses = generatedCode.Classes
                .GroupBy(c => c.FileName)
                .Where(g => g.Count() > 1)
                .ToList();

            if (headersWithMultipleClasses.Count == 0)
            {
                return false;
            }

            foreach (var headerWithMultipleClasses in headersWithMultipleClasses)
            {
                //Use relative paths to make it easier to find
                Console.Error.WriteLine("Multiple classes found in header {0}:", Path.GetRelativePath(sourceDirectory, headerWithMultipleClasses.Key));

                foreach (var clazz in headerWithMultipleClasses)
                {
                    Console.Error.WriteLine("\tClass {0}", clazz.FullyQualifiedName);
                }
            }

            Console.Error.WriteLine("Move each class to its own file to avoid codegen errors");

            return true;
        }
    }
}
