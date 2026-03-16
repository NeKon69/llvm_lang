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

TEST(LexerIdentifierTest, LexesAlphanumericIdentifierPayload) {
    std::istringstream input("alpha123beta456");
    klds::lexer        lex(input);

    auto ident = lex.get_token();
    auto eof   = lex.get_token();

    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    const auto ident_value = ident.m_val
                                 ? std::get<std::string_view>(*ident.m_val)
                                 : std::string_view {};
    EXPECT_EQ(ident_value, "alpha123beta456");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerIdentifierTest, LeadingWhitespaceBeforeIdentifierIsIgnored) {
    std::istringstream input("   \t   spaced");
    klds::lexer        lex(input);

    auto ident = lex.get_token();
    auto eof   = lex.get_token();

    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    const auto ident_value = ident.m_val
                                 ? std::get<std::string_view>(*ident.m_val)
                                 : std::string_view {};
    EXPECT_EQ(ident_value, "spaced");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerIdentifierTest, SingleLetterIdentifierIsLexed) {
    std::istringstream input("x");
    klds::lexer        lex(input);

    auto ident = lex.get_token();
    auto eof   = lex.get_token();

    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    EXPECT_EQ(ident.get_string(), "x");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerIdentifierTest, IdentifierStopsBeforeUnknownToken) {
    std::istringstream input("name@");
    klds::lexer        lex(input);

    auto ident   = lex.get_token();
    auto unknown = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    const auto ident_value = ident.m_val
                                 ? std::get<std::string_view>(*ident.m_val)
                                 : std::string_view {};
    EXPECT_EQ(ident_value, "name");
    EXPECT_EQ(unknown.m_tok, klds::lexer::TOK_UNK);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerIdentifierTest, IdentifierStopsBeforeCommentAndCommentIsLexedNext) {
    std::istringstream input("name# trailing comment");
    klds::lexer        lex(input);

    auto ident   = lex.get_token();
    auto comment = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    const auto ident_value = ident.m_val
                                 ? std::get<std::string_view>(*ident.m_val)
                                 : std::string_view {};
    EXPECT_EQ(ident_value, "name");
    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerIdentifierTest, IdentifierFollowedByNewlineProducesNewlineToken) {
    std::istringstream input(R"(name
next)");
    klds::lexer        lex(input);

    auto first = lex.get_token();
    auto nl    = lex.get_token();
    auto next  = lex.get_token();
    auto eof   = lex.get_token();

    EXPECT_EQ(first.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(first.m_val.has_value());
    const auto first_value = first.m_val
                                 ? std::get<std::string_view>(*first.m_val)
                                 : std::string_view {};
    EXPECT_EQ(first_value, "name");
    EXPECT_EQ(nl.m_tok, klds::lexer::TOK_NL);
    EXPECT_EQ(next.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(next.m_val.has_value());
    const auto next_value = next.m_val ? std::get<std::string_view>(*next.m_val)
                                       : std::string_view {};
    EXPECT_EQ(next_value, "next");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerIdentifierTest, DefKeywordUsesKeywordPathInsteadOfIdentifierPayload) {
    std::istringstream input("def");
    klds::lexer        lex(input);

    auto keyword = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(keyword.m_tok, klds::lexer::TOK_DEF);
    EXPECT_FALSE(keyword.m_val.has_value());
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerIdentifierTest,
     ExternKeywordUsesKeywordPathInsteadOfIdentifierPayload) {
    std::istringstream input("extern");
    klds::lexer        lex(input);

    auto keyword = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(keyword.m_tok, klds::lexer::TOK_EXTRN);
    EXPECT_FALSE(keyword.m_val.has_value());
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerIdentifierTest, KeywordPrefixesThatContinueWithAlnumStayIdentifiers) {
    std::istringstream input("definitely extern1");
    klds::lexer        lex(input);

    auto first  = lex.get_token();
    auto second = lex.get_token();
    auto eof    = lex.get_token();

    EXPECT_EQ(first.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(first.m_val.has_value());
    const auto first_value = first.m_val
                                 ? std::get<std::string_view>(*first.m_val)
                                 : std::string_view {};
    EXPECT_EQ(first_value, "definitely");
    EXPECT_EQ(second.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(second.m_val.has_value());
    const auto second_value = second.m_val
                                  ? std::get<std::string_view>(*second.m_val)
                                  : std::string_view {};
    EXPECT_EQ(second_value, "extern1");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerIdentifierTest, DigitsAfterIdentifierRemainPartOfSameToken) {
    std::istringstream input("abc123 7");
    klds::lexer        lex(input);

    auto ident  = lex.get_token();
    auto number = lex.get_token();
    auto eof    = lex.get_token();

    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    const auto ident_value = ident.m_val
                                 ? std::get<std::string_view>(*ident.m_val)
                                 : std::string_view {};
    EXPECT_EQ(ident_value, "abc123");
    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    EXPECT_NEAR(number.get_double(), 7.0, 1e-9);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}
