#ifndef ST5_H_
#define ST5_H_

#include "nmt_nfkc_rules.h"
#include "darts.h"
#include <string>
#include <vector>
#include <map>
#include <set>

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

struct NormalizerSpec {
    std::string name = "nmt_nfkc";
    mutable std::string precompiled_charsmap;
    bool add_dummy_prefix = true;
    bool remove_extra_whitespaces = true;
    bool escape_whitespaces = true;
};

class PrefixMatcher {
    public:
    // Initializes the PrefixMatcher with `dic`.
    explicit PrefixMatcher(const std::set<std::string_view> &dic);

    // Finds the longest string in dic, which is a prefix of `w`.
    // Returns the UTF8 byte length of matched string.
    // `found` is set if a prefix match exists.
    // If no entry is found, consumes one Unicode character.
    int PrefixMatch(std::string_view w, bool *found = nullptr) const;

    // Replaces entries in `w` with `out`.
    std::string GlobalReplace(std::string_view w, std::string_view out) const;

    private:
    std::unique_ptr<Darts::DoubleArray> trie_;
};

class Normalizer {
    public:
    Normalizer(const NormalizerSpec &spec);
    ~Normalizer();
    std::string Normalize(std::string_view) const;
    
    private:
    void Init();

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

    void Normalize(
        std::string_view input,
        std::string *normalized,
        std::vector<size_t> *norm_to_orig
    ) const;

    std::pair<std::string_view, int> NormalizePrefix(
        std::string_view
    ) const;

    const NormalizerSpec *spec_;

    const PrefixMatcher *matcher_ = nullptr;

    // Split hello world into "hello_" and "world_" instead of
    // "_hello" and "_world".
    const bool treat_whitespace_as_suffix_ = false;

    // Internal trie for efficient longest matching.
    std::unique_ptr<Darts::DoubleArray> trie_;

    // "\0" delimitered output string.
    // the value of |trie_| stores pointers to this string.
    const char *normalized_ = nullptr;

    static constexpr int kMaxTrieResultsSize = 32;
};

namespace string_util { // Contains utility funtion definitions.

template <typename T>
inline bool DecodePOD(std::string_view str, T *result) {
  if (sizeof(*result) != str.size()) {
    return false;
  }
  memcpy(result, str.data(), sizeof(T));
  return true;
}

inline bool EndsWith(std::string str, std::string_view expected) {
    if (expected.length() > str.length()) return false;
    else {
        return str.compare(str.length() - expected.length(), 
            expected.length(), expected) == 0;
    }
}

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

inline char32 DecodeUTF8(std::string_view input, size_t *mblen) {
  return DecodeUTF8(input.data(), input.data() + input.size(), mblen);
}

inline bool IsValidDecodeUTF8(std::string_view input, size_t *mblen) {
  const char32 c = DecodeUTF8(input, mblen);
  return c != kUnicodeError || *mblen == 3;
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

// Return length of a single UTF-8 source character
inline size_t OneCharLen(const char *src) {
  return "\1\1\1\1\1\1\1\1\1\1\1\1\2\2\3\4"[(*src & 0xFF) >> 4];
}

} // namespace string_util

} // namespace st5

#endif // ST5_H_