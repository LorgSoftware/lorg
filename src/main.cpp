#include <iostream>
#include <filesystem>

constexpr int EXIT_CODE_OK = 0;
constexpr int EXIT_CODE_ERROR_ARGUMENTS = 1;

int main(int argc, char* argv[])
{
    // Check the argument count
    if(argc < 2)
    {
        std::cerr << "Need a file as an argument" << std::endl;
        exit(EXIT_CODE_ERROR_ARGUMENTS);
    }

    // Check if file exists
    auto filepath = argv[1];
    {
        std::filesystem::path f(filepath);
        if(!std::filesystem::exists(f))
        {
            std::cerr << "The file \"" << filepath << "\" does not exist." << std::endl;
            exit(EXIT_CODE_ERROR_ARGUMENTS);
        }
    }

    return EXIT_CODE_OK;
}
