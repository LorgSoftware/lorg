#include "lorg.hpp"

#include <iostream>

using namespace lorg;

/*
 * Stream to a given string, and keeps the column and the line position
 * synchronized with the current character the stream is currently pointing to.
 */
struct StringStream
{
    // The line of the last character returned by "get()".
    int line;

    // The column of the last character returned by "get()".
    int column;

    // The current look up character index in the string.
    size_t index;

    std::string const & s;

    StringStream(std::string const & string_reference):
        line(1),
        column(0),
        index(0),
        s(string_reference)
    {
    }

    bool eof() noexcept
    {
        return index >= s.size();
    }

    char get() noexcept
    {
        if(eof())
        {
            // NOTE: we do not manage the case really well here...
            return '\0';
        }
        char c = s[index];
        if(c == '\n')
        {
            line++;
            column = 0;
        }
        else
        {
            column++;
        }
        index++;
        return c;
    }

    char peek() noexcept
    {
        if(eof())
        {
            // NOTE: we do not manage the case really well here...
            return '\0';
        }
        else
        {
            return s[index];
        }
    }
};

bool is_char_whitespace(char const & c)
{
    return c == ' ' || c == '\t';
}

ParserResult lorg::parse(std::string const & content)
{
    ParserResult result;
    result.has_error = false;

    result.total_node.title = "TOTAL";

    StringStream stream(content);

    while(!stream.eof())
    {
        char c = stream.get();
        // TODO
    }

    return result;
}
