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

        int getServerFD();
        bool getConnectionStatus();

        // Getters
        std::string getDirName();
        std::string getFileName();
        int getPort();

        ConnectionManager(int port = 6379, std::string dirName = "", std::string fileName = "", std::string role = "master");
        ~ConnectionManager();

        void setProtocolIdr(ProtocolID::ProtocolIdentifier* protocolIdr, bool overWrite = false);
        void setDbManager(RemusDB::DbManager* dbManager, bool overWrite = false);

        ProtocolID::ProtocolIdentifier* getProtocolIdr();
        RemusDB::DbManager* getDbManager();
        std::string getRole();

        private:
            ProtocolID::ProtocolIdentifier* protocolIdr;
            RemusDB::DbManager* dbManager;

            std::string dirName;
            std::string fileName;
            std::string role;

            int port;

            int serverFD;
            bool connectionStatus;

            void createSocket();
            void checkAddress();
            void checkConnection();
    };


}