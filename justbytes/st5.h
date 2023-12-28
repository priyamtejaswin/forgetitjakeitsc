#ifndef ST5_H_
#define ST5_H_

#include "nmt_nfkc_rules.h"
#include <string>
#include <vector>
#include <map>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t char32;
typedef uint32_t uint32;
typedef uint64_t uint64;
static constexpr uint32 kUnicodeError = 0xFFFD;

namespace st5 {

// Basic Unicode character sequence.
using Chars = std::vector<char32>;

// String-to-string mapping.
using CharsMap = std::map<Chars, Chars>;

class Normalizer {
    public:
    Normalizer(std::string_view name);
    ~Normalizer();

    private:
    void GetPrecompiledCharsMap(
        std::string_view name,
        std::string *output
    );
    void DecodePrecompiledCharsMap(
        std::string_view blob, 
        std::string_view *trie_blob,
        std::string_view *normalized
    );
    // void DecompileCharsMap(std::string_vi, std::string_view blob, CharsMap *chars_map);
};

template <typename T>
inline bool DecodePOD(std::string_view str, T *result) {
    if (sizeof(*result) != str.size()) {
        return false;
    }
    memcpy(result, str.data(), sizeof(T));
    return true;
}

} // namespace st5

#endif // ST5_H