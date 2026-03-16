#include <cstdio>
#include <exception>
#include <iostream>
#include <sstream>
#include <string_view>

#include "lexer.h"

auto main() -> int {
    try {
        constexpr std::string_view input = R"(extern sin
def add
foo 12.345 bar # comment time
@ baz 7.5
)";

        std::istringstream source {std::string {input}};
        klds::lexer        lex(source);

        std::cout << "Input:\n" << input << "\n";

        while (lex.get_token().m_tok != klds::lexer::TOK_EOF) {
        }

        std::cout << "Lexer got:\n";
        lex.print_tokens();
        std::cout << '\n';

        return 0;
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "fatal: %s\n", ex.what());
    } catch (...) {
        std::fputs("fatal: unknown exception\n", stderr);
    }

    return 1;
}
