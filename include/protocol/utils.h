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
    constexpr const char* EMPTY_ARRAY = "*0\r\n";

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

    /**
     * @struct ReturnObject
     * @brief Represents a structured response object with metadata for protocol utilities.
    */
    struct ReturnObject {
        std::string rValue; ///< The return value as a string.
        size_t bytes; ///< The size of the return value in bytes. calculated as rValue.size().
        unsigned short behavior; ///< A behavior code associated with the return object.
        bool sendResponse; ///< Indicates whether a response should be sent.

        /**
         * @brief Constructs a ReturnObject with specified values.
         *
         * @param rValue The return value as a string.
         * @param behavior An optional behavior code (default: 0).
         * @param sendResponse An optional boolean indicating whether to send a response (default: true).
        */
        ReturnObject(std::string rValue, unsigned short behavior = 0, bool sendResponse = true);
    };

    /**
     * @struct CommandObj
     * @brief Represents a command object containing details about a client command.
    */
    struct CommandObj {
        std::vector<std::string> splittedBuffer; ///< A vector of strings representing the parsed command arguments.
        unsigned short cSize; ///< The size of the command (e.g., the number of arguments).
        unsigned short clientFD; ///< The file descriptor of the client that sent the command.
    };

    /**
     * @brief Constructs an error message protocol.
     *
     * @param msg The error message.
     * @return A protocol string representing the error.
    */
    std::string constructError(const std::string msg);

    /**
     * @brief Constructs an integer message protocol.
     *
     * @param integer The integer value as a string.
     * @return A protocol string representing the integer.
    */
    std::string constructInteger(const std::string integer);

    /**
     * @brief Constructs a rest bulk string message protocol.
     *
     * @param args A vector of strings representing the arguments.
     * @return A protocol string representing the rest bulk string.
    */
    std::string constructBulkString(const std::vector<std::string> args);

    /**
     * @brief Constructs a bulk string message protocol.
     *
     * @param args A vector of strings representing the arguments.
     * @return A protocol string representing the bulk string.
    */
    std::string constructSimpleString(const std::vector<std::string> args);

    /**
     * @brief Constructs a rest bulk string message protocol.
     *
     * @param args A vector of strings representing the arguments.
     * @return A protocol string representing the rest bulk string.
    */
    std::string constructRestBulkString(const std::vector<std::string> args);

    /**
     * @brief Constructs an array message protocol.
     *
     * @param args A vector of strings representing the arguments.
     * @param checkFirstByte A boolean to check the first byte of each argument for special characters.
     * @return A protocol string representing the array.
    */
    std::string constructArray(const std::vector<std::string> args, bool checkFirstByte = false);

    /**
     * @brief Constructs a protocol message based on the specified response type and arguments.
     *
     * @param args A vector of strings representing the arguments.
     * @param rType The response type (e.g., ARRAY, SIMPLE STRING, BULK STRING, etc.). See ProtocolTypes::ResposneType
     * @return A protocol message string.
     * @exception std::runtime_error if not a proper rType if passed
    */
    std::string constructProtocol(const std::vector<std::string> args, ProtocolTypes::ResponseType rType);


}