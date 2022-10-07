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
    bool add_indent = true;
    bool prettify = false;
    std::string total_name = "TOTAL";
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

    // "has_next_sibling" and "prefix_from_parent" are useful for pretty print.
    bool const has_next_sibling;
    std::string const prefix_from_parent;

    PrintContainer(
        lorg::Node const & node, int const level,
        bool has_next_sibling=false, std::string prefix_from_parent=""
    ):
        node(node), level(level),
        has_next_sibling(has_next_sibling), prefix_from_parent(prefix_from_parent)
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
        else if(are_equal(argv[i], "--no-indent") || are_equal(argv[i], "-nin"))
        {
            config.add_indent = false;
        }
        else if(are_equal(argv[i], "--prettify") || are_equal(argv[i], "-p"))
        {
            config.prettify = true;
        }
        else if(are_equal(argv[i], "--total-name") || are_equal(argv[i], "-tn"))
        {
            i++;
            if(i < argc)
            {
                arguments.config.total_name = argv[i];
            }
            else
            {
                std::cerr << "The option " << argv[i-1] << " requires an argument." << std::endl;
                exit(EXIT_CODE_ERROR_ARGUMENTS);
            }
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

inline bool should_hide_unit(lorg::Unit const & unit, Config const & config)
{
    return (
        (unit.is_ignored && config.hide_ignored) ||
        ((unit.is_ignored && !unit.is_real) && config.hide_ignored_and_calculated)
    );
}

// We do not want to override the "<<" operator just for that. It makes
// semantically no sense.
void cout_unit(std::ostream & o, lorg::Unit const & unit)
{
    o << "$ " << unit.name << ": " << unit.value;
    if(!unit.is_real)
    {
        o << " [Calculated]";
    }
    if(unit.is_ignored)
    {
        o << " [Ignored]";
    }
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
        if(config.add_indent)
        {
            for(int i = 0; i < level - 1; i++)
            {
                indentation += "  ";
            }
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
            if(should_hide_unit(unit, config))
            {
                continue;
            }
            if(config.add_indent)
            {
                std::cout << indentation << "  ";
            }
            cout_unit(std::cout, unit);
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

void print_pretty(std::vector<lorg::Node*> const root_nodes, Config const & config)
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
        bool const & has_next_sibling = current.has_next_sibling;
        std::string const & prefix_from_parent = current.prefix_from_parent;

        // Print the title.
        if(level == 1)
        {
            std::cout << node.title << std::endl;
        }
        else
        {
            if(has_next_sibling)
            {
                std::cout << prefix_from_parent << "├── " << node.title << std::endl;
            }
            else
            {
                std::cout << prefix_from_parent << "└── " << node.title << std::endl;
            }
        }

        // Set up the prefix for the children.
        std::string prefix_for_next_lines;
        if(level > 1)
        {
            std::string to_add = has_next_sibling ? "│   " : "    ";
            prefix_for_next_lines = prefix_from_parent + to_add;
        }

        // Print the units.
        for(auto const & unit_pair : node.units)
        {
            lorg::Unit const & unit = unit_pair.second;
            if(should_hide_unit(unit, config))
            {
                continue;
            }
            if(node.children.empty())
            {
                std::cout << prefix_for_next_lines << "  ";
            }
            else
            {
                std::cout << prefix_for_next_lines << "│ ";
            }
            cout_unit(std::cout, unit);
            std::cout << std::endl;
        }

        // Add the children for printing.
        if(!node.children.empty())
        {
            auto it = node.children.crbegin();
            nodes_to_print.push(PrintContainer(*it, level + 1, false, prefix_for_next_lines));
            it++;
            for(; it != node.children.crend(); it++)
            {
                auto const & child = *it;
                nodes_to_print.push(PrintContainer(child, level + 1, true, prefix_for_next_lines));
            }
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
    Config const & config = arguments.config;
    result.total_node.title = config.total_name;

    // Print the result.
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
    if(config.prettify)
    {
        print_pretty(root_nodes, config);
    }
    else
    {
        print_simple(root_nodes, config);
    }

    return EXIT_CODE_OK;
}
