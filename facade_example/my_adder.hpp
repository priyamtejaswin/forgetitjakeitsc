#ifndef FE_MYADDER
#define FE_MYADDER

#include <string>

class MyAdderException {
    public:

    std::string error_message;

    explicit MyAdderException(const std::string &message): error_message(message) {};
};

class MyAdder {
    public:
    int m_value;
    int m_original;
    explicit MyAdder(const int val);
    ~MyAdder();

    void add(const int val);
    void reset();
};

#endif