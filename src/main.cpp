#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>

#include "lorg.hpp"

#define VERSION "1.0"

// Indentation step for printing prettily JSON.
#define INDENTATION_STEP "    "

constexpr int EXIT_CODE_OK = 0;
constexpr int EXIT_CODE_ERROR_ARGUMENTS = 1;
constexpr int EXIT_CODE_ERROR_PARSE = 2;

struct Config
{
    bool print_version = false;
    bool display_total_node = false;
    bool prettify = false;
    bool to_json = false;
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

std::string escape_json(std::string const & str)
{
    std::string escaped;
    for(char const & c : str)
    {
        if(c == '"')
        {
            escaped.append("\\\"");
        }
        else if(c == '\\')
        {
            escaped.append("\\\\");
        }
        else if(c == '\n')
        {
            escaped.append("\\n");
        }
        else if(c == '\r')
        {
            escaped.append("\\r");
        }
        else if(c == '\t')
        {
            escaped.append("\\t");
        }
        else if('\x00' <= c && c <= '\x1f')
        {
            escaped.append("\\u00");
            if('\x00' <= c && c < '\x10')
            {
                escaped.push_back('0');
            }
            std::ostringstream os;
            os << std::hex << static_cast<int>(c);
            for(char const & oc : os.str())
            {
                escaped.push_back(oc);
            }
        }
        else
        {
            escaped.push_back(c);
        }
    }
    return escaped;
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
        if(std::strlen(argv[i]) > 1 && argv[i][0] == '-' && argv[i][1] != '-')
        {
            size_t length = std::strlen(argv[i]);
            for(size_t n = 1; n < length; n++)
            {
                char const & c = argv[i][n];
                if(c =='v')
                {
                    config.print_version = true;
                }
                else if(c =='t')
                {
                    config.display_total_node = true;
                }
                else if(c =='p')
                {
                    config.prettify = true;
                }
                else if(c =='j')
                {
                    config.to_json = true;
                }
                else
                {
                    std::cerr << "Unknown option \"-" << c << "\"." << std::endl;
                    exit(EXIT_CODE_ERROR_ARGUMENTS);
                }

            }
        }
        else if(are_equal(argv[i], "--version") || are_equal(argv[i], "-v"))
        {
            config.print_version = true;
        }
        else if(are_equal(argv[i], "--total") || are_equal(argv[i], "-t"))
        {
            config.display_total_node = true;
        }
        else if(are_equal(argv[i], "--prettify") || are_equal(argv[i], "-p"))
        {
            config.prettify = true;
        }
        else if(are_equal(argv[i], "--json") || are_equal(argv[i], "-j"))
        {
            config.to_json = true;
        }
        else
        {
            if(arguments.filepath.empty())
            {
                arguments.filepath = argv[i];
            }
            else
            {
                if(argv[i][0] == '-')
                {
                    std::cerr << "Unknown option \"" << argv[i] << "\"." << std::endl;
                    exit(EXIT_CODE_ERROR_ARGUMENTS);
                }
                else
                {
                    std::cerr << "Only one file at a time can be parsed." << std::endl;
                    exit(EXIT_CODE_ERROR_ARGUMENTS);
                }
            }
        }
        i++;
    }

    return arguments;
}

std::string get_file_content_or_exit(std::string const filepath)
{
    // NOTE: we do not use `filesystem` because this is not at all portable. For
    // some moronic reasons some people thought it was a great idea to sometime
    // put that in `<filesystem>` and sometime in `<experimental/filesystem>`.
    // At the end even if the correct inclusion is done, some components of
    // `filesystem` like `path` does not exist. Or maybe it does but must
    // include in the build process options like `-lstdc++fs` to work; but it
    // did not work when we try.
    // We have other stuff to do in our life rather than fixing some stupid
    // issues like that. We use `cstdio` and the C standard library like any
    // sane person will do.
    FILE* f = std::fopen(filepath.c_str(), "r");

    // Check if file can be read.
    if(f == NULL)
    {
        std::cerr << "\"" << filepath << "\" cannot be read." << std::endl;
        exit(EXIT_CODE_ERROR_ARGUMENTS);
    }

    // Get the file content.
    std::string content;
    {
        int c = std::fgetc(f);
        while(c != EOF)
        {
            content.push_back(static_cast<char>(c));
            c = std::fgetc(f);
        }
    }
    std::fclose(f);
    return content;
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

void print_simple(
    std::vector<lorg::Node const *> const root_nodes,
    std::vector<std::string> const & sorted_unit_names
)
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
        for(auto const & name : sorted_unit_names)
        {
            lorg::Unit const & unit = node.units.at(name);
            std::cout << indentation << "  ";
            cout_unit(std::cout, unit);
            std::cout << std::endl;
        }

        // Add the children for printing.
        for(auto it = node.children.crbegin(); it != node.children.crend(); it++)
        {
            auto const & child = *it;
            nodes_to_print.push(PrintContainer(*child, level + 1));
        }
    }
}

