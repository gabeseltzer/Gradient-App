#pragma once

#include "command.h"

class GCodeParser {
    public:
    GCodeParser(std::iostream& inputGCode_) : inputGCode{inputGCode_} {};

    Command getNextCommand();

    private:
    std::iostream& inputGCode;
};