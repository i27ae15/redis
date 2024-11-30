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
#include <map>

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
        bool identifyProtocol(const std::string rawBuffer, const std::string command, const unsigned short commandSize, bool clearObject = true);

        void cleanResponseObject();

        bool getInProcess();
        void setInProcess(bool value);

        std::string getIdFromBuffer();

        private:

        bool inProcess;

        RemusConn::ConnectionManager* conn;
        ProtocolUtils::ReturnObject* rObject;

        std::string buffer;
        std::string protocol;
        std::string rawBuffer;
        std::vector<std::string> splittedBuffer;

        void setSplitedBuffer();

        std::unordered_map<std::string, std::function<bool()>> checkMethods;
        bool actionForPing();
        bool actionForEcho();
        bool actionForSet();
        bool actionForGet();
        bool actionForGetNoDB();
        bool actionForGetWithDB();
        bool actionForConfig();
        bool actionForKeys();
        bool actionForInfo();
        bool actionForReplConf();
        bool actionForReplConfMaster();
        bool actionForReplConfSlave();
        bool actionForPsync();
        bool actionForFullResync();
    };
}