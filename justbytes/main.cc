#include <sentencepiece_processor.h>
#include <iostream>

int main(int argc, char* argv[]) {
    sentencepiece::SentencePieceProcessor processor;
    const auto status = processor.Load("./spm_char.model");
    if (!status.ok()) {
        // error
        std::cerr << status.ToString() << std::endl;
    }
}