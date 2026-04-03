#include "interpreter.hpp"

#include <iostream>
#include <exception>

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: interpreter filename\n";
        return 1;
    }

    try {
        std::string source = loadFile(argv[1]);
        runInterpreter(source);
        return 0;
    } catch (const std::exception& ex) {
        if (CurrentToken.line > 0) {
            std::cerr << "Runtime error on line " << CurrentToken.line << ": " << ex.what() << "\n";
        } else {
            std::cerr << "Runtime error: " << ex.what() << "\n";
        }
        return 1;
    }
}