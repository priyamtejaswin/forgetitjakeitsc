#include "st5.h"
#include "darts.h"
#include <string>
#include <iostream>
#include <array>

namespace st5 {

Normalizer::Normalizer(std::string_view name) {
    // Load pre-compiled rules.
    std::string precompiled_chars_map;
    GetPrecompiledCharsMap(name, &precompiled_chars_map);

    // Decompile to chars-map.
    CharsMap cmap;
    DecompileCharsMap(precompiled_chars_map, &cmap);
    std::cout << "Inside constructor. cmap size " << cmap.size() << std::endl;
    chars_map = std::move(cmap);    
}

Normalizer::~Normalizer() {}

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

    blob.remove_prefix(sizeof(trie_blob_size));

    *trie_blob = std::string_view(blob.data(), trie_blob_size);

    blob.remove_prefix(trie_blob_size);
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

} // namespace st5