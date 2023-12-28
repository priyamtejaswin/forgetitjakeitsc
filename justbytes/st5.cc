#include "st5.h"
#include "nmt_nfkc_rules.h"
#include <string>
#include <iostream>
#include <array>

namespace st5 {

Normalizer::Normalizer(std::string_view name) {
    // Load pre-compiled rules.
    for (size_t i = 0; i < kNormalizationRules_size; ++i) {
        const auto *blob = &kNormalizationRules_blob[i];
        if (blob->name == name) {
            rules->assign(blob->data, blob->size);
        }
    }
    // TODO(priyam) handle unknown rule name.

    // Decompile string to CharsMap.
}

Normalizer::~Normalizer() {}

void Normalizer::DecodePrecompiledCharsMap(
    std::string_view blob, 
    std::string_view *trie_blob,
    std::string_view *normalized
    ) {
    uint32 trie_blob_size = 0;

    blob.remove_prefix(sizeof(trie_blob_size));

    *trie_blob = std::string_view(blob.data(), trie_blob_size);

    blob.remove_prefix(trie_blob_size);
    *normalized = std::string_view(blob.data(), blob.size());
}

void Normalizer::DecompileCharsMap(
    std::string_view blob, 
    CharsMap *chars_map
    ) {
    chars_map->clear();
    std::string_view trie_blob, normalized;
    DecodePrecompiledCharsMap(*rules, &trie_blob, &normalized);
}

} // namespace st5