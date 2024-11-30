#pragma once
#include <string>
#include <vector>

namespace ProtocolTypes {
    constexpr signed char ARRAY = '*';         // Arrays
    constexpr signed char BSTRING = '$';       // Bulk Strings
    constexpr signed char RBSTRING = 'R';      // Rest Bulk Strings
    constexpr signed char SSTRING = '+';       // Simple Strings
    constexpr signed char ERROR = '-';         // Errors
    constexpr signed char INTEGER = ':';       // Integers

    constexpr const char* PONG_R = "+PONG\r\n";
    constexpr const char* OK_R = "+OK\r\n";
    constexpr const char* NONE_R = "$-1\r\n";

    enum class ResponseType : unsigned short {
        ARRAY,
        BSTRING,
        RBSTRING,
        SSTRING,
        ERROR,
        INTEGER
    };
}

namespace ProtocolUtils {

    struct ReturnObject {
        std::string return_value;
        size_t bytes;
        int behavior;
        bool sendResponse;

        ReturnObject(std::string return_value, char behavior = 0, bool sendResponse = true);
    };

    std::string constructArray(std::vector<std::string> &args);
    std::string constructBulkString(std::vector<std::string> &args);
    std::string constructRestBulkString(std::vector<std::string> &args);
    std::string constructSimpleString(std::vector<std::string> &args);
    std::string constructProtocol(std::vector<std::string> args, ProtocolTypes::ResponseType rType);
}