void print_pretty(
    std::vector<lorg::Node const *> const root_nodes,
    std::vector<std::string> const & sorted_unit_names
)
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
        for(auto const & name : sorted_unit_names)
        {
            lorg::Unit const & unit = node.units.at(name);
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
            nodes_to_print.push(PrintContainer(**it, level + 1, false, prefix_for_next_lines));
            it++;
            for(; it != node.children.crend(); it++)
            {
                auto const & child = *it;
                nodes_to_print.push(PrintContainer(*child, level + 1, true, prefix_for_next_lines));
            }
        }
    }
}

inline std::string to_string(bool const v)
{
    return v ? "true" : "false";
}

void print_json_unit(lorg::Unit const & unit)
{
    // NOTE: Instead of escaping that everytime, maybe we should map the unit
    // names with escaped unit names.
    std::string escaped_unit_name = escape_json(unit.name);
    std::cout << "\"" << escaped_unit_name << "\":{";
    std::cout << "\"name\":\"" << escaped_unit_name << "\",";
    std::cout << "\"value\":" << unit.value << ",";
    std::cout << "\"isReal\":" << to_string(unit.is_real) << ",";
    std::cout << "\"isIgnored\":" << to_string(unit.is_ignored);
    std::cout << "}";
}

void print_json_node(
    lorg::Node const & node, std::vector<std::string> const & sorted_unit_names
)
{
    std::cout << "{";
    // Print title.
    std::cout << "\"title\":\"" << escape_json(node.title) << "\"";

    // Print the units.
    std::cout << ",\"units\":{";
    {
        // Needed to manage the last `,`.
        std::string separator = "";
        for(std::string const & name : sorted_unit_names)
        {
            std::cout << separator;
            print_json_unit(node.units.at(name));
            separator = ",";
        }
    }
    std::cout << "}";

    // Print children.
    std::cout << ",\"children\":[";
    if(node.children.size() > 0)
    {
        for(size_t i = 0; i < node.children.size() - 1; i++)
        {
            print_json_node(*(node.children[i]), sorted_unit_names);
            std::cout << ",";
        }
        print_json_node(*(node.children[node.children.size()-1]), sorted_unit_names);
    }
    std::cout << "]";

    std::cout << "}";
}

void print_json(
    std::vector<lorg::Node const *> const root_nodes,
    std::vector<std::string> const & sorted_unit_names
)
{
    // NOTE: This code should be refactored. We avoid in this software
    // recursion because it is not compatible with large data. But for the
    // moment, I do not know how to write this code using stacks (for example).
    // The fact that a node should print something before *and* after its
    // children is the problem.

    std::cout << "[";
    if(root_nodes.size() > 0)
    {
        for(size_t i = 0; i < root_nodes.size() - 1; i++)
        {
            print_json_node(*(root_nodes[i]), sorted_unit_names);
            std::cout << ",";
        }
        print_json_node(*(root_nodes[root_nodes.size()-1]), sorted_unit_names);
    }
    std::cout << "]";
    std::cout << std::endl;
}

