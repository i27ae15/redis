#pragma once
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>
#include <atomic>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <utils.h>

namespace RemusDB {
    class DbManager;
}

namespace ProtocolID {
    class ProtocolIdentifier;
}

namespace RemusConn {

    constexpr const char* MASTER = "MASTER";
    constexpr const char* SLAVE = "SLAVE";

    class ConnectionManager {

        public:

        ConnectionManager(
            unsigned short port, std::string role, std::string host,
            std::string dirName = "", std::string fileName = ""
        );

        int rs;
        bool sendDBFile;
        bool replicaHand;

        // Methods

        virtual ~ConnectionManager();

        // Getters

        unsigned short getServerFD();
        unsigned short getPort();
        unsigned int getBytesProcessed();

        bool getConnectionStatus();

        std::string getHost();
        std::string getDirName();
        std::string getFileName();
        std::string getId();

        void setProtocolIdr(ProtocolID::ProtocolIdentifier* protocolIdr, bool overWrite = false);
        void setDbManager(RemusDB::DbManager* dbManager, bool overWrite = false);

        std::string getDbFile();

        ProtocolID::ProtocolIdentifier* getProtocolIdr();
        RemusDB::DbManager* getDbManager();
        std::string getRole();

        // Setters

        void addBytesProcessed(unsigned short bytes);
        void startProcessingBytes();

        // Utilities

        void print(std::string msg);
        void print(std::string msg, std::string color);


        private:

            unsigned int bytesProcessed;

            bool listenToBytes;

            const std::string id;

            std::vector<ProtocolID::ProtocolIdentifier*> protocols;

            ProtocolID::ProtocolIdentifier* protocolIdr;
            RemusDB::DbManager* dbManager;

            std::string dirName;
            std::string fileName;
            std::string role;
            std::string host;

            unsigned short port;
            unsigned short serverFD;

            bool connectionStatus;

            void createSocket();
            void checkAddress();
            void checkConnection();
    };
}