#include <stdio.h>
#include "f_my_adder.hpp"

int main() {
    FMyAdder* fma = createMyAdder(0);
    do_add(fma, 4);
    destroyMyAdder(fma);
    return 0;
}