#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>

#include "lorg.hpp"

constexpr int EXIT_CODE_OK = 0;
constexpr int EXIT_CODE_ERROR_ARGUMENTS = 1;
constexpr int EXIT_CODE_ERROR_PARSE = 2;

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

    // Print the result
    std::stack<lorg::Node> nodes_to_print;
    nodes_to_print.push(result.total_node);

    while(!nodes_to_print.empty())
    {
        auto node = nodes_to_print.top();
        nodes_to_print.pop();

        std::cout << "Title: " << node.title << std::endl;
        for(auto const & unit_pair : node.units)
        {
            std::cout << "  " << unit_pair.second.name << " = " << unit_pair.second.value << std::endl;
        }

        for(int i = node.children.size() - 1; i >= 0; i--)
        {
            auto child = node.children[i];
            nodes_to_print.push(child);
        }
    }

    return EXIT_CODE_OK;
}
