#pragma once
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>
#include <functional>

#include <cache.h>
#include <utils.h>
#include <regex>
#include <config_manager.h>

#include <db/db_manager.h>
#include <db/utils.h>
#include <protocol/utils.h>

namespace ProtocolID {

    class ProtocolIdentifier {

        public:

        ProtocolIdentifier(std::string buffer);
        ~ProtocolIdentifier();

        ProtocolUtils::ReturnObject* getRObject();
        std::string getProtocol();

        private:

        ProtocolUtils::ReturnObject* rObject;
        std::string buffer;
        std::string protocol;
        std::string cleaned_buffer;

        std::pair<bool, size_t> getExpireTime();


        bool identifyProtocol();

        bool (ProtocolIdentifier::*checkMethods[6])();
        bool identifyPing();
        bool identifyEcho();
        bool identifySet();
        bool identifyGet();
        bool identifyConfig();
        bool identifyKeys();

        size_t searchProtocol(std::string search_word);
        std::string getVariable(
            size_t starts_at,
            char listenOnSymbol = '$',
            char endsOnSymbol = '$'
        );

    };
}