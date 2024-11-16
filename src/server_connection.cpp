#include <server_connection.h>

namespace ServerConnection {

    ConnectionManager::ConnectionManager() :
    server_fd {},
    connection_status {}
    {
        create_socket();
        check_address();
        check_connection();

        PRINT_SUCCESS("Connection Stablished");
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

        this->server_fd = server_fd;
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

