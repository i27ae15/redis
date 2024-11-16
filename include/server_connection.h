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

namespace ServerConnection {

    void handle_connection(int client_fd);
    void listener(int server_fd);

    class ConnectionManager {

        public:

        int get_server_fd();
        bool get_connection_status();

        ConnectionManager();
        ~ConnectionManager();

        private:

            int server_fd;
            bool connection_status;

            void create_socket();
            void check_address();
            void check_connection();
    };


}