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

// Storing unicode text.
using UnicodeText = std::vector<char32>;

class Normalizer {
    public:
    Normalizer(std::string_view name);
    ~Normalizer();
    CharsMap chars_map;

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
    void DecompileCharsMap(
        std::string_view blob,
        CharsMap *chars_map
    );
};

namespace string_util { // Contains utility funtion definitions.

// Return (x & 0xC0) == 0x80;
// Since trail bytes are always in [0x80, 0xBF], we can optimize:
inline bool IsTrailByte(char x) { return static_cast<signed char>(x) < -0x40; }

inline bool IsValidCodepoint(char32 c) {
  return (static_cast<uint32>(c) < 0xD800) || (c >= 0xE000 && c <= 0x10FFFF);
}

// mblen sotres the number of bytes consumed after decoding.
inline char32 DecodeUTF8(const char *begin, const char *end, size_t *mblen) {
  const size_t len = end - begin;

  if (static_cast<unsigned char>(begin[0]) < 0x80) {
    *mblen = 1;
    return static_cast<unsigned char>(begin[0]);
  } else if (len >= 2 && (begin[0] & 0xE0) == 0xC0) {
    const char32 cp = (((begin[0] & 0x1F) << 6) | ((begin[1] & 0x3F)));
    if (IsTrailByte(begin[1]) && cp >= 0x0080 && IsValidCodepoint(cp)) {
      *mblen = 2;
      return cp;
    }
  } else if (len >= 3 && (begin[0] & 0xF0) == 0xE0) {
    const char32 cp = (((begin[0] & 0x0F) << 12) | ((begin[1] & 0x3F) << 6) |
                       ((begin[2] & 0x3F)));
    if (IsTrailByte(begin[1]) && IsTrailByte(begin[2]) && cp >= 0x0800 &&
        IsValidCodepoint(cp)) {
      *mblen = 3;
      return cp;
    }
  } else if (len >= 4 && (begin[0] & 0xf8) == 0xF0) {
    const char32 cp = (((begin[0] & 0x07) << 18) | ((begin[1] & 0x3F) << 12) |
                       ((begin[2] & 0x3F) << 6) | ((begin[3] & 0x3F)));
    if (IsTrailByte(begin[1]) && IsTrailByte(begin[2]) &&
        IsTrailByte(begin[3]) && cp >= 0x10000 && IsValidCodepoint(cp)) {
      *mblen = 4;
      return cp;
    }
  }

  // Invalid UTF-8.
  *mblen = 1;
  return kUnicodeError;
}

inline UnicodeText UTF8ToUnicodeText(std::string_view utf8) {
    UnicodeText uc;
    const char *begin = utf8.data();
    const char *end = utf8.data() + utf8.size();
    while (begin < end) {
        size_t mblen;
        const char32 c = DecodeUTF8(begin, end, &mblen);
        uc.push_back(c);
        begin += mblen;
    }
    return uc;
}

} // namespace string_util

} // namespace st5

#endif // ST5_H_