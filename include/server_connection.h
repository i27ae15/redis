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

#define BUFFER_SIZE 1024


namespace ServerConnection {

    void handle_connection(int client_fd, char[BUFFER_SIZE] buffer);
    void listener(int& server_fd, char[BUFFER_SIZE] buffer);

    class ConnectionManager {

        public:

        int get_server_fd();
        bool get_connection_status();

        ConnectionManager();
        ~ConnectionManager();

        private:

            std::vector<std::thread> threads;
            char buffer[BUFFER_SIZE];
            int server_fd;
            bool connection_status;

            void create_socket();
            void check_address();
            void check_connection();
    };


}