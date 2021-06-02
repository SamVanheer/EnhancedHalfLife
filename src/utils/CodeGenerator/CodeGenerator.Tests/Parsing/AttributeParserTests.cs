using CodeGenerator.Parsing;
using Xunit;

namespace CodeGenerator.Tests.Parsing
{
    public class AttributeParserTests
    {
        [Theory]
        [InlineData("")]
        [InlineData(" ")]
        [InlineData("   ")]
        [InlineData("\t")]
        [InlineData("\n")]
        [InlineData("\r")]
        public void Parse_EmptyOrWhitespaceString_ReturnsEmptyList(string text)
        {
            var parser = new AttributeParser(text);

            var attributes = parser.Parse();

            Assert.Empty(attributes);
        }

        [Theory]
        [InlineData("Key")]
        [InlineData(" Key")]
        [InlineData("Key ")]
        [InlineData(" Key ")]
        public void Parse_Key_ReturnsOneElement(string text)
        {
            var parser = new AttributeParser(text);

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);
            Assert.Empty(attribute.Values);
        }

        [Fact]
        public void Parse_KeyValue_ReturnsOneElement()
        {
            var parser = new AttributeParser("Key=Value");

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var value = Assert.Single(attribute.Values);
            Assert.Equal("Value", value.Name);
            Assert.Empty(value.Values);
        }

        [Fact]
        public void Parse_KeyEmptyQuotedValue_ReturnsOneElement()
        {
            var parser = new AttributeParser("Key=\"\"");

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var value = Assert.Single(attribute.Values);
            Assert.Equal("", value.Name);
            Assert.Empty(value.Values);
        }

        [Fact]
        public void Parse_KeyQuotedValue_ReturnsOneElement()
        {
            var parser = new AttributeParser("Key=\"Value with quotes \"");

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var value = Assert.Single(attribute.Values);
            Assert.Equal("Value with quotes ", value.Name);
            Assert.Empty(value.Values);
        }

        [Theory]
        [InlineData("Key1,Key2")]
        [InlineData("Key1, Key2")]
        [InlineData("Key1 ,Key2")]
        [InlineData("Key1 , Key2")]
        [InlineData(" Key1 , Key2 ")]
        public void Parse_PlainKeyList_ReturnsListOfAttributes(string text)
        {
            var parser = new AttributeParser(text);

            var attributes = parser.Parse();

            Assert.Collection(attributes,
                first =>
                {
                    Assert.Equal("Key1", first.Name);
                    Assert.Empty(first.Values);
                },
                second =>
                {
                    Assert.Equal("Key2", second.Name);
                    Assert.Empty(second.Values);
                });
        }

