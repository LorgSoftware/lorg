#include <filesystem>
#include <fstream>
#include <iostream>

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

    lorg::ParserResult result = lorg::parse(content);
    if(result.has_error)
    {
        std::cerr << result.error_message << std::endl;
        exit(EXIT_CODE_ERROR_PARSE);
    }

    std::cout << "Successfully parsed the content." << std::endl;

    return EXIT_CODE_OK;
}
