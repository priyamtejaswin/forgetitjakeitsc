#include <iostream>
#include "st5.h"

int main(int argc, char *argv[]) {
    st5::Normalizer normalizer("nmt_nfkc");
    std::cout << "CharsMap size " << normalizer.chars_map.size() << std::endl;
    return 0;
}