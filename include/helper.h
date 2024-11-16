#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>

namespace Helper {

    struct ReturnObject {
        std::string return_value;
        size_t bytes;
        int behavior;

        ReturnObject(std::string return_value, int behavior);

    };

    class ProtocolIdentifier {

        public:

        ProtocolIdentifier(std::string buffer);
        ~ProtocolIdentifier();


        ReturnObject* getRObject();
        std::string getProtocol();

        private:

        ReturnObject* rObject;
        std::string buffer;
        std::string protocol;

        bool identify_protocol();
        bool identify_ping();
        bool identify_echo();

    };
}