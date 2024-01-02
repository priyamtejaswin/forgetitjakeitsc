#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "st5.h"

// Space symbol
#define WS "\xe2\x96\x81"

// Replacement char
#define RC "\xEF\xBF\xBD"

TEST_CASE("Normalize") {
    st5::NormalizerSpec default_spec;
    st5::Normalizer normalizer(default_spec);

    // Empty strings.
    CHECK_EQ("", normalizer.Normalize(""));
    CHECK_EQ("", normalizer.Normalize("      "));
    CHECK_EQ("", normalizer.Normalize("　"));

    // Sentence with heading/tailing/redundant spaces.
    CHECK_EQ(WS "ABC", normalizer.Normalize("ABC"));
    CHECK_EQ(WS "ABC", normalizer.Normalize(" ABC "));
    CHECK_EQ(WS "A" WS "B" WS "C", normalizer.Normalize(" A  B  C "));
    CHECK_EQ(WS "ABC", normalizer.Normalize("   ABC   "));
    CHECK_EQ(WS "ABC", normalizer.Normalize("   ＡＢＣ   "));
    CHECK_EQ(WS "ABC", normalizer.Normalize("　　ABC"));
    CHECK_EQ(WS "ABC", normalizer.Normalize("　　ABC　　"));

    // NFKC char to char normalization.
    CHECK_EQ(WS "123", normalizer.Normalize("①②③"));

    // NFKC char to multi-char normalization.
    CHECK_EQ(WS "株式会社", normalizer.Normalize("㍿"));

    // Half width katakana, character composition happens.
    CHECK_EQ(WS "グーグル", normalizer.Normalize(" ｸﾞｰｸﾞﾙ "));

    CHECK_EQ(WS "I" WS "saw" WS "a" WS "girl",
                normalizer.Normalize(" I  saw a　 　girl　　"));
}