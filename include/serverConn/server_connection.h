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

    class ConnectionManager {

        public:

        // Getters

        signed short getServerFD();
        signed short getPort();

        bool getConnectionStatus();

        std::string getHost();
        std::string getDirName();
        std::string getFileName();
        std::string getId();

        ConnectionManager(
            signed short port, std::string role, std::string host,
            std::string dirName = "", std::string fileName = ""
        );

        ~ConnectionManager();

        void setProtocolIdr(ProtocolID::ProtocolIdentifier* protocolIdr, bool overWrite = false);
        void setDbManager(RemusDB::DbManager* dbManager, bool overWrite = false);

        bool sendDBFile;
        std::string getDbFile();

        ProtocolID::ProtocolIdentifier* getProtocolIdr();
        RemusDB::DbManager* getDbManager();
        std::string getRole();

        private:

            const std::string id;

            ProtocolID::ProtocolIdentifier* protocolIdr;
            RemusDB::DbManager* dbManager;

            std::string dirName;
            std::string fileName;
            std::string role;
            std::string host;

            signed short port;
            signed short serverFD;

            bool connectionStatus;

            void createSocket();
            void checkAddress();
            void checkConnection();
    };

    class Master : public ConnectionManager {

        public:
            Master(signed short port, std::string host,
             std::string dirName = "", std::string fileName = "");

    };

    class Slave : public ConnectionManager {

        public:
            Slave(signed short port, std::string host,
             std::string dirName = "", std::string fileName = "");

            void assignMaster(Master* master);
            void assignMaster(signed short port, signed short serverFD, std::string host);
            void handShakeWithMaster();

            signed short getMasterPort();
            signed short getMasterServerFD();

            bool isInHandShake();

            std::string getMasterHost();

        private:

            Master* master;

            std::string masterHost;

            bool handShakedWithMaster;

            signed short handShakeStep;
            signed short masterPort;
            signed short masterServerFD;

    };


}