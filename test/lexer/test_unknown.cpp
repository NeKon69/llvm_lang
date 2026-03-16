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

TEST(LexerUnknownTest, UnknownTokenWithoutFollowerStillReachesEof) {
    std::istringstream input("@");
    klds::lexer        lex(input);

    auto unknown = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(unknown.m_tok, klds::lexer::TOK_UNK);
    EXPECT_FALSE(unknown.m_val.has_value());
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerUnknownTest, ConsecutiveUnknownCharactersProduceSeparateTokens) {
    std::istringstream input("@$%");
    klds::lexer        lex(input);

    auto first  = lex.get_token();
    auto second = lex.get_token();
    auto third  = lex.get_token();
    auto eof    = lex.get_token();

    EXPECT_EQ(first.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(second.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(third.m_tok, klds::lexer::TOK_UNK);
    EXPECT_FALSE(first.m_val.has_value());
    EXPECT_FALSE(second.m_val.has_value());
    EXPECT_FALSE(third.m_val.has_value());
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerUnknownTest, UnknownTokenDoesNotBlockNumberParsing) {
    std::istringstream input("@123.5");
    klds::lexer        lex(input);

    auto unknown = lex.get_token();
    auto number  = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(unknown.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    const auto number_value =
        number.m_val ? std::get<double>(*number.m_val) : 0.0;
    EXPECT_DOUBLE_EQ(number_value, 123.5);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerUnknownTest, UnknownTokenDoesNotBlockKeywordParsing) {
    std::istringstream input("@extern");
    klds::lexer        lex(input);

    auto unknown = lex.get_token();
    auto keyword = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(unknown.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(keyword.m_tok, klds::lexer::TOK_EXTRN);
    EXPECT_FALSE(keyword.m_val.has_value());
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerUnknownTest, UnknownTokenDoesNotBlockCommentParsing) {
    std::istringstream input("@# trailing comment");
    klds::lexer        lex(input);

    auto unknown = lex.get_token();
    auto comment = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(unknown.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_FALSE(comment.m_val.has_value());
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerUnknownTest, UnknownTokenBeforeNewlineStillProducesNewlineToken) {
    std::istringstream input(R"(@
)");
    klds::lexer        lex(input);

    auto unknown = lex.get_token();
    auto nl      = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(unknown.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(nl.m_tok, klds::lexer::TOK_NL);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerUnknownTest, UnknownTokenAfterWhitespaceIsStillReported) {
    std::istringstream input("   \t@");
    klds::lexer        lex(input);

    auto unknown = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(unknown.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerUnknownTest, UnknownTokenBeforeCommentStillLexesFollowingIdentifier) {
    std::istringstream input(R"(@# comment
next)");
    klds::lexer        lex(input);

    auto unknown = lex.get_token();
    auto comment = lex.get_token();
    auto nl      = lex.get_token();
    auto ident   = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(unknown.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    EXPECT_EQ(nl.m_tok, klds::lexer::TOK_NL);
    ASSERT_TRUE(ident.m_val.has_value());

    const auto ident_value = ident.m_val
                                 ? std::get<std::string_view>(*ident.m_val)
                                 : std::string_view {};
    EXPECT_EQ(ident_value, "next");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerUnknownTest, UnknownTokenCanAppearBetweenValidTokens) {
    std::istringstream input("abc@def");
    klds::lexer        lex(input);

    auto first   = lex.get_token();
    auto unknown = lex.get_token();
    auto second  = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(first.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(first.m_val.has_value());
    const auto first_value = first.m_val
                                 ? std::get<std::string_view>(*first.m_val)
                                 : std::string_view {};
    EXPECT_EQ(first_value, "abc");
    EXPECT_EQ(unknown.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(second.m_tok, klds::lexer::TOK_DEF);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}
