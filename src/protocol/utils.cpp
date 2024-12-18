#include <protocol/identifier.h>
#include <protocol/utils.h>

namespace ProtocolUtils {

    ReturnObject::ReturnObject(std::string rValue, unsigned short behavior, bool sendResponse) {
        this->rValue = rValue;
        this->behavior = behavior;
        this->bytes = rValue.size();
        this->sendResponse = sendResponse;
    }

    std::string constructProtocol(
        const std::vector<std::string> args,
        ProtocolTypes::ResponseType rType
    ) {

        if (rType == ProtocolTypes::ResponseType::ARRAY) return constructArray(args);
        if (rType == ProtocolTypes::ResponseType::SSTRING) return constructSimpleString(args);
        if (rType == ProtocolTypes::ResponseType::RBSTRING) return constructRestBulkString(args);
        if (rType == ProtocolTypes::ResponseType::BSTRING) return constructBulkString(args);
        if (rType == ProtocolTypes::ResponseType::INTEGER) return constructInteger(args[0]);
        if (rType == ProtocolTypes::ResponseType::ERROR) return constructError(args[0]);

        PRINT_ERROR("ERROR CONSTRUCTING PROTOCOL - NO VALID PROTOCOL PASSED: " + std::to_string(ProtocolTypes::BSTRING));
        throw std::runtime_error("ILLO, PERO QUE PROTOCOL QUERES???? HIJO DE PUTA");

        return "";
    }

    std::string constructError(const std::string msg) {
        return "-ERR " + msg + "\r\n";
    }

    std::string constructInteger(const std::string integer) {
        return ":" + integer + "\r\n";
    }

    std::string constructBulkString(const std::vector<std::string> args) {

        std::string response {};

        for (std::string arg : args) {
            response += arg + "\r";
        }

        return '$' + std::to_string(response.size() - 1) + "\r\n" + response + "\n";
    }

    std::string constructRestBulkString(const std::vector<std::string> args) {

        std::string response {};

        for (std::string arg : args) {
            response += "$" + std::to_string(arg.size()) + "\r\n" + arg + "\r\n";
        }

        return response;
    }

    std::string constructSimpleString(const std::vector<std::string> args) {

        std::string response = "+";

        for (unsigned short i {}; i < args.size() - 1; i++) {
            response += args[i] + " ";
        }

        response += args[args.size() - 1] + "\r\n";
        return response;
    }

    std::string constructArray(const std::vector<std::string> args, bool checkFirstByte) {

        std::string response = '*' + std::to_string(args.size()) + "\r\n";

        for (std::string arg : args) {
            if (checkFirstByte && (arg[0] == ':' || arg[0] == '-' || arg[0] == '*')) {
                response += arg;
            } else {
                response += constructRestBulkString({arg});
            }
        }

        return response;
    }
}