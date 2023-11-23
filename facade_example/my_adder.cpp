#include "my_adder.h"
#include <iostream>

MyAdder::MyAdder(const int val) {
    m_value = val;
    m_original = val;
    std::cout << __func__ << " : Creating MyAdder[" << this << "] " << m_value << std::endl;
}

MyAdder::~MyAdder() {
    std::cout << __func__ << " : Destroying MyAdder[" << this << "] " << m_value << std::endl;
}

void MyAdder::add(const int val) {
    if (val % 2 == 1) throw MyAdderException("Odd number provided!");
    else {
        m_value += val;
        std::cout << __func__ << " Updated MyAdder[" << this << "] " << m_value << std::endl;
    }
}

void MyAdder::reset() {
    m_value = m_original;
    std::cout << __func__ << " Resetting to original." << std::endl;
}