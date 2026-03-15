#include <gtest/gtest.h>

#include <sstream>
#include <string_view>

#include "lexer.h"

TEST(LexerUnknownTest, UnknownTokenDoesNotBlockNextToken) {
    std::istringstream input("@abc");
    klds::lexer        lex(input);

    auto unknown = lex.get_token();
    auto ident   = lex.get_token();

    EXPECT_EQ(unknown.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    if (!ident.m_val.has_value()) {
        GTEST_FAIL() << "identifier token should carry a value";
    }

    EXPECT_EQ(std::get<std::string_view>(*ident.m_val), "abc");
}
