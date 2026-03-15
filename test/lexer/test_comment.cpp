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
