#include <iostream>
#include "st5.h"

int main(int argc, char *argv[]) {
    st5::NormalizerSpec default_spec;
    st5::Normalizer normalizer(default_spec);
    std::string test_str = "   ①②③     ABC   ";
    std::cout << "Input:" << test_str << std::endl;

    auto nmd = normalizer.Normalize(test_str);
    std::cout << "Normalized:" << nmd << std::endl;
    return 0;
}