#include <string>

namespace RemusParser {
    unsigned char getVarChars(unsigned short& index, const char* buffer);
    std::string parserArray(unsigned short& index, const char* buffer);
    std::string parserString(unsigned short& index, const char* buffer, size_t bufferSize);
}