using System;
using System.Collections.Immutable;
using System.Text;

namespace CodeGenerator.Parsing
{
    /// <summary>
    /// Parses a string into a list of attributes
    /// </summary>
    public ref struct AttributeParser
    {
        private const char KeyDelimiter = '=';
        private const char ListDelimiter = ',';

        private const char QuotedStringOpen = '\"';
        private const char QuotedStringClose = '\"';

        private const char ListOpen = '[';
        private const char ListClose = ']';

        private const char EscapeCharacterStart = '\\';

        private static readonly ImmutableHashSet<char> Delimiters = ImmutableHashSet.Create(
            KeyDelimiter, ListDelimiter, QuotedStringOpen, QuotedStringClose, ListOpen, ListClose);

        internal readonly ReadOnlySpan<char> _text;

        internal int _index;

        public AttributeParser(ReadOnlySpan<char> text)
        {
            _text = text;
            _index = 0;
        }

        public static ImmutableList<CodeAttribute> ParseText(ReadOnlySpan<char> text)
        {
            return new AttributeParser(text).Parse();
        }

        public ImmutableList<CodeAttribute> Parse()
        {
            var builder = ImmutableList.CreateBuilder<CodeAttribute>();

            while (true)
            {
                var state = CaptureState();

                SkipWhitespace();

                if (!HasMoreText())
                {
                    break;
                }

                var attribute = ParseAttribute();

                builder.Add(attribute);

                VerifyHasDelimiterOrEnd(state, ListDelimiter);
            }

            return builder.ToImmutable();
        }

        private CodeAttribute ParseAttribute()
        {
            var state = CaptureState();

            var (key, hasDelimiter) = ParseKey();

            SkipWhitespace();

            var values = ImmutableList<CodeAttribute>.Empty;

            if (hasDelimiter)
            {
                VerifyHasMoreText(state);
                values = ParseValues();
            }

            return new CodeAttribute(key, values);
        }

        private (string Key, bool HasDelimiter) ParseKey()
        {
            var state = CaptureState();
            var token = NextToken();

            bool hasDelimiter = ConsumeDelimiter(KeyDelimiter);

            var key = token.Value.Trim().ToString();

            if (key.Length == 0)
            {
                throw new AttributeParseException($"Got empty attribute key while parsing token at {state.Index}");
            }

            return (key, hasDelimiter);
        }

        private ImmutableList<CodeAttribute> ParseValues()
        {
            if (_text[_index] == ListOpen)
            {
                ++_index;
                return ParseListValue();
            }
            else if (_text[_index] == QuotedStringOpen)
            {
                return ImmutableList.Create(ParseQuotedStringValue());
            }
            else
            {
                return ImmutableList.Create(ParseUnquotedStringValue());
            }
        }

        private CodeAttribute ParseUnquotedStringValue()
        {
            var state = CaptureState();
            var token = NextToken();

            var value = token.Value.Trim().ToString();

            if (value.Length == 0)
            {
                throw new AttributeParseException($"Got empty attribute value while parsing token at {state.Index} (use quoted strings to specify empty values)");
            }

            return new CodeAttribute(value);
        }

        private CodeAttribute ParseQuotedStringValue()
        {
            var state = CaptureState();

            ++_index;

            var token = NextToken();

            VerifyHasMoreText(state);
            VerifyHasDelimiter(state, QuotedStringClose);

            //Don't trim this! User has to do that
            var value = token.Value.ToString();

            //Allowed to be empty

            return new CodeAttribute(value);
        }

        private ImmutableList<CodeAttribute> ParseListValue()
        {
            var builder = ImmutableList.CreateBuilder<CodeAttribute>();

            while (true)
            {
                var state = CaptureState();

                SkipWhitespace();

                VerifyHasMoreText(state);

                char c = _text[_index];

                if (c == ListClose)
                {
                    ++_index;
                    break;
                }

                var attribute = ParseAttribute();

                builder.Add(attribute);

                if (!ConsumeDelimiter(ListDelimiter))
                {
                    VerifyHasDelimiter(state, ListClose, false);
                }
            }

            return builder.ToImmutable();
        }

        private AttributeParseState CaptureState()
        {
            return new AttributeParseState(ref this);
        }

        private void SkipWhitespace()
        {
            for (; _index < _text.Length; ++_index)
            {
                if (!char.IsWhiteSpace(_text[_index]))
                {
                    break;
                }
            }
        }

        private Token NextToken()
        {
            var state = CaptureState();

            int startIndex = _index;
            int? endIndex = null;

            StringBuilder? builder = null;

            for (; _index < _text.Length; ++_index)
            {
                char c = _text[_index];

                if (c == EscapeCharacterStart)
                {
                    ++_index;

                    VerifyHasMoreText(state);

                    c = _text[_index];

                    var escaped = TryParseEscapeCharacter(c);

                    if (escaped is null)
                    {
                        throw new AttributeParseException($"Invalid escape sequence \"\\{c}\" at index {_index - 1} while parsing token at index {state.Index}");
                    }

                    //Switch to string builder
                    builder ??= new(_text[startIndex..(_index - 1)].ToString());

                    builder.Append(escaped.Value);
                }
                else if (Delimiters.Contains(c))
                {
                    endIndex = _index;
                    break;
                }
                else if (builder is not null)
                {
                    builder.Append(c);
                }
            }

            if (builder is not null)
            {
                return new Token(builder.ToString());
            }

            return new Token(_text[startIndex..(endIndex ?? _index)]);
        }

        /// <summary>
        /// Tries to parse a character as its equivalent escape character
        /// This is a sub-set of standard escape characters
        /// </summary>
        /// <param name="c"></param>
        private static char? TryParseEscapeCharacter(char c)
        {
            return c switch
            {
                'r' => '\r',
                'n' => '\n',
                't' => '\t',
                'b' => '\b',
                'f' => '\f',
                '\\' => '\\',
                '\'' => '\'',
                '\"' => '\"',
                _ => null,
            };
        }

        private bool HasMoreText()
        {
            return _index < _text.Length;
        }

        private void VerifyHasMoreText(in AttributeParseState state)
        {
            if (!HasMoreText())
            {
                throw new AttributeParseException($"Unexpected end of text encountered while parsing token at {state.Index}");
            }
        }

        private bool HasDelimiter(char delimiter)
        {
            SkipWhitespace();

            if (_index >= _text.Length)
            {
                return false;
            }

            char c = _text[_index];

            return c == delimiter;
        }

        private bool ConsumeDelimiter(char delimiter)
        {
            if (HasDelimiter(delimiter))
            {
                ++_index;
                return true;
            }

            return false;
        }

        private void VerifyHasDelimiter(in AttributeParseState state, char delimiter, bool consumeDelimiter = true)
        {
            SkipWhitespace();

            VerifyHasMoreText(state);

            VerifyHasDelimiterCore(state, delimiter, consumeDelimiter);
        }

        private void VerifyHasDelimiterCore(in AttributeParseState state, char delimiter, bool consumeDelimiter = true)
        {
            char c = _text[_index];

            if (c != delimiter)
            {
                throw new AttributeParseException($"Expected \"{delimiter}\", got \"{c}\" while parsing token at {state.Index}");
            }

            if (consumeDelimiter)
            {
                ++_index;
            }
        }

        private void VerifyHasDelimiterOrEnd(in AttributeParseState state, char delimiter, bool consumeDelimiter = true)
        {
            SkipWhitespace();

            if (_index >= _text.Length)
            {
                return;
            }

            VerifyHasDelimiterCore(state, delimiter, consumeDelimiter);
        }
    }
}
