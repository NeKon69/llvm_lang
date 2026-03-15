#include <chrono>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "fwd.h"

namespace klds {
template<typename StringType = std::string_view>
using token_value = std::variant<double, StringType>;
class lexer {
public:
    using tok_size = int8_t;

    enum token : tok_size {
        TOK_UNK = 0,
        // supposed to be at the end of every file
        TOK_EOF = -1,
        // new line token
        TOK_NL = -2,
        // `def` keyword
        TOK_DEF = -3,
        // `extern` keyword
        TOK_EXTRN = -4,
        // identifier (name of function/variable)
        TOK_IDENT = -5,
        // double value token
        TOK_NUM = -6,
        // comment
        TOK_COMMENT = -7,
    };

    struct tok_data {
        token m_tok;

        // Optionally stores additional info about token (like value)
        std::optional<token_value<>> m_val;

        double           get_double();
        std::string_view get_string();
    };

    struct location {
        // Lol i found out there isnt a limit for max characters a column so 64
        // bits it is!
        uint64_t column = 0;
        uint64_t line   = 0;

        location& operator++();
        location  operator++(int);
        void      new_line();
    };

private:
    std::vector<token> m_tokens;
    // Stores token order -> value
    std::unordered_map<size_t, token_value<std::string>> m_token_map;

    std::istream& m_contents;

    int      m_last_char = ' ';
    location m_curr_loc;

public:
    lexer(std::istream& contents);

    tok_data get_token();
    void     print_tokens();

#ifdef TEST
    std::vector<token> get_tokens();

    std::unordered_map<size_t, token_value<std::string>> get_token_values();
#endif // TEST

private:
    void consume();

    std::optional<tok_data> handle_keywords(std::string& str);

    // a fe days ago i as saying that making functions that accept functions
    // templated is an anti pattern, yet, here i am
    // Func should accept `int`
    template<typename Func>
    void complete_identifier(std::string& identifier, Func&& cond) {
        identifier.push_back(static_cast<char>(m_last_char));

        consume();
        while (cond(m_last_char)) {
            identifier.push_back(static_cast<char>(m_last_char));
            consume();
        }
    }

    template<typename T>
    T get_curr_token() {
        get_token_val<T>(m_tokens.size());
    }

    template<typename T>
    T get_token_val(size_t idx) {
        return std::get<T>(m_token_map.at(idx));
    }

    tok_data construct_token();

    static token_value<> to_token_value(
        const decltype(m_token_map)::mapped_type& v) {
        return std::visit(
            [](const auto& x) -> token_value<> {
                return x;
            },
            v);
    }
};

} // namespace klds
