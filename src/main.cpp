#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>

#include "lorg.hpp"

constexpr int EXIT_CODE_OK = 0;
constexpr int EXIT_CODE_ERROR_ARGUMENTS = 1;
constexpr int EXIT_CODE_ERROR_PARSE = 2;

// Container used to print the nodes. The goal is to avoid using recursion. The
// structure contains information that would have been send as function
// parameters if it was done in a recursive way.
struct PrintContainer
{
    lorg::Node const & node;
    int const level;

    PrintContainer(lorg::Node const & node, int const level):
        node(node), level(level)
    {
    }
};

int main(int argc, char* argv[])
{
    // Check the argument count.
    if(argc < 2)
    {
        std::cerr << "Need a file as an argument" << std::endl;
        exit(EXIT_CODE_ERROR_ARGUMENTS);
    }

    // Check if file exists.
    auto filepath = argv[1];
    {
        std::filesystem::path f(filepath);
        if(!std::filesystem::exists(f))
        {
            std::cerr << "\"" << filepath << "\" does not exist." << std::endl;
            exit(EXIT_CODE_ERROR_ARGUMENTS);
        }
        std::filesystem::file_status status = std::filesystem::status(f);
        if(status.type() != std::filesystem::file_type::regular)
        {
            std::cerr << "\"" << filepath << "\" is not a valid file." << std::endl;
            exit(EXIT_CODE_ERROR_ARGUMENTS);
        }
    }

    // Get the file content.
    std::string content;
    {
        std::ifstream f(filepath, std::ios::in);
        char c;
        while(f.get(c))
        {
            content.push_back(c);
        }
    }

    // Parse the content.
    lorg::ParserResult result = lorg::parse(content);
    if(result.has_error)
    {
        std::cerr << result.error_message << std::endl;
        exit(EXIT_CODE_ERROR_PARSE);
    }

    // Print the result.
    {
        std::stack<PrintContainer> nodes_to_print;
        nodes_to_print.push(PrintContainer(result.total_node, 1));
        while(!nodes_to_print.empty())
        {
            PrintContainer current = nodes_to_print.top();
            nodes_to_print.pop();
            lorg::Node const & node = current.node;
            int const & level = current.level;

            std::string indentation;
            // NOTE: could we use a dynamic string or a map of levels instead
            // of looping?
            for(int i = 0; i < level - 1; i++)
            {
                indentation += "  ";
            }

            // Print the title.
            std::cout << indentation;
            for(int i = 0; i < level; i++)
            {
                std::cout << '#';
            }
            std::cout << " " << node.title << std::endl;

            // Print the units.
            for(auto const & unit_pair : node.units)
            {
                lorg::Unit const & unit = unit_pair.second;
                std::cout << indentation << "  ";
                std::cout << "$ " << unit.name << ": " << unit.value << std::endl;
            }

            // Add the children for printing.
            for(auto it = node.children.crbegin(); it != node.children.crend(); it++)
            {
                auto const & child = *it;
                nodes_to_print.push(PrintContainer(child, level + 1));
            }
        }
    }

    return EXIT_CODE_OK;
}
