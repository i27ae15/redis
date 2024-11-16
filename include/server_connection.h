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

    class ConnectionManager {

        public:

        int get_server_fd();
        bool get_connection_status();

        private:

            std::vector<std::thread>> threads;
            char buffer[BUFFER_SIZE];
            int server_fd;
            bool connection_status;

            ConnectionManager();

            void create_socket();
            void check_address();
            void check_connection();
            void handle_connection(int client_fd);
            void listener();

    };

}