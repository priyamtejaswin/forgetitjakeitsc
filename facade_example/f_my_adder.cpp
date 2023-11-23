#include "my_adder.hpp"

extern "C" {
    #include "f_my_adder.hpp"
}

FMyAdder* createMyAdder(const int val) {
    return reinterpret_cast<FMyAdder*>(new MyAdder(val));
}

void destroyMyAdder(FMyAdder* fma) {
    delete reinterpret_cast<MyAdder*>(fma);
}

void do_add(FMyAdder *fma, const int val) {
    reinterpret_cast<MyAdder*>(fma)->add(val);
}