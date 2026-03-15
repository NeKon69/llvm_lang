#pragma once
#include <cstdint>

namespace klds {
// We use values for tokens from -1 to -32768, probably wont need much more of
// em. also everything that isnt a recognizable token is returned as its char
// value.
enum token : int16_t {
    TOK_EOF   = -1,
    TOK_DEF   = -2,
    TOK_EXTRN = -3,
    TOK_IDENT = -4,
    TOK_NUM   = -5,
};
} // namespace klds