void print_json_pretty_node(
    lorg::Node const & node, std::vector<std::string> const & sorted_unit_names,
    std::string const & indentation, bool has_sibling
)
{
    std::string indentation_key = indentation + INDENTATION_STEP;
    std::string indentation_value = indentation + INDENTATION_STEP + INDENTATION_STEP;

    std::cout << indentation << "{" << std::endl;

    // Print title.
    std::cout << indentation_key << "\"title\": \"" << escape_json(node.title) << "\"," << std::endl;

    // Print the units.
    if(node.units.empty())
    {
        std::cout << indentation_key << "\"units\": {}," << std::endl;
    }
    else
    {
        std::cout << indentation_key << "\"units\": {" << std::endl;
        // Needed to manage the last `},`.
        bool is_first = true;
        for(auto const & name : sorted_unit_names)
        {
            lorg::Unit const & unit = node.units.at(name);
            // NOTE: Instead of escaping that everytime, maybe we should map the unit
            // names with escaped unit names.
            std::string escaped_unit_name = escape_json(unit.name);

            std::string const & i = indentation_value;
            std::string const & iv = i + INDENTATION_STEP;

            if(is_first)
            {
                is_first = false;
            }
            else
            {
                // Closing the last sibling unit JSON print.
                std::cout << i << "}," << std::endl;
            }

            std::cout << i << "\"" << escaped_unit_name << "\": {" << std::endl;
            std::cout << iv << "\"name\": \"" << escaped_unit_name << "\"," << std::endl;
            std::cout << iv << "\"value\": " << unit.value << "," << std::endl;
            std::cout << iv << "\"isReal\": " << to_string(unit.is_real) << "," << std::endl;
            std::cout << iv << "\"isIgnored\": " << to_string(unit.is_ignored) << std::endl;
        }
        if(!node.units.empty())
        {
            // Closing the last sibling unit JSON print.
            std::cout << indentation_value << "}" << std::endl;
        }
        std::cout << indentation_key << "}," << std::endl;
    }

    // Print children.
    if(node.children.empty())
    {
        std::cout << indentation_key << "\"children\": []" << std::endl;
    }
    else
    {
        std::cout << indentation_key << "\"children\": [" << std::endl;
        for(auto it = node.children.cbegin(); it < node.children.cend() - 1; it++)
        {
            print_json_pretty_node(**it, sorted_unit_names, indentation_value, true);
        }
        print_json_pretty_node(
            **(node.children.cend()-1), sorted_unit_names, indentation_value, false
        );
        std::cout << indentation_key << "]" << std::endl;
    }

    if(has_sibling)
    {
        std::cout << indentation << "}," << std::endl;
    }
    else
    {
        std::cout << indentation << "}" << std::endl;
    }
}

void print_json_pretty(
    std::vector<lorg::Node const *> const root_nodes,
    std::vector<std::string> const & sorted_unit_names
)
{
    // NOTE: This code should be refactored. We should avoid recursion.

    std::cout << "[" << std::endl;
    if(root_nodes.size() > 0)
    {
        for(auto it = root_nodes.cbegin(); it < root_nodes.cend() - 1; it++)
        {
            print_json_pretty_node(**it, sorted_unit_names, INDENTATION_STEP, true);
        }
        print_json_pretty_node(
            **(root_nodes.cend()-1), sorted_unit_names, INDENTATION_STEP, false
        );
    }
    std::cout << "]" << std::endl;
}

int main(int argc, char* argv[])
{
    CommandArguments arguments = parse_command_arguments_or_exit(argc, argv);
    Config const & config = arguments.config;

    if(config.print_version)
    {
        std::cout << VERSION << std::endl;
        // There is no need to do anything else. It should not be a correct
        // behaviour to use the `--version` option with other options.
        exit(0);
    }

    // Parse the content.
    lorg::ParserResult result;
    {
        // NOTE: We put the content variable into this scope because we get the
        // full content of the file. The file may be very big, and we do not
        // need the content anymore after parsing it.
        std::string content = get_file_content_or_exit(arguments.filepath);
        result = lorg::parse(content);
    }
    if(result.has_error)
    {
        std::cerr << result.error_message << std::endl;
        exit(EXIT_CODE_ERROR_PARSE);
    }
    result.total_node->title = "TOTAL";

    // Print the result.
    std::vector<lorg::Node const *> root_nodes;
    if(config.display_total_node)
    {
        root_nodes.push_back(result.total_node.get());
    }
    else
    {
        for(auto & child : result.total_node->children)
        {
            root_nodes.push_back(child.get());
        }
    }
    std::vector<std::string> sorted_unit_names;
    for(auto unit_pair : result.total_node->units)
    {
        sorted_unit_names.push_back(unit_pair.first);
    }
    std::sort(sorted_unit_names.begin(), sorted_unit_names.end());

    if(config.to_json)
    {
        if(config.prettify)
        {
            print_json_pretty(root_nodes, sorted_unit_names);
        }
        else
        {
            print_json(root_nodes, sorted_unit_names);
        }
    }
    else
    {
        if(config.prettify)
        {
            print_pretty(root_nodes, sorted_unit_names);
        }
        else
        {
            print_simple(root_nodes, sorted_unit_names);
        }
    }

    return EXIT_CODE_OK;
}
