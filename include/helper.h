#pragma
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>
#include <cache.h>

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
        std::string cleaned_buffer;

        bool identify_protocol();
        bool identifyPing();
        bool identifyEcho();
        bool identifySet();
        bool identifyGet();

        std::string constructProtocol(std::vector<std::string> args, bool isArray);

        size_t searchProtocol(std::string search_word);
        std::string getVariable(
            size_t starts_at,
            char listenOnSymbol = '$',
            char endsOnSymbol = '$'
        );

    };
}