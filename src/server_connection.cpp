#include "server_connection.h"

namespace ServerConnection {

    void handle_connection(int client_fd) {
        char[1024] buffer {};
        while (true) {
            size_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) break;

            buffer[bytes_received] = '\0';

            if (strcasecmp(buffer,"*1\r\n$4\r\nping\r\n") == 0 ) {
            send(client_fd, "+PONG\r\n", 7, 0);
            }
        }
        close(client_fd);
    }

    void listener(int server_fd) {
        while (true) {
            struct sockaddr_in client_addr {};
            int client_addr_len = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);

            threads.emplace_back(std::thread(handle_connection, client_fd)).detach();
        }
        close(server_fd);
    }

    ConnectionManager::ConnectionManager() :
    server_fd {},
    connection_status {}
    {
        create_socket();
        check_address();
        check_connection();
    }

    ConnectionManager::~ConnectionManager() {
        close(server_fd);
    }

    // GETTERS

    int ConnectionManager::get_server_fd() {return server_fd;}

    bool ConnectionManager::get_connection_status() {return connection_status;}

    // METHODS

    void ConnectionManager::create_socket() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "Failed to create server socket\n";
            connection_status = false;
            return;
            }

        this->server_fd;
    }

    void ConnectionManager::check_address() {

        // Since the tester restarts your program quite often, setting SO_REUSEADDR
        // ensures that we don't run into 'Address already in use' errors
        int reuse = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            std::cerr << "setsockopt failed\n";
            connection_status = false;
            return;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(6379);

        if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
            connection_status = false;
            std::cerr << "Failed to bind to port 6379\n";
            return;
        }
    }

    void ConnectionManager::check_connection() {
        int connection_backlog = 5;
        if (listen(server_fd, connection_backlog) != 0) {
            std::cerr << "listen failed\n";
            connection_status = false;
        }
    }

}

