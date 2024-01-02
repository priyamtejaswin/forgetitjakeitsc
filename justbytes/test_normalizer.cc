#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "st5.h"

// Space symbol
#define WS "\xe2\x96\x81"

// Replacement char
#define RC "\xEF\xBF\xBD"

TEST_CASE("Normalize") {
    st5::Normalizer normalizer("nmt_nfkc");

    // Empty strings.
    CHECK_EQ("", normalizer.Normalize(""));
    CHECK_EQ("", normalizer.Normalize("      "));
    CHECK_EQ("", normalizer.Normalize("　"));
}