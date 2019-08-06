#include "parser.h"

#include <string>

Command GCodeParser::getNextCommand() {

    std::string currLine;
    if(!inputGCode.eof()) {
        std::getline(inputGCode, currLine);
        // TODO: add regex to split the line into command components
        https://www.quora.com/How-do-I-split-a-string-by-space-into-an-array-in-c++

        // build command
        Command command;
        return command;
    }
    else {

        Command eofCommand;
        eofCommand.kind = CommandKind::tok_eof;
        return eofCommand;
    }
}
