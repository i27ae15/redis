#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctype.h>

#include <utils.h>


namespace RomulusUtils {
    std::vector<std::string> splitString(
        const std::string& str,
        const std::string& delimiter
    ) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }
        tokens.push_back(str.substr(start));
        return tokens;
    }

    // A utility function to print raw bytes
    void printRawBytes(const char* buffer, size_t size) {
        PRINT_WARNING("Printing buffer");
        std::cout << "Raw Bytes: ";
        for (size_t i = 0; i < size; ++i) {
            // Print each byte in hex format
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (static_cast<unsigned char>(buffer[i])) << " ";
        }
        std::cout << std::dec << std::endl; // Switch back to decimal format
        PRINT_SUCCESS("buffer printed");
    }

    void printMixedBytes(const char* buffer, size_t size) {
        std::cout << "Mixed Bytes: ";
        for (size_t i = 0; i < size; ++i) {
            unsigned char byte = static_cast<unsigned char>(buffer[i]);
            if (std::isprint(byte)) {
                std::cout << byte; // Print printable characters as-is
            } else {
                std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
            }
        }
        std::cout << std::dec << std::endl; // Reset formatting
    }

    void displayRawBytesAsChars(const char* buffer, size_t size) {
        std::cout << "[your_program] Processed Bytes: ";
        for (size_t i = 0; i < size; ++i) {
            unsigned char byte = static_cast<unsigned char>(buffer[i]);
            if (std::isprint(byte)) {
                // Print printable characters directly
                std::cout << byte;
            } else {
                // Print non-printable characters as \xXX
                std::cout << "\\x"
                          << std::hex << std::setw(2) << std::setfill('0')
                          << static_cast<int>(byte);
            }
        }
        std::cout << std::dec << std::endl; // Reset stream to decimal format
    }

    bool canConvertToInt(const std::string& str) {
        std::istringstream iss(str);
        int num;
        // Try to read an integer and ensure no leftover characters
        return (iss >> num) && (iss.eof());
    }

}
