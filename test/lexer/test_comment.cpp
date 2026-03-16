#include <gtest/gtest.h>

#include <sstream>

#include "lexer.h"

TEST(LexerCommentTest, LexesCommentThenEof) {
    std::istringstream input("# just a comment");
    klds::lexer        lex(input);

    auto comment = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerCommentTest, CommentFollowedByNewlineProducesNewlineToken) {
    std::istringstream input(R"(# just a comment
)");
    klds::lexer        lex(input);

    auto comment = lex.get_token();
    auto nl      = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_FALSE(comment.m_val.has_value());
    EXPECT_EQ(nl.m_tok, klds::lexer::TOK_NL);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerCommentTest, CommentFollowedByCarriageReturnSkipsReturnAndContinues) {
    std::istringstream input(R"(# windows style comment)"
                             "\r"
                             R"(next)");
    klds::lexer        lex(input);

    auto comment = lex.get_token();
    auto ident   = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    const auto ident_value = ident.m_val
                                 ? std::get<std::string_view>(*ident.m_val)
                                 : std::string_view {};
    EXPECT_EQ(ident_value, "next");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerCommentTest, CommentFollowedByCrLfDoesNotProduceNewlineToken) {
    std::istringstream input(R"(# windows style comment)"
                             "\r\n"
                             R"(next)");
    klds::lexer        lex(input);

    auto comment = lex.get_token();
    auto ident   = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    const auto ident_value = ident.m_val
                                 ? std::get<std::string_view>(*ident.m_val)
                                 : std::string_view {};
    EXPECT_EQ(ident_value, "next");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerCommentTest, LeadingWhitespaceBeforeCommentIsIgnored) {
    std::istringstream input("   \t# padded comment");
    klds::lexer        lex(input);

    auto comment = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerCommentTest, IdentifierBeforeCommentIsLexedFirst) {
    std::istringstream input(R"(name# trailing comment
next)");
    klds::lexer        lex(input);

    auto ident   = lex.get_token();
    auto comment = lex.get_token();
    auto nl      = lex.get_token();
    auto next    = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(ident.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(ident.m_val.has_value());
    const auto ident_value = ident.m_val
                                 ? std::get<std::string_view>(*ident.m_val)
                                 : std::string_view {};
    EXPECT_EQ(ident_value, "name");
    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(nl.m_tok, klds::lexer::TOK_NL);
    EXPECT_EQ(next.m_tok, klds::lexer::TOK_IDENT);
    ASSERT_TRUE(next.m_val.has_value());
    const auto next_value = next.m_val ? std::get<std::string_view>(*next.m_val)
                                       : std::string_view {};
    EXPECT_EQ(next_value, "next");
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerCommentTest, NumberBeforeCommentIsLexedFirst) {
    std::istringstream input("12.5# trailing comment");
    klds::lexer        lex(input);

    auto number  = lex.get_token();
    auto comment = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(number.m_tok, klds::lexer::TOK_NUM);
    EXPECT_NEAR(number.get_double(), 12.5, 1e-9);
    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerCommentTest, ConsecutiveCommentsSeparatedByNewlinesAreAllLexed) {
    std::istringstream input(R"(# first
#second
)");
    klds::lexer        lex(input);

    auto first_comment  = lex.get_token();
    auto first_nl       = lex.get_token();
    auto second_comment = lex.get_token();
    auto second_nl      = lex.get_token();
    auto eof            = lex.get_token();

    EXPECT_EQ(first_comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(first_nl.m_tok, klds::lexer::TOK_NL);
    EXPECT_EQ(second_comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_EQ(second_nl.m_tok, klds::lexer::TOK_NL);
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}

TEST(LexerCommentTest, BareHashIsStillAComment) {
    std::istringstream input("#");
    klds::lexer        lex(input);

    auto comment = lex.get_token();
    auto eof     = lex.get_token();

    EXPECT_EQ(comment.m_tok, klds::lexer::TOK_COMMENT);
    EXPECT_FALSE(comment.m_val.has_value());
    EXPECT_EQ(eof.m_tok, klds::lexer::TOK_EOF);
}
