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

#include <db/utils.h>
#include <protocol/utils.h>
#include <server_connection.h>


namespace ProtocolID {

    class ProtocolIdentifier {

        public:

        ProtocolIdentifier(RemusConn::ConnectionManager* conn);
        ~ProtocolIdentifier();

        ProtocolUtils::ReturnObject* getRObject();
        std::string getProtocol();
        bool identifyProtocol(const std::string& buffer);

        private:

        RemusConn::ConnectionManager* conn;
        ProtocolUtils::ReturnObject* rObject;

        std::string buffer;
        std::string protocol;
        std::string cleaned_buffer;

        std::pair<bool, size_t> getExpireTime();

        bool (ProtocolIdentifier::*checkMethods[8])();
        bool identifyPing();
        bool identifyEcho();
        bool identifySet();
        bool identifyGetNoDB();
        bool identifyGet();
        bool identifyConfig();
        bool identifyKeys();
        bool identifyInfo();

        size_t searchProtocol(std::string search_word);
        std::string getVariable(
            size_t starts_at,
            char listenOnSymbol = '$',
            char endsOnSymbol = '$'
        );

    };
}