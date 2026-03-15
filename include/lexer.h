#include <chrono>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "fwd.h"

namespace klds {
class lexer {
public:
    using tok_size = int8_t;

    enum token : tok_size {
        // supposed to be at the end of every file
        TOK_EOF = -1,
        // `def` keyword
        TOK_DEF = -2,
        // `extern` keyword
        TOK_EXTRN = -3,
        // identifier (name of function/variable)
        TOK_IDENT = -4,
        // double value token
        TOK_NUM = -5,
    };

    struct tok_data {
        token m_tok;

        // Optionally stores additional info about token (like value)
        std::optional<std::variant<double, std::string_view>> m_val;

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
    std::unordered_map<size_t, std::variant<double, std::string>> m_token_map;

    std::ifstream& m_contents;

    int      m_last_char = ' ';
    location m_curr_loc;

public:
    lexer(std::ifstream& contents);

    tok_data get_token();

private:
    void consume();

    std::optional<tok_data> handle_keywords(std::string& str);

    // a few days ago i was saying that making functions that accept functions
    // templated is an anti pattern, yet, here i am
    // Func should accept `int, Ts..., std::locale`
    template<typename Func, typename... Ts>
    void complete_identifier(std::string& identifier, Func&& cond,
                             Ts&&... args);
};

} // namespace klds
