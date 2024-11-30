#include <protocol/identifier.h>
#include <protocol/utils.h>

namespace ProtocolUtils {

    ReturnObject::ReturnObject(std::string return_value, char behavior, bool sendResponse) {
        this->return_value = return_value;
        this->behavior = behavior;
        this->bytes = return_value.size();
        this->sendResponse = sendResponse;
    }

    std::string constructProtocol(std::vector<std::string> args, ProtocolTypes::ResponseType rType) {

        if (rType == ProtocolTypes::ResponseType::ARRAY) return constructArray(args);
        if (rType == ProtocolTypes::ResponseType::SSTRING) return constructSimpleString(args);
        if (rType == ProtocolTypes::ResponseType::RBSTRING) return constructRestBulkString(args);
        if (rType == ProtocolTypes::ResponseType::BSTRING) return constructBulkString(args);

        PRINT_ERROR("ERROR CONSTRUCTING PROTOCOL - NO VALID PROTOCOL PASSED: " + std::to_string(ProtocolTypes::BSTRING));
        return "";

        // for (std::string arg : args) {
        //     if (asBulkString) {
        //         response += arg;
        //     } else {
        //         response += "$" + std::to_string(arg.size()) + "\r\n" + arg + "\r\n";
        //     }

        // }

        // if (asBulkString) response = "$" + std::to_string(response.size()) + "\r\n" + response + "\r\n";
        // return response;
    }

    std::string constructBulkString(std::vector<std::string> &args) {
        // $<length>\r\n<data>\r\n

        std::string response {};

        for (std::string arg : args) {
            response += arg + "\r";
        }

        return '$' + std::to_string(response.size() - 1) + "\r\n" + response + "\n";
    }


    std::string constructRestBulkString(std::vector<std::string> &args) {

        std::string response {};

        for (std::string arg : args) {
            response += "$" + std::to_string(arg.size()) + "\r\n" + arg + "\r\n";
        }

        return response;
    }

    std::string constructSimpleString(std::vector<std::string> &args) {

        std::string response = "+";

        for (unsigned short i {}; i < args.size() - 1; i++) {
            response += args[i] + " ";
        }

        response += args[args.size() - 1] + "\r\n";
        return response;
    }

    std::string constructArray(std::vector<std::string> &args) {

        std::string response = '*' + std::to_string(args.size()) + "\r\n";

        response += constructRestBulkString(args);

        return response;
    }
}