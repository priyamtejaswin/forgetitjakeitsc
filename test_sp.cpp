#include <sentencepiece_processor.h>
#include <iostream>

int main() {
    sentencepiece::SentencePieceProcessor processor;
    const auto status = processor.Load("/Users/priyamtejaswin/Stuff/forgetitjakeitsc/tokenizer.model");
    if (!status.ok()) {
        std::cerr << status.ToString() << std::endl;
        // error
        return 1;
    }
    return 0;
}

// g++ --std=c++17 -arch arm64 -I /usr/local/include -L /usr/local/lib/ test_sp.cpp -l sentencepiece -o test_sp