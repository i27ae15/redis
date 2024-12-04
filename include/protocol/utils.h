#pragma once
#include <string>
#include <vector>

namespace ProtocolTypes {
    constexpr unsigned char ARRAY = '*';         // Arrays
    constexpr unsigned char BSTRING = '$';       // Bulk Strings
    constexpr unsigned char RBSTRING = 'R';      // Rest Bulk Strings
    constexpr unsigned char SSTRING = '+';       // Simple Strings
    constexpr unsigned char ERROR = '-';         // Errors
    constexpr unsigned char INTEGER = ':';       // Integers

    constexpr const char* PONG_R = "+PONG\r\n";
    constexpr const char* OK_R = "+OK\r\n";
    constexpr const char* NONE_R = "$-1\r\n";
    constexpr const char* QUEUE_R = "+QUEUED\r\n";

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

    std::string constructError(const std::string msg);
    std::string constructInteger(const std::string integer);
    std::string constructArray(const std::vector<std::string> args);
    std::string constructBulkString(const std::vector<std::string> args);
    std::string constructRestBulkString(const std::vector<std::string> args);
    std::string constructSimpleString(const std::vector<std::string> args);
    std::string constructProtocol(const std::vector<std::string> args, ProtocolTypes::ResponseType rType);
}