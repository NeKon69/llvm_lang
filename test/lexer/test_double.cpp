
#include <gtest/gtest.h>

#include <cmath>
#include <sstream>
#include <stdexcept>

#include "lexer.h"

TEST(LexerDoubleTest, LexesIntegerAsNumber) {
    std::istringstream input("42");
    klds::lexer        lex(input);

    auto number = lex.get_token();
    auto eof    = lex.get_token();

    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_DOUBLE_EQ(number.get_double(), 42.0);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerDoubleTest, LexesFractionalNumber) {
    std::istringstream input("12.345");
    klds::lexer        lex(input);

    auto number = lex.get_token();
    auto eof    = lex.get_token();

    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_NEAR(number.get_double(), 12.345, 1e-9);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerDoubleTest, LeadingDotNumberIsAccepted) {
    std::istringstream input(".125");
    klds::lexer        lex(input);

    auto number = lex.get_token();
    auto eof    = lex.get_token();

    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_NEAR(number.get_double(), 0.125, 1e-9);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerDoubleTest, MalformedMultiDotNumberSplitsIntoTwoNumberTokens) {
    std::istringstream input("1.23.45");
    klds::lexer        lex(input);

    auto number  = lex.get_token();
    auto number2 = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_NEAR(number.get_double(), 1.23, 1e-9);
    EXPECT_NEAR(number2.get_double(), .45, 1e-9);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerDoubleTest, DotOnlyInputProducesUnknownToken) {
    std::istringstream input(".");
    klds::lexer        lex(input);

    auto unk = lex.get_token();

    EXPECT_EQ(unk.m_tok, klds::lexer::TOK_UNK);
}

TEST(LexerDoubleTest, NumberBeforeIdentifierKeepsBothTokens) {
    std::istringstream input("12abc");
    klds::lexer        lex(input);

    auto number = lex.get_token();
    auto ident  = lex.get_token();
    auto eof    = lex.get_token();

    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_DOUBLE_EQ(number.get_double(), 12.0);
    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    EXPECT_EQ(ident.get_string(), "abc");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerDoubleTest, NumberBeforeCommentIsLexedFirst) {
    std::istringstream input("12.5# trailing comment");
    klds::lexer        lex(input);

    auto number  = lex.get_token();
    auto comment = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_NEAR(number.get_double(), 12.5, 1e-9);
    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerDoubleTest, NumberThenNewlineContinuesLexing) {
    std::istringstream input(R"(7
8)");
    klds::lexer        lex(input);

    auto first_number  = lex.get_token();
    auto newline       = lex.get_token();
    auto second_number = lex.get_token();
    auto eof           = lex.get_token();

    EXPECT_EQ(first_number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(first_number.m_val.has_value());
    EXPECT_DOUBLE_EQ(first_number.get_double(), 7.0);
    EXPECT_EQ(newline.m_tok, klds::lexer::TOK_NL);
    EXPECT_EQ(second_number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(second_number.m_val.has_value());
    EXPECT_DOUBLE_EQ(second_number.get_double(), 8.0);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}
