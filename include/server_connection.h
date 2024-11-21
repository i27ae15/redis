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

namespace RemusConn {

    void handle_connection(int clientFD);
    void listener(int serverFD);

    class ConnectionManager {

        public:

        int getServerFD();
        bool getConnectionStatus();

        // Getters
        std::string getDirName();
        std::string getFileName();
        int getPort();

        ConnectionManager(int port = 6379, std::string dirName = "", std::string fileName = "");
        ~ConnectionManager();

        private:

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