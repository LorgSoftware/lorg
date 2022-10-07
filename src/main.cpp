#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>
#include <string>

#include "lorg.hpp"

constexpr int EXIT_CODE_OK = 0;
constexpr int EXIT_CODE_ERROR_ARGUMENTS = 1;
constexpr int EXIT_CODE_ERROR_PARSE = 2;

struct Config
{
    bool hide_ignored = false;
    bool hide_ignored_and_calculated = false;
    bool display_total_node = true;
};

struct CommandArguments
{
    std::string filepath;
    Config config;
};

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

// Useful for comparing "argv[i]" with a litteral string.
bool are_equal(char const * const str1, std::string && str2)
{
    return str2.compare(str1) == 0;
}

CommandArguments parse_command_arguments_or_exit(int argc, char const * const argv[])
{
    // Check the argument count.
    if(argc < 2)
    {
        std::cerr << "Need a file as an argument" << std::endl;
        exit(EXIT_CODE_ERROR_ARGUMENTS);
    }

    CommandArguments arguments;
    Config & config = arguments.config;

    int i = 1;
    while(i < argc)
    {
        if(are_equal(argv[i], "--no-ignored") || are_equal(argv[i], "-nig"))
        {
            config.hide_ignored = true;
        }
        else if(are_equal(argv[i], "--no-ignored-and-calculated") || are_equal(argv[i], "-nic"))
        {
            config.hide_ignored_and_calculated = true;
        }
        else if(are_equal(argv[i], "--no-total") || are_equal(argv[i], "-nt"))
        {
            config.display_total_node = false;
        }
        else
        {
            if(arguments.filepath.empty())
            {
                arguments.filepath = argv[i];
            }
            else
            {
                std::cerr << "Only one file at a time can be parsed." << std::endl;
                exit(EXIT_CODE_ERROR_ARGUMENTS);
            }
        }
        i++;
    }

    return arguments;
}

std::string get_file_content_or_exit(std::string const filepath)
{
    // Check if file exists.
    {
        std::filesystem::path path(filepath);
        if(!std::filesystem::exists(path))
        {
            std::cerr << "\"" << filepath << "\" does not exist." << std::endl;
            exit(EXIT_CODE_ERROR_ARGUMENTS);
        }
        std::filesystem::file_status status = std::filesystem::status(path);
        if(status.type() != std::filesystem::file_type::regular)
        {
            std::cerr << "\"" << filepath << "\" is not a valid file." << std::endl;
            exit(EXIT_CODE_ERROR_ARGUMENTS);
        }
    }

    // Get the file content.
    std::string content;
    {
        std::ifstream stream(filepath, std::ios::in);
        char c;
        while(stream.get(c))
        {
            content.push_back(c);
        }
    }
    return content;
}

void print_simple(std::vector<lorg::Node*> const root_nodes, Config const & config)
{
    std::stack<PrintContainer> nodes_to_print;
    for(auto it = root_nodes.crbegin(); it != root_nodes.crend(); it++)
    {
        nodes_to_print.push(PrintContainer(**it, 1));
    }
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
            if(
                (unit.is_ignored && config.hide_ignored) ||
                ((unit.is_ignored && !unit.is_real) && config.hide_ignored_and_calculated)
            )
            {
                continue;
            }
            std::cout << indentation << "  ";
            std::cout << "$ " << unit.name << ": " << unit.value;
            if(!unit.is_real)
            {
                std::cout << " [Calculated]";
            }
            if(unit.is_ignored)
            {
                std::cout << " [Ignored]";
            }
            std::cout << std::endl;
        }

        // Add the children for printing.
        for(auto it = node.children.crbegin(); it != node.children.crend(); it++)
        {
            auto const & child = *it;
            nodes_to_print.push(PrintContainer(child, level + 1));
        }
    }
}

int main(int argc, char* argv[])
{
    CommandArguments arguments = parse_command_arguments_or_exit(argc, argv);
    std::string content = get_file_content_or_exit(arguments.filepath);

    // Parse the content.
    lorg::ParserResult result = lorg::parse(content);
    if(result.has_error)
    {
        std::cerr << result.error_message << std::endl;
        exit(EXIT_CODE_ERROR_PARSE);
    }

    // Print the result.
    Config const & config = arguments.config;
    std::vector<lorg::Node *> root_nodes;
    if(config.display_total_node)
    {
        root_nodes.push_back(&(result.total_node));
    }
    else
    {
        for(auto & child : result.total_node.children)
        {
            root_nodes.push_back(&child);
        }
    }
    print_simple(root_nodes, config);

    return EXIT_CODE_OK;
}
