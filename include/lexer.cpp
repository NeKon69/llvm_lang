#include "lexer.h"

#include <cassert>
#include <cctype>
#include <optional>
#include <string>

#include "fwd.h"
#include "llvm/

namespace klds {
double lexer::tok_data::get_double() {
    if (m_tok == TOK_NUM) {
        return std::get<double>(*m_val);
    }
    // Not double
    return std::numeric_limits<double>::infinity();
}

std::string_view lexer::tok_data::get_string() {
    if (m_tok == TOK_DEF) {
        return std::get<std::string_view>(*m_val);
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

lexer::lexer(std::ifstream& contents) : m_contents(contents) {}

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
    if (!m_contents.eof()) {
        m_last_char = m_contents.get();
        m_curr_loc++;
    } else {
        m_last_char = EOF;
    }
}

template<typename Func, typename... Ts>
void lexer::complete_identifier(std::string& identifier, Func&& cond,
                                Ts&&... args) {
    identifier += std::string(1, static_cast<char>(m_last_char));

    consume();
    while (std::forward<Func>(cond)(m_last_char, std::forward<Ts>(args)...,
                                    std::locale {})) {
        identifier += std::string(1, static_cast<char>(m_last_char));
        consume();
    }
}

lexer::tok_data lexer::get_token() {
    while (std::isspace(m_last_char)) {
        consume();
    }

    std::string identifier;
    // I hate this alpha/alnum naming so ill leave hints for this
    // isalpha is practically to check if char is from alphabet
    if (std::isalpha(m_last_char)) {
        complete_identifier(identifier, std::isalnum<int>);

        if (auto val = handle_keywords(identifier)) {
            return *val;
        }

        return tok_data {TOK_IDENT, std::string_view(identifier)};
    }

    if (std::isdigit(m_last_char) || m_last_char == '.') {
        identifier += std::string(1, static_cast<char>(m_last_char));

        auto is_digit_or_dot = [](int chr, const std::locale& l) {
            // FIXME: currently if you write 1.23.45 it will turn into 1.23,
            // gotta fix that later.
            return std::isdigit(chr, l) || chr == '.';
        };

        complete_identifier(identifier, is_digit_or_dot);

        return tok_data {TOK_NUM, std::stod(identifier)};
    }

    assert(
        !std::format(
             "Unhandled token type was found. current charachter: {}, line: {}, column: {}",
             m_last_char, m_curr_loc.line, m_curr_loc.column)
             .c_str());
    // Can't be reached
}

} // namespace klds