        [Theory]
        [InlineData("Key=[]")]
        [InlineData("Key= []")]
        [InlineData("Key=[ ]")]
        [InlineData("Key=[] ")]
        public void Parse_KeyEmptyList_ReturnsOneElementWithEmptyList(string text)
        {
            var parser = new AttributeParser(text);

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);
            Assert.Empty(attribute.Values);
        }

        [Theory]
        [InlineData("Key=[Element]")]
        [InlineData("Key= [Element]")]
        [InlineData("Key=[ Element]")]
        [InlineData("Key=[Element ]")]
        [InlineData("Key=[Element] ")]
        public void Parse_KeyListValueOneElement_ReturnsOneElementWithList(string text)
        {
            var parser = new AttributeParser(text);

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var element = Assert.Single(attribute.Values);
            Assert.Equal("Element", element.Name);
            Assert.Empty(element.Values);
        }

        [Theory]
        [InlineData("Key=[Element=Value]")]
        [InlineData("Key= [Element=Value]")]
        [InlineData("Key=[ Element=Value]")]
        [InlineData("Key=[Element=Value ]")]
        [InlineData("Key=[Element=Value] ")]
        [InlineData("Key=[Element =Value]")]
        [InlineData("Key=[Element= Value]")]
        [InlineData("Key=[Element = Value]")]
        [InlineData("Key=[ Element = Value ]")]
        [InlineData("Key = [ Element = Value ] ")]
        public void Parse_KeyListValueOneElementWithValue_ReturnsOneElementWithList(string text)
        {
            var parser = new AttributeParser(text);

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var element = Assert.Single(attribute.Values);
            Assert.Equal("Element", element.Name);

            var value = Assert.Single(element.Values);
            Assert.Equal("Value", value.Name);
            Assert.Empty(value.Values);
        }

        [Theory]
        [InlineData("Key=[Element=[NestedElement]]")]
        [InlineData("Key= [Element=[NestedElement]]")]
        [InlineData("Key=[ Element=[NestedElement]]")]
        [InlineData("Key=[Element=[NestedElement] ]")]
        [InlineData("Key=[Element=[NestedElement]] ")]
        [InlineData("Key=[Element= [NestedElement]]")]
        [InlineData("Key=[Element=[ NestedElement]]")]
        [InlineData("Key=[Element=[NestedElement ]]")]
        [InlineData("Key=[Element=[ NestedElement ]]")]
        [InlineData("Key=[Element= [ NestedElement ]]")]
        [InlineData("Key=[ Element=[NestedElement] ]")]
        [InlineData("Key=[Element= [ NestedElement ] ]")]
        public void Parse_KeyListValueOneElementWithNestedList_ReturnsOneElementWithNestedList(string text)
        {
            var parser = new AttributeParser(text);

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var element = Assert.Single(attribute.Values);
            Assert.Equal("Element", element.Name);

            var nestedElement = Assert.Single(element.Values);
            Assert.Equal("NestedElement", nestedElement.Name);
            Assert.Empty(nestedElement.Values);
        }

        [Theory]
        [InlineData("Key=[Element=[NestedElement=Value]]")]
        public void Parse_KeyListValueOneElementWithNestedListAndKeyValue_ReturnsOneElementWithNestedListAndKeyValue(string text)
        {
            var parser = new AttributeParser(text);

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var element = Assert.Single(attribute.Values);
            Assert.Equal("Element", element.Name);

            var nestedElement = Assert.Single(element.Values);
            Assert.Equal("NestedElement", nestedElement.Name);

            var value = Assert.Single(nestedElement.Values);
            Assert.Equal("Value", value.Name);
            Assert.Empty(value.Values);
        }

        [Theory]
        [InlineData("Key=[Element=\"\"]")]
        [InlineData("Key= [Element=\"\"]")]
        [InlineData("Key=[ Element=\"\"]")]
        [InlineData("Key=[Element=\"\" ]")]
        [InlineData("Key=[Element=\"\"] ")]
        public void Parse_KeyListValueOneElementWithEmptyQuotedValue_ReturnsOneElementWithListAndEmptyQuotedValue(string text)
        {
            var parser = new AttributeParser(text);

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var element = Assert.Single(attribute.Values);
            Assert.Equal("Element", element.Name);

            var value = Assert.Single(element.Values);
            Assert.Equal("", value.Name);
            Assert.Empty(value.Values);
        }

        [Theory]
        [InlineData("Key=[Element=\" Value \"]")]
        [InlineData("Key= [Element=\" Value \"]")]
        [InlineData("Key=[ Element=\" Value \"]")]
        [InlineData("Key=[Element=\" Value \" ]")]
        [InlineData("Key=[Element=\" Value \"] ")]
        public void Parse_KeyListValueOneElementWithQuotedValue_ReturnsOneElementWithListAndQuotedValue(string text)
        {
            var parser = new AttributeParser(text);

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var element = Assert.Single(attribute.Values);
            Assert.Equal("Element", element.Name);

            var value = Assert.Single(element.Values);
            Assert.Equal(" Value ", value.Name);
            Assert.Empty(value.Values);
        }

        [Fact]
        public void Parse_KeyValueWithEscapeCharacters_ReturnsOneElementWithEscapedValue()
        {
            var parser = new AttributeParser(@"Key=""\r\n\t\b\f\\\'\""""");

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var element = Assert.Single(attribute.Values);
            Assert.Equal("\r\n\t\b\f\\\'\"", element.Name);
        }

        [Fact]
        public void Parse_KeyValueWithEscapedQuotes_ReturnsOneElementWithQuotedString()
        {
            var parser = new AttributeParser(@"Key=""\""Quoted string\""""");

            var attributes = parser.Parse();

            var attribute = Assert.Single(attributes);
            Assert.Equal("Key", attribute.Name);

            var element = Assert.Single(attribute.Values);
            Assert.Equal("\"Quoted string\"", element.Name);
        }

        [Theory]
        [InlineData("=")]
        [InlineData(",")]
        [InlineData("\"")]
        [InlineData("[")]
        [InlineData("]")]
        public void Parse_Delimiter_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }

        [Theory]
        [InlineData("Key=")]
        [InlineData("Key= ")]
        public void Parse_KeyEqualsNoValue_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }

        [Theory]
        [InlineData("=Value")]
        [InlineData(" =Value")]
        public void Parse_EmptyKey_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }

        [Theory]
        [InlineData("=")]
        [InlineData(" =")]
        [InlineData(" = ")]
        [InlineData("= ")]
        public void Parse_EmptyKeyAndValue_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }

        [Theory]
        [InlineData("Key=\"")]
        [InlineData("Key= \"")]
        [InlineData("Key=\" ")]
        [InlineData("Key=\"Value")]
        public void Parse_UnclosedQuote_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }

        [Theory]
        [InlineData("Key=[Element")]
        [InlineData("Key=[Element,")]
        [InlineData("Key=[Element ")]
        [InlineData("Key=[Element, ")]
        public void Parse_KeyUnclosedList_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }

        [Theory]
        [InlineData("Key=[Element=]")]
        [InlineData("Key=[Element= ]")]
        public void Parse_KeyListElementEqualsNoValue_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }

        [Theory]
        [InlineData("Key=[, Element]")]
        [InlineData("Key=[Element,,Element2]")]
        [InlineData("Key=[Element, , Element2]")]
        [InlineData("Key=[Element, ,]")]
        public void Parse_KeyListElementEmptyElement_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }

        [Theory]
        [InlineData("[Element]")]
        [InlineData("=[Element]")]
        [InlineData("[]")]
        public void Parse_ListWithNoKey_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }

        [Theory]
        [InlineData("Key[Value")]
        [InlineData("Key]Value")]
        [InlineData("Key\"Value")]
        [InlineData("Key=]Element")]
        [InlineData("Key==Value")]
        [InlineData("Key=,Value]")]
        [InlineData("Key=[Value[")]
        public void Parse_InvalidUseOfDelimiters_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }

        [Theory]
        [InlineData(@"Key=\")]
        [InlineData(@"Key=\v")]
        [InlineData(@"Key=\ v")]
        [InlineData(@"Key=\a")]
        [InlineData(@"Key=\q")]
        [InlineData(@"Key=\1")]
        [InlineData(@"Key=\FF")]
        public void Parse_InvalidEscapeSequence_ThrowsException(string text)
        {
            Assert.Throws<AttributeParseException>(() =>
            {
                var parser = new AttributeParser(text);
                parser.Parse();
            });
        }
    }
}
