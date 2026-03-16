#include "lexer.h"

#include <cassert>
#include <cctype>
#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <print>
#include <string>

#include "fwd.h"

namespace klds {
double lexer::tok_data::get_double() {
    if (m_tok == TOK_NUM) {
        if (m_val) {
            return std::get<double>(*m_val);
        }
    }
    // Not double
    return std::numeric_limits<double>::infinity();
}

std::string_view lexer::tok_data::get_string() {
    if (m_tok == TOK_IDENT) {
        if (m_val) {
            return std::get<std::string_view>(*m_val);
        }
    }
    // Not string
    return std::string_view {};
};

lexer::location& lexer::location::operator++() {
    ++column;
    return *this;
}

lexer::location lexer::location::operator++(int) {
    location old = *this;
    operator++();
    return old;
}

void lexer::location::new_line() {
    ++line;
    column = 0;
}

lexer::lexer(std::istream& contents) : m_contents(contents) {}

std::optional<lexer::tok_data> lexer::handle_keywords(std::string& str) {
    if (str == "def") {
        return tok_data {TOK_DEF, std::nullopt};
    }
    if (str == "extern") {
        return tok_data {TOK_EXTRN, std::nullopt};
    }
    return std::nullopt;
}

void lexer::consume() {
    m_last_char = m_contents.get();
    m_curr_loc++;
}

lexer::tok_data lexer::construct_token() {
    auto pos = m_tokens.size() - 1;
    if (m_token_map.contains(pos)) {
        return tok_data {m_tokens[pos], to_token_value(m_token_map[pos])};
    }
    return tok_data {m_tokens[pos], std::nullopt};
}

lexer::tok_data lexer::get_token() {
    while (std::isspace(m_last_char) && m_last_char != '\n') {
        consume();
    }

    if (m_last_char == '\n') {
        m_curr_loc.new_line();
        consume();
        m_tokens.push_back(TOK_NL);
        return construct_token();
    }

    std::string identifier;
    auto        is_alpha = [](int chr) {
        return std::isalpha(static_cast<unsigned char>(chr));
    };

    auto is_alnum = [](int chr) {
        return std::isalnum(static_cast<unsigned char>(chr));
    };

    // I hate this alpha/alnum naming so ill leave hints for this
    // isalpha is practically to check if char is from alphabet
    if (is_alpha(m_last_char)) {
        complete_identifier(identifier, is_alnum);

        if (auto val = handle_keywords(identifier)) {
            m_tokens.push_back(val->m_tok);
            return construct_token();
        }

        m_tokens.push_back(TOK_IDENT);
        m_token_map[m_tokens.size() - 1] = identifier;
        return construct_token();
    }

    if (std::isdigit(static_cast<unsigned char>(m_last_char)) ||
        m_last_char == '.') {
        uint64_t dot_count = 0;
        m_last_char == '.' ? dot_count++ : dot_count;
        auto is_digit_or_dot = [&dot_count](int chr) {
            // I guess now doubles will be split into two separate parts, for
            // exmaple, if we have 1.23.45 it will turn into 1.23 .45 for my
            // future AST. dont really think its the desired behavior, but dk
            // really how lexer will interact will all this stuff anyways
            // Oh just got some kind of idea. i think lexer on its own shouldnt
            // even decide whether what we see is a double or not, it should do
            // sema later on

            chr == '.' ? dot_count++ : dot_count;
            if (dot_count > 1)
                return false;
            return std::isdigit(static_cast<unsigned char>(chr)) || chr == '.';
        };

        complete_identifier(identifier, is_digit_or_dot);

        m_tokens.push_back(TOK_NUM);
        double value;
        try {
            value = std::stod(identifier);
        } catch (const std::invalid_argument& e) {
            goto err;
        }
        m_token_map[m_tokens.size() - 1] = value;
        return construct_token();
    }

    if (m_last_char == '#') {
        // While next char is anything but not new_line/end of file, ignore it.
        while (m_last_char != '\n' && m_last_char != '\r' &&
               m_last_char != EOF) {
            consume();
        }
        m_tokens.push_back(TOK_COMMENT);
        return construct_token();
    }

    if (m_last_char == EOF) {
        m_tokens.push_back(TOK_EOF);
        return construct_token();
    }
err:
    // If this is reached we are screwed
    std::print(
        std::cerr,
        "Unhandled token type was found. current charachter: {}, line: {}, column: {}\n",
        m_last_char, m_curr_loc.line, m_curr_loc.column);

    // token unc, ha!
    consume();
    m_tokens.push_back(TOK_UNK);
    return construct_token();
}
void lexer::print_tokens() {
    if (m_tokens.size()) {
        for (const auto& t : m_tokens) {
            std::print(std::cout, "Token: {}({})\n", magic_enum::enum_name(t),
                       static_cast<int>(t));
        }
    } else {
        std::cerr << "No tokens found.\n";
    }
}

#ifdef TEST
std::vector<lexer::token> lexer::get_tokens() {
    return m_tokens;
}

std::unordered_map<size_t, token_value<std::string>> lexer::get_token_values() {
    return m_token_map;
}
#endif // TEST
} // namespace klds
