#pragma once 

#include <iostream>
#include <regex>


class Kernel {

private:

    std::iostream& inputGCode;
    std::iostream& outputGCode;

public:

    Kernel(std::iostream& inputGCode_, std::iostream& outputGCode_) : inputGCode{inputGCode_}, outputGCode{outputGCode_} {}

    void transformGCode(/*config*/);

    int getLayerCount();
};

size_t countLayers(GCodeParser& parser) {
    Command command = parser.getNextCommand();

    size_t numLayers = 0;
    while(command.kind != CommandKind::tok_eof) {
        if(command.kind == CommandKind::tok_comment) {

            auto result = command.comment.find_first_of("; layer");
            numLayers += (result != std::string::npos) ? 1 : 0;

        }
    }

    return numLayers;
}

class CommandTransform {
public:
    virtual std::vector<Command> processCommand(const Command& command) = 0;
};

class GCodeGradientTransform : public CommandTransform {
    public:
    std::vector<Command> processCommand(const Command& command);
};

class GCodeUntretractTransform : public CommandTransform {
    public:
    std::vector<Command> processCommand(const Command& command);
};

void transformCommands (GCodeParser& parser, , std::vector<CommandTransform*> transformPipeline) {

    while(/* TODO */) {
        Command command;

        for(auto transform : transformPipeline) {
            auto command = transform.processCommand(command);
        }

        // write to file
    }
}
