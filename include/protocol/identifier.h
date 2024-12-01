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

namespace ProtocolUtils {
    class ReturnObject;
}

namespace RemusConn {
    class ConnectionManager;
}

namespace ProtocolID {

    constexpr const char* PING = "PING";
    constexpr const char* ECHO = "ECHO";
    constexpr const char* GET = "GET";
    constexpr const char* SET = "SET";
    constexpr const char* CONFIG = "CONFIG";
    constexpr const char* KEYS = "KEYS";
    constexpr const char* INFO = "INFO";
    constexpr const char* REPLCONF = "REPLCONF";
    constexpr const char* PSYNC = "PSYNC";
    constexpr const char* FULLRESYNC = "FULLRESYNC";

    constexpr const char* PONG = "PONG";
    constexpr const char* OK = "OK";
    constexpr const char* GETACK = "GETACK";

    class ProtocolIdentifier {

        public:

        ProtocolIdentifier(RemusConn::ConnectionManager* conn);
        ~ProtocolIdentifier();

        ProtocolUtils::ReturnObject* getRObject();
        std::string getProtocol();
        bool identifyProtocol(const std::string rawBuffer, const std::string buffer, bool clearRobject = true);

        void cleanResponseObject();

        bool getInProcess();
        void setInProcess(bool value);

        private:

        bool inProcess;

        RemusConn::ConnectionManager* conn;
        ProtocolUtils::ReturnObject* rObject;

        std::string buffer;
        std::string protocol;
        std::string rawBuffer;
        std::vector<std::string> splittedBuffer;

        std::pair<bool, size_t> getExpireTime();

        void setSplitedBuffer();

        bool (ProtocolIdentifier::*checkMethods[11])();
        bool identifyPing();
        bool identifyEcho();
        bool identifySet();
        bool identifyGetNoDB();
        bool identifyGet();
        bool identifyConfig();
        bool identifyKeys();
        bool identifyInfo();
        bool identifyReplConfi();
        bool identifyPsync();
        bool identifyFullResync();

        size_t searchProtocol(std::string search_word);
        std::string getVariable(
            size_t starts_at,
            bool cleanFrontDigits = true,
            signed short avoidNChars = 0,
            char listenOnSymbol = '$',
            char endsOnSymbol = '$'
        );

    };
}