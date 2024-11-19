#include <string>

namespace ProtocolUtils {

    struct ReturnObject {
        std::string return_value;
        size_t bytes;
        int behavior;

        ReturnObject(std::string return_value, int behavior);
    };

    std::string constructProtocol(std::vector<std::string> args, bool isArray);

}