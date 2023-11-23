#include "my_adder.hpp"
#include <iostream>
#include "string"

int main(int argc, char **argv) {
    auto a = new MyAdder(5);
    if (argc != 2) throw MyAdderException("You must provide exactly 1 value!");
    else {
        try {
            int x = std::stoi(argv[1]);
            a->add(x);
        } catch (MyAdderException error) {
            std::cout << "EXCEPTION" << std::endl;
            std::cout << error.error_message.c_str() << std::endl;
        }
    }
    return 0;
}