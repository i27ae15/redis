#include <protocol/identifier.h>

namespace ProtocolUtils {

    ReturnObject::ReturnObject(std::string return_value, int behavior) {
        this->return_value = return_value;
        this->behavior = behavior;
        this->bytes = return_value.size();
    }

    std::string constructProtocol(std::vector<std::string> args, bool isArray) {

        std::string protocol {};
        if (isArray) {
            protocol += "*" + std::to_string(args.size()) + "\r\n";
        }

        for (std::string arg : args) {
            protocol += "$" + std::to_string(arg.size()) + "\r\n" + arg + "\r\n";
        }

        return protocol;
    }

}