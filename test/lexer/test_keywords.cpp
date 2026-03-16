
#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>

#include "lexer.h"

namespace {

struct keyword_case {
    const char*        text;
    klds::lexer::token token;
};

class LexerKeywordTest : public ::testing::TestWithParam<keyword_case> {};

const auto kKeywordCases = std::array {
    keyword_case {"def", klds::lexer::TOK_DEF},
    keyword_case {"extern", klds::lexer::TOK_EXTRN},
};

std::string keyword_name(const ::testing::TestParamInfo<keyword_case>& info) {
    return info.param.text;
}

} // namespace

TEST_P(LexerKeywordTest, KeywordBeforeFractionalNumberProducesBothTokens) {
    const auto&        keyword = GetParam();
    std::istringstream input(std::string(keyword.text) + " 12.345");
    klds::lexer        lex(input);

    auto keyword_tok = lex.get_token();
    auto number      = lex.get_token();

    EXPECT_EQ(keyword_tok.m_tok, keyword.token);
    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_NEAR(number.get_double(), 12.345, 1e-9);
}

TEST_P(LexerKeywordTest, KeywordStopsBeforeIdentifierSuffix) {
    const auto&        keyword = GetParam();
    std::istringstream input(std::string(keyword.text) + "42");
    klds::lexer        lex(input);

    auto ident = lex.get_token();
    auto eof   = lex.get_token();

    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    EXPECT_EQ(ident.get_string(), std::string(keyword.text) + "42");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST_P(LexerKeywordTest, KeywordThenIntegerEndsAtEof) {
    const auto&        keyword = GetParam();
    std::istringstream input(std::string(keyword.text) + " 42");
    klds::lexer        lex(input);

    auto keyword_tok = lex.get_token();
    auto number      = lex.get_token();
    auto eof         = lex.get_token();

    EXPECT_EQ(keyword_tok.m_tok, keyword.token);
    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_DOUBLE_EQ(number.get_double(), 42.0);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST_P(LexerKeywordTest, KeywordThenLeadingDotNumberIsAccepted) {
    const auto&        keyword = GetParam();
    std::istringstream input(std::string(keyword.text) + " .125");
    klds::lexer        lex(input);

    auto keyword_tok = lex.get_token();
    auto number      = lex.get_token();
    auto eof         = lex.get_token();

    EXPECT_EQ(keyword_tok.m_tok, keyword.token);
    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_NEAR(number.get_double(), 0.125, 1e-9);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST_P(LexerKeywordTest, KeywordThenNumberBeforeIdentifierKeepsBothTokens) {
    const auto&        keyword = GetParam();
    std::istringstream input(std::string(keyword.text) + " 12abc");
    klds::lexer        lex(input);

    auto keyword_tok = lex.get_token();
    auto number      = lex.get_token();
    auto ident       = lex.get_token();
    auto eof         = lex.get_token();

    EXPECT_EQ(keyword_tok.m_tok, keyword.token);
    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_DOUBLE_EQ(number.get_double(), 12.0);
    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    EXPECT_EQ(ident.get_string(), "abc");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST_P(LexerKeywordTest,
       KeywordThenMalformedMultiDotNumberSplitsIntoTwoNumberTokens) {
    const auto&        keyword = GetParam();
    std::istringstream input(std::string(keyword.text) + " 1.23.45");
    klds::lexer        lex(input);

    auto keyword_tok = lex.get_token();
    auto number      = lex.get_token();
    auto num2        = lex.get_token();
    auto eof         = lex.get_token();

    EXPECT_EQ(keyword_tok.m_tok, keyword.token);
    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_NEAR(number.get_double(), 1.23, 1e-9);
    EXPECT_NEAR(num2.get_double(), .45, 1e-9);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST_P(LexerKeywordTest, KeywordThenDotOnlyInputProducesUnknownToken) {
    const auto&        keyword = GetParam();
    std::istringstream input(std::string(keyword.text) + " .");
    klds::lexer        lex(input);

    auto keyword_tok = lex.get_token();
    auto unk         = lex.get_token();

    EXPECT_EQ(keyword_tok.m_tok, keyword.token);
    EXPECT_EQ(unk.m_tok, klds::lexer::TOK_UNK);
}

TEST_P(LexerKeywordTest,
       KeywordThenNumberBeforeCommentKeepsCommentPathReachable) {
    const auto&        keyword = GetParam();
    std::istringstream input(std::string(keyword.text) +
                             " 12.5# trailing comment");
    klds::lexer        lex(input);

    auto keyword_tok = lex.get_token();
    auto number      = lex.get_token();
    auto comment     = lex.get_token();
    auto eof         = lex.get_token();

    EXPECT_EQ(keyword_tok.m_tok, keyword.token);
    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    ASSERT_TRUE(number.m_val.has_value());
    EXPECT_NEAR(number.get_double(), 12.5, 1e-9);
    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST_P(LexerKeywordTest, KeywordThenNumberThenNewlineContinuesLexing) {
    const auto& keyword = GetParam();
    std::istringstream input(std::string(keyword.text) + R"( 7
)" + keyword.text + " 8");
    klds::lexer lex(input);

    auto first_keyword  = lex.get_token();
    auto first_number   = lex.get_token();
    auto newline        = lex.get_token();
    auto second_keyword = lex.get_token();
    auto second_number  = lex.get_token();
    auto eof            = lex.get_token();

    EXPECT_EQ(first_keyword.m_tok, keyword.token);
    EXPECT_EQ(first_number.m_tok, klds::lexer::TOK_NUM);
    EXPECT_DOUBLE_EQ(first_number.get_double(), 7.0);
    EXPECT_EQ(newline.m_tok, klds::lexer::TOK_NL);
    EXPECT_EQ(second_keyword.m_tok, keyword.token);
    EXPECT_EQ(second_number.m_tok, klds::lexer::TOK_NUM);
    EXPECT_DOUBLE_EQ(second_number.get_double(), 8.0);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

INSTANTIATE_TEST_SUITE_P(AllKeywords, LexerKeywordTest,
                         ::testing::ValuesIn(kKeywordCases), keyword_name);
