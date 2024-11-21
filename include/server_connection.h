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
#include <config_manager.h>

namespace ServerConnection {

    void handle_connection(int clientFD);
    void listener(int serverFD);

    class ConnectionManager {

        public:

        int getServerFD();
        bool getConnectionStatus();

        ConnectionManager(RemusConfig::ConfigManager* config);
        ~ConnectionManager();

        private:

            RemusConfig::ConfigManager* config;

            int serverFD;
            bool connectionStatus;

            void createSocket();
            void checkAddress();
            void checkConnection();
    };


}