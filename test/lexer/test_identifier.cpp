#include <gtest/gtest.h>

#include <sstream>
#include <string_view>

#include "lexer.h"

TEST(LexerIdentifierTest, LexesIdentifierPayload) {
    std::istringstream input("hello");
    klds::lexer        lex(input);

    auto ident = lex.get_token();

    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    if (!ident.m_val.has_value()) {
        GTEST_FAIL() << "identifier token should carry a value";
    }

    EXPECT_EQ(std::get<std::string_view>(*ident.m_val), "hello");
}
