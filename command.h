#pragma once

#include <string>
#include <vector>

// TODO: is this a good name for the enum class???
enum class CommandKind {
    tok_comment,
    tok_blank_line,
    tok_G_Generic,

    tok_G0,
    tok_G1,
    tok_G92,

    tok_M_Generic,

    tok_T,
    tok_eof // End of file (eof)
};

struct Argument {
    char letter;
    double value;
};

struct Command {
    CommandKind kind;
    std::vector<Argument> arguments;
    std::string comment;
};