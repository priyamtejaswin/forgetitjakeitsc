#include <iostream>
#include "st5.h"

int main(int argc, char *argv[]) {
    st5::Normalizer normalizer("nmt_nfkc");
    std::string test_str = "  HELLO  ";
    auto test_out = normalizer.Normalize(test_str);
    std::cout << test_str << std::endl;
    std::cout << test_out << std::endl;
    return 0;
}