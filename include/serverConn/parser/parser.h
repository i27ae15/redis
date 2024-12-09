#include <string>

namespace RomulusParser {

    struct ParseCommand {
        std::string command;
        unsigned short size;

        bool isEmpty();
    };

    unsigned char getVarChars(unsigned short& index, const char* buffer);
    ParseCommand parserArray(unsigned short& index, const char* buffer);
    ParseCommand parserString(unsigned short& index, const char* buffer, size_t bufferSize);
    ParseCommand parserBString(unsigned short& index, const char* buffer, size_t bufferSize);
}