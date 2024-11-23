#include <protocol/identifier.h>
#include <protocol/utils.h>

namespace ProtocolUtils {

    ReturnObject::ReturnObject(std::string return_value, int behavior) {
        this->return_value = return_value;
        this->behavior = behavior;
        this->bytes = return_value.size();
    }

    std::string constructProtocol(std::vector<std::string> args, bool isArray, bool asBulkString) {

        std::string response {};
        if (isArray) {
            response += "*" + std::to_string(args.size()) + "\r\n";
        }

        for (std::string arg : args) {
            
            if (asBulkString) {
                response += arg;
            } else {
                response += "$" + std::to_string(arg.size()) + "\r\n" + arg + "\r\n";
            }

        }

        if (asBulkString) response = "$" + std::to_string(response.size()) + "\r\n" + response + "\r\n";
        return response;
    }

}