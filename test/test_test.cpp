#include <gtest/gtest.h>

#include "lexer.h"

TEST(SmokeTest, TokensHaveExpectedValues) {
    EXPECT_NE(klds::lexer::TOK_EOF, 0);
    EXPECT_EQ(klds::lexer::TOK_UNK, 0);
}
