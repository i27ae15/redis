#include <string>

namespace RemusParser {
    constexpr char ARRAY = '*';
    constexpr char BSTRING = '+';

    unsigned char getVarChars(unsigned short& index, const char* buffer);
    std::string parserArray(unsigned short& index, const char* buffer);
    std::string parserString(unsigned short& index, const char* buffer, size_t bufferSize);
}