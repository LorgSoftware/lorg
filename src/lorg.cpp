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
            return '\0';
        }
        else
        {
            return s[index];
        }
    }
};

bool is_whitespace(char const & c)
{
    return c == ' ' || c == '\t';
}

void skip_whitespaces(StringStream & stream)
{
    // No need to check EOF because stream returns '\0' when EOF.
    while(is_whitespace(stream.peek()))
    {
        stream.get();
    }
}

void skip_line(StringStream & stream)
{
    while(!stream.eof() && stream.peek() != '\n')
    {
        stream.get();
    }
}

ParserResult lorg::parse(std::string const & content)
{
    ParserResult result;
    result.has_error = false;

    result.total_node.title = "TOTAL";

    StringStream stream(content);

    while(!stream.eof())
    {
        // Skip useless possible white spaces at the beginning of the line.
        if(stream.column == 0 && is_whitespace(stream.peek()))
        {
            skip_whitespaces(stream);
            if(stream.eof())
            {
                break;
            }
        }

        char c = stream.get();

        if(c == NODE_DEFINITION_CHARACTER)
        {
            std::cout << "Manage node definition at line " << stream.line << std::endl;
            skip_line(stream);  // TODO: delete
        }
        else if(c == UNIT_DEFINITION_CHARACTER)
        {
            std::cout << "Manage unit definition at line " << stream.line << std::endl;
            skip_line(stream);  // TODO: delete
        }
        else if(c == '\n')
        {
            continue;
        }
        else
        {
            skip_line(stream);
        }
    }

    return result;
}
