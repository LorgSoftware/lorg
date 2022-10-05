#include "lorg.hpp"

#include <iostream>
#include <stack>

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

inline bool is_whitespace(char const & c)
{
    return c == ' ' || c == '\t';
}

inline bool is_end_of_line(char const & c)
{
    return c == '\n' || c == '\0';
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
    while(!is_end_of_line(stream.peek()))
    {
        stream.get();
    }
}

std::string format_error(std::string const message, int line, int column = 0)
{
    std::string error_message = "Line " + std::to_string(line);
    if(column != 0)
    {
        error_message += ", column " + std::to_string(column);
    }
    error_message += ": " + message;
    return error_message;
}

ParserResult lorg::parse(std::string const & content)
{
    ParserResult result;
    result.has_error = false;

    result.total_node.title = "TOTAL";

    StringStream stream(content);

    // Contain the node currently being parsed. We use a stack to avoid
    // unnecessary recursion. The stack size represents the level of the node
    // on top. The node below it is its direct parent.
    std::stack<Node> nodes_to_add;

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
            // Keep the current line because maybe the node definition is
            // ill-formed, and when we detect it the stream is pointing to the
            // next line.
            int current_line = stream.line;

            // Get node level.
            size_t level = 0;
            while(c == NODE_DEFINITION_CHARACTER)
            {
                level++;
                c = stream.get();
            }

            // Get node title.
            if(is_whitespace(c))
            {
                skip_whitespaces(stream);
            }
            c = stream.get();
            if(is_end_of_line(c))
            {
                result.has_error = true;
                result.error_message = format_error("The node has no title.", current_line);
                return result;
            }
            std::string title;
            int trailing_space_count = 0;
            while(!is_end_of_line(c))
            {
                title.push_back(c);
                if(is_whitespace(c))
                {
                    trailing_space_count++;
                }
                else
                {
                    trailing_space_count = 0;
                }
                c = stream.get();
            }
            if(trailing_space_count > 0)
            {
                title.resize(title.size() - trailing_space_count);
            }

            // Manage hierarchy.
            if(level > nodes_to_add.size() + 1)
            {
                result.has_error = true;
                result.error_message = format_error(
                    "The node is not a direct descendant to any other node.", current_line
                );
                return result;
            }
            while(level < nodes_to_add.size() + 1)
            {
                // Moving the siblings and nephews until the top of the stack
                // is the direct parent of the current node.
                Node other = nodes_to_add.top();
                nodes_to_add.pop();
                if(nodes_to_add.size() > 0)
                {
                    nodes_to_add.top().children.push_back(other);
                }
                else
                {
                    result.total_node.children.push_back(other);
                }
            }
            Node current_node;
            current_node.title = title;
            nodes_to_add.push(current_node);
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

    if(!nodes_to_add.empty())
    {
        while(nodes_to_add.size() > 1)
        {
            Node other = nodes_to_add.top();
            nodes_to_add.pop();
            nodes_to_add.top().children.push_back(other);
        }
        result.total_node.children.push_back(nodes_to_add.top());
        nodes_to_add.pop();
    }

    return result;
}
