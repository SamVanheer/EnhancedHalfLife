using ClangSharp;
using CodeGenerator.Diagnostics;
using CodeGenerator.Utility;
using Newtonsoft.Json;
using System.IO;

namespace CodeGenerator.Parsing
{
    public static class AttributeSerializer
    {
        /// <summary>
        /// Deserializes attribute text to an object
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="cursor"></param>
        /// <param name="diagnosticEngine"></param>
        /// <param name="text"></param>
        public static T? Deserialize<T>(Cursor cursor, DiagnosticEngine diagnosticEngine, string text)
            where T : class, new()
        {
            try
            {
                //Wrap the JSON to match the expected structure
                var json = "{" + text + "}";

                bool hasErrors = false;

                var settings = new JsonSerializerSettings
                {
                    MissingMemberHandling = MissingMemberHandling.Error,
                    Error = (sender, args) =>
                    {
                        cursor.Location.GetSpellingLocation(out var file, out var line, out var column, out _);
                        diagnosticEngine.AddDiagnostic(Path.GetFullPath(file.Name.CString.NormalizeSlashes()),
                            line, column, args.ErrorContext.Error.Message, Severity.Error);

                        args.ErrorContext.Handled = true;

                        hasErrors = true;
                    }
                };

                var attributes = JsonConvert.DeserializeObject<T>(json, settings);

                //Check for errors before checking for null. If there are errors null is returned from DeserializeObject.
                if (hasErrors)
                {
                    return null;
                }

                if (attributes is null)
                {
                    throw new AttributeSerializationException("Error deserializing attributes: null object");
                }

                return attributes;
            }
            catch (JsonException e)
            {
                throw new AttributeSerializationException($"Error deserializing attribute: {e.Message}", e);
            }
        }
    }
}
