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
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>

#include <serverConn/cache/cache.h>
#include <utils.h>
#include <regex>

namespace ProtocolUtils {
    struct ReturnObject;
    struct CommandObj;
}

namespace RomulusConn {
    class BaseConnection;
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
    constexpr const char* WAIT = "WAIT";
    constexpr const char* INCR = "INCR";
    constexpr const char* MULTI = "MULTI";
    constexpr const char* EXEC = "EXEC";
    constexpr const char* DISCARD = "DISCARD";

    constexpr const char* PONG = "PONG";
    constexpr const char* OK = "OK";
    constexpr const char* GETACK = "GETACK";

    class ProtocolIdentifier {

        public:

        ProtocolIdentifier(RomulusConn::BaseConnection* conn);
        ~ProtocolIdentifier();

        ProtocolUtils::ReturnObject* getRObject();
        std::string getProtocol();

        void cleanResponseObject();

        bool getInProcess();

        void setInProcess(bool value);
        void setReplicasOscarKilo(unsigned short n);
        void interruptWait();

        // Shared state that will be passed to the child
        std::string getIdFromBuffer();

        static bool getProIsWaiting();

        void processProtocol(
            unsigned short clientFD,
            const std::string rawBuffer,
            const std::string command,
            const unsigned short commandSize,
            bool clearObject = true
        );

        // PRIVATE

        private:

        bool identifyProtocol(bool clearObject = true);

        void _processProtocol(
            const unsigned short clientFD,
            const std::string rawBuffer,
            const std::string command,
            const unsigned short commandSize,
            bool clearObject = true
        );

        static bool pWrite;
        static bool pIsWaiting;

        std::atomic<bool> interruptFlag;
        std::mutex mtx;
        std::condition_variable cv;

        // Should qCommands be static?
        static std::unordered_map<unsigned short, std::queue<ProtocolUtils::CommandObj>> mCommands;
        std::unordered_map<unsigned short, bool> allowExecutionOnClient;
        std::unordered_map<unsigned short, bool> multiCalledOnClient;

        bool inProcess;
        bool isExecute;

        unsigned short currentClient;
        unsigned short currentCommandSize;
        RomulusConn::BaseConnection* conn;
        ProtocolUtils::ReturnObject* rObject;

        std::string buffer;
        std::string protocol;
        std::string rawBuffer;
        std::vector<std::string> splittedBuffer;

        std::queue<ProtocolUtils::CommandObj>& getCommandQueue();

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
        bool actionForWait();
        bool actionForIncr();
        bool actionForMulti();
        bool actionForExec();
        bool actionForDiscard();

        bool processSpecialCommands();

        void processCommandQueue();
        void processDBFile(unsigned short clientFD);
        void sendResponse(
            unsigned short commandSize,
            unsigned short clientFD,
            ProtocolUtils::ReturnObject* rObj
        );
    };
}