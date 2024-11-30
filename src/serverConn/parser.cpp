#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctype.h>

#include <serverConn/parser.h>

#include <utils.h>


namespace RemusParser {

    unsigned char getVarChars(unsigned short& index, const char* buffer) {

        std::string nVar {};

        while (true) {
            unsigned char byte = static_cast<unsigned char>(buffer[index]);
            if (!isdigit(byte)) break;
            nVar += byte;
            index++;
        }

        return static_cast<unsigned char>(std::stoi(nVar));
    }

    std::string parserArray(unsigned short& index, const char* buffer) {

        unsigned char nArgs = static_cast<unsigned char>(buffer[index] - '0');

        index += 4;
        std::string found {};

        while (nArgs > 0) {
            unsigned char nVarChars = getVarChars(index, buffer);
            index += 2;
            found += std::string(buffer + index, nVarChars);
            if (nArgs > 1) found += " ";
            index += nVarChars + 3;

            nArgs--;
        }

        index -= 2;

        PRINT_WARNING("ARRAY VALUE(S) FOUND: " + found);
        return found;
    }

    std::string parserString(unsigned short& index, const char* buffer, size_t bufferSize) {

        std::string found {};
        unsigned char byte {};

        while (bufferSize > index) {
            byte = static_cast<unsigned char>(buffer[index++]);
            if (byte == '\n' || byte == '\r') continue;
            if (byte == ARRAY || byte == BSTRING) break;
            found += byte;
        }
        index -= 2;

        PRINT_WARNING("BULK STRING FOUND: " + found);
        return found;
    }
}