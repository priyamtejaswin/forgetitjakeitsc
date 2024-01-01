#include "st5.h"
#include <string>
#include <iostream>
#include <array>

namespace st5 {

Normalizer::Normalizer(std::string_view name) : name_(name) {
    Init();
    std::cout << "outside init " << trie_ << std::endl;
}

void Normalizer::Init() {
    spec.name = name_;
    GetPrecompiledCharsMap(spec.name, &spec.precompiled_charsmap);
    std::string_view index = spec.precompiled_charsmap;

    std::string_view trie_blob, normalized;
    DecodePrecompiledCharsMap(index, &trie_blob, &normalized);

    // Reads the body of double array.
    trie_ = std::unique_ptr<Darts::DoubleArray>(new Darts::DoubleArray());

    // The second arg of set_array is not the size of blob,
    // but the number of double array units.
    trie_->set_array(const_cast<char *>(trie_blob.data()),
                     trie_blob.size() / trie_->unit_size());

    std::cout << trie_->size() << std::endl;
    
    normalized_ = normalized.data();

    std::cout << "inside init " << trie_ << std::endl;
}

Normalizer::~Normalizer() {}

std::string Normalizer::Normalize(std::string_view input) const {
    // Public!
    std::vector<size_t> norm_to_orig;
    std::string normalized;
    Normalize(input, &normalized, &norm_to_orig);
    return normalized;
}

void Normalizer::Normalize(
    std::string_view input,
    std::string *normalized,
    std::vector<size_t> *norm_to_orig
    ) const {
    // Private!
    norm_to_orig->clear();
    normalized->clear();

    if (input.empty()) {return;}

    int consumed = 0;

    // Ignores heading space by default.
    if (spec.remove_extra_whitespaces) {
        while (!input.empty()) {
            const auto p = NormalizePrefix(input);
            if (p.first != " ") {
                break;
            }
            input.remove_prefix(p.second);
            consumed += p.second;
        }
    }

    if (input.empty()) return;

    // Reserves the output buffer to avoid re-allocations.
    const size_t kReservedSize = input.size() * 3;
    normalized->reserve(kReservedSize);
    norm_to_orig->reserve(kReservedSize);

    // Replaces white space with U+2581 (LOWER ONE EIGHT BLOCK)
    // if escape_whitespaces() is set (default = true).
    const std::string_view kSpaceSymbol = "\xe2\x96\x81";

    // adds kSpaceSymbol to the current context.
    auto add_ws = [this, &consumed, &normalized, &norm_to_orig, &kSpaceSymbol]() {
        if (spec.escape_whitespaces) {
            normalized->append(kSpaceSymbol.data(), kSpaceSymbol.size());
            for (size_t n = 0; n < kSpaceSymbol.size(); ++n) {
                norm_to_orig->push_back(consumed);
            }
        } else {
            normalized->append(" ");
            norm_to_orig->push_back(consumed);
        }
    };

    // Adds a space symbol as a prefix (default is true)
    // With this prefix, "world" and "hello world" are converted into
    // "_world" and "_hello_world", which help the trainer to extract
    // "_world" as one symbol.
    if (!treat_whitespace_as_suffix_ && spec.add_dummy_prefix) add_ws();

    bool is_prev_space = spec.remove_extra_whitespaces;
    while (!input.empty()) {
        auto p = NormalizePrefix(input);
        // std::string_view _sp_temp = p.first;
        // const char *ptr_ = _sp_temp.data();
        std::string_view sp(p.first);

        // Removes heading spaces in sentence piece,
        // if the previous sentence piece ends with whitespace.
        // int size = _sp_temp.length();
        // while (is_prev_space && _sp_temp.front() == ' ') {
        //     ++ptr_;
        //     --size;
        // }
        while (is_prev_space && sp.front() == ' ' && sp.size() > 0) {
            sp.remove_prefix(1);
        }

        // std::string_view sp(ptr_, 1);
        std::cout << "sp:" << sp << std::endl;

        if (!sp.empty()) {
            const char *data = sp.data();
            for (size_t n = 0; n < sp.size(); ++n) {
                if (spec.escape_whitespaces && data[n] == ' ') {
                    // replace ' ' with kSpaceSymbol.
                    normalized->append(kSpaceSymbol.data(), kSpaceSymbol.size());
                    for (size_t m = 0; m < kSpaceSymbol.size(); ++m) {
                        norm_to_orig->push_back(consumed);
                    }
                } else {
                    *normalized += data[n];
                    norm_to_orig->push_back(consumed);
                }
            }
            // Checks whether the last character of sp is whitespace.
            is_prev_space = sp.back() == ' ';
        }

        consumed += p.second;
        input.remove_prefix(p.second);
        if (!spec.remove_extra_whitespaces) {
            is_prev_space = false;
        }
    }

    // Ignores tailing space.
    if (spec.remove_extra_whitespaces) {
        const std::string_view space = 
            spec.escape_whitespaces ? kSpaceSymbol : " ";
        while (string_util::EndsWith(*normalized, space)) {
            const int length = normalized->size() - space.size();
            // CHECK_GE_OR_RETURN(length, 0);
            if (length >= 0) return;
            consumed = (*norm_to_orig)[length];
            normalized->resize(length);
            norm_to_orig->resize(length);
        }
    }

    // Adds a space symbol as a suffix (default is false)
    if (treat_whitespace_as_suffix_ && spec.add_dummy_prefix) add_ws();

    norm_to_orig->push_back(consumed);

    // CHECK_EQ_OR_RETURN(norm_to_orig->size(), normalized->size() + 1);
    if (norm_to_orig->size() != normalized->size() + 1) return;

    // return util::OkStatus;
    return;  // But this has to be a valid status return!
}

std::pair<std::string_view, int> Normalizer::NormalizePrefix(
    std::string_view input
    ) const {
    std::pair<std::string_view, int> result;

    if (input.empty()) return result;

    if (matcher_ != nullptr) {
        bool found = false;
        const int mblen = matcher_->PrefixMatch(input, &found);
        if (found) return std::make_pair(input.substr(0, mblen), mblen);
    }

    size_t longest_length = 0;
    int longest_value = 0;

    if (trie_ != nullptr) {
        // Allocates trie_results in stack, which makes the encoding speed 36%
        // faster. (38k sentences/sec => 60k sentences/sec). Builder checks that the
        // result size never exceeds kMaxTrieResultsSize. This array consumes
        // 0.5kByte in stack, which is less than default stack frames (16kByte).
        Darts::DoubleArray::result_pair_type
            trie_results[Normalizer::kMaxTrieResultsSize];

        const size_t num_nodes = trie_->commonPrefixSearch(
            input.data(), trie_results, Normalizer::kMaxTrieResultsSize,
            input.size());

        // Finds the longest rule.
        for (size_t k = 0; k < num_nodes; ++k) {
            if (longest_length == 0 || trie_results[k].length > longest_length) {
                longest_length = trie_results[k].length;  // length of prefix
                longest_value = trie_results[k].value;    // pointer to |normalized_|.
            }
        }
    }

    if (longest_length == 0) {
        size_t length = 0;
        if (!string_util::IsValidDecodeUTF8(input, &length)) {
            // Found a malformed utf8.
            // The rune is set to be 0xFFFD (REPLACEMENT CHARACTER),
            // which is a valid Unicode of three bytes in utf8,
            // but here we only consume one byte.
            result.second = 1;
            static const char kReplacementChar[] = "\xEF\xBF\xBD";
            result.first = std::string_view(kReplacementChar);
        } else {
            result.second = length;
            result.first = std::string_view(input.data(), result.second);
        }
    } else {
        result.second = longest_length;
        // No need to pass the size of normalized sentence,
        // since |normalized| is delimitered by "\0".
        result.first = std::string_view(&normalized_[longest_value]);
    }

    return result;
}

void Normalizer::GetPrecompiledCharsMap(
    std::string_view name, 
    std::string *output
    ) {
    for (size_t i = 0; i < kNormalizationRules_size; ++i) {
        const auto *blob = &kNormalizationRules_blob[i];
        if (blob->name == name) {
            output->assign(blob->data, blob->size);
            std::cout << "Found " << name << std::endl;
        }
    }
}

void Normalizer::DecodePrecompiledCharsMap(
    std::string_view blob, 
    std::string_view *trie_blob,
    std::string_view *normalized
    ) {
    uint32 trie_blob_size = 0;

    if (blob.size() <= sizeof(trie_blob_size) ||
        !string_util::DecodePOD<uint32>(
            std::string_view(blob.data(), sizeof(trie_blob_size)),
            &trie_blob_size)) {
        std::cout << "ERROR! Blob for normalization rule is broken." << std::endl;
    }

    if (trie_blob_size >= blob.size()) {
        // return util::InternalError();
        std::cout << "ERROR! Trie data size exceeds the input blob size" << std::endl;
        return;
    }

    blob.remove_prefix(sizeof(trie_blob_size));
    std::cout << blob.size() << std::endl;

    *trie_blob = std::string_view(blob.data(), trie_blob_size);

    blob.remove_prefix(trie_blob_size);
    std::cout << blob.size() << std::endl;

    *normalized = std::string_view(blob.data(), blob.size());
}

void Normalizer::DecompileCharsMap(
    std::string_view blob, // View into the blob, not the blob itself!
    CharsMap *chars_map
    ) {
    chars_map->clear();
    
    std::string_view trie_blob, normalized;
    std::string buf;

    DecodePrecompiledCharsMap(blob, &trie_blob, &normalized);
    
    Darts::DoubleArray trie;
    trie.set_array(const_cast<char *>(trie_blob.data()),
                   trie_blob.size() / trie.unit_size());
    
    std::string key;
    std::function<void(size_t, size_t)> traverse;

    // Given a Trie node at `node_pos` and the key position at `key_position`,
    // Expands children nodes from `node_pos`.
    // When leaf nodes are found, stores them into `chars_map`.
    traverse = [&traverse, &key, &trie, &normalized, &chars_map](
                    size_t node_pos, size_t key_pos) -> void {
        for (int c = 0; c <= 255; ++c) {
            key.push_back(static_cast<char>(c));
            size_t copied_node_pos = node_pos;
            size_t copied_key_pos = key_pos;
            // Note: `copied_(node|key)_pos` are non-const references.
            // They store the new positions after node traversal.
            const Darts::DoubleArray::result_type result = trie.traverse(
                key.data(), copied_node_pos, copied_key_pos, key.size());
            if (result >= -1) {   // node exists.
                if (result >= 0) {  // has a value after transition.
                    const std::string_view value = normalized.data() + result;
                    Chars key_chars, value_chars;
                    for (const auto c : string_util::UTF8ToUnicodeText(key))
                        key_chars.push_back(c);
                    for (const auto c : string_util::UTF8ToUnicodeText(value))
                        value_chars.push_back(c);
                    (*chars_map)[key_chars] = value_chars;
                }
                // Recursively traverse.
                traverse(copied_node_pos, copied_key_pos);
            }
            key.pop_back();
        }
    };

    traverse(0, 0);
}

PrefixMatcher::PrefixMatcher(const std::set<std::string_view> &dic) {
    if (dic.empty()) return;
    std::vector<const char *> key;
    key.reserve(dic.size());
    for (const auto &it : dic) key.push_back(it.data());
    trie_ = std::unique_ptr<Darts::DoubleArray>(new Darts::DoubleArray());
    trie_->build(key.size(), const_cast<char **>(&key[0]), nullptr,
                            nullptr);
}

int PrefixMatcher::PrefixMatch(std::string_view w, bool *found) const {
    if (trie_ == nullptr) {
        if (found) *found = false;
        return std::min<int>(w.size(), string_util::OneCharLen(w.data()));
    }

    constexpr int kResultSize = 64;
    Darts::DoubleArray::result_pair_type trie_results[kResultSize];
    const int num_nodes =
        trie_->commonPrefixSearch(w.data(), trie_results, kResultSize, w.size());

    if (found) *found = (num_nodes > 0);
    if (num_nodes == 0) {
        return std::min<int>(w.size(), string_util::OneCharLen(w.data()));
    }

    int mblen = 0;
    for (int i = 0; i < num_nodes; ++i) {
        mblen = std::max<int>(trie_results[i].length, mblen);
    }

    return mblen;
}

std::string PrefixMatcher::GlobalReplace(std::string_view w,
                                         std::string_view out) const {
    std::string result;
    while (!w.empty()) {
        bool found = false;
        const int mblen = PrefixMatch(w, &found);
        if (found) {
        result.append(out.data(), out.size());
        } else {
        result.append(w.data(), mblen);
        }
        w.remove_prefix(mblen);
    }
    return result;
}

} // namespace st5