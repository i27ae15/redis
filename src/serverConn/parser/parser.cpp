#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctype.h>

#include <serverConn/parser/parser.h>

#include <protocol/utils.h>

#include <utils.h>


namespace RomulusParser {

    unsigned char getVarChars(unsigned short& index, const char* buffer) {

        std::string nVar {};

        while (true) {
            unsigned char byte = static_cast<unsigned char>(buffer[index]);
            if (!isdigit(byte)) break;
            nVar += byte;
            index++;
        }

        // The error might be here
        unsigned char rValue {};
        try {
            rValue = static_cast<unsigned char>(std::stoi(nVar));
        } catch (const std::invalid_argument) {
            PRINT_ERROR("STOI: " + nVar);
            throw;
        }

        return rValue;
    }

    ParseCommand parserArray(unsigned short& index, const char* buffer) {

        unsigned char nArgs = static_cast<unsigned char>(buffer[index] - '0');
        unsigned short size = index;

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

        size = index - size;
        index -= 2;

        // PRINT_WARNING("ARRAY VALUE(S) FOUND: " + found);
        return ParseCommand{found, size};
    }

    ParseCommand parserString(unsigned short& index, const char* buffer, size_t bufferSize) {

        std::string found {};
        unsigned short size {};
        unsigned char byte {};

        while (bufferSize > index) {
            size++;
            byte = static_cast<unsigned char>(buffer[index++]);
            if (byte == '\n' || byte == '\r') continue;
            if (byte == ProtocolTypes::ARRAY || byte == ProtocolTypes::BSTRING) break;
            found += byte;
        }
        index -= 2;

        // PRINT_WARNING("BULK STRING FOUND: " + found);
        return ParseCommand{found, size};
    }
}