#include <gtest/gtest.h>

#include <cmath>
#include <sstream>

#include "lexer.h"

TEST(LexerExternTest, LexesExternAndNumber) {
    std::istringstream input("extern 12.345");
    klds::lexer        lex(input);

    auto keyword = lex.get_token();
    auto number  = lex.get_token();

    EXPECT_EQ(keyword.m_tok, klds::lexer::TOK_EXTRN);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    EXPECT_NEAR(number.get_double(), 12.345, 1e-9);
}
