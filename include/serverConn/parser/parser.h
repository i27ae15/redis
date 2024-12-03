#include <string>

namespace RomulusParser {

    struct ParseCommand {
        std::string command;
        unsigned short size;
    };

    unsigned char getVarChars(unsigned short& index, const char* buffer);
    ParseCommand parserArray(unsigned short& index, const char* buffer);
    ParseCommand parserString(unsigned short& index, const char* buffer, size_t bufferSize);
}