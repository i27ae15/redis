#include <server_connection.h>
#include <protocol/identifier.h>

namespace RemusConn {

    // ./your_program.sh --port <PORT> --replicaof "<MASTER_HOST> <MASTER_PORT>"

    void ConfigManager::methodRouter(int argc, char** argv) {

        int port = 6379;
        std::dirName {};
        std::fileName {};
        std::role {};

        std::unordered_map<std::string, std::string> flags {};

        flags["--dir"] = "";
        flags["--dbfilename"] = "";
        flags["--port"] = "";

        std::string masterHost {};
        std::string masterPort {};

        std::vector<std::string> flags {"--dir", "--dbfilename", "--port", "--replicaof"}
        for (int i = 1; i < argc; ++i) {

            std::string key = argv[i];
            if (key.size() < 2 || ( key[0] != '-' && key[1] != '-')) continue;

            std::string input = argv[i+1];
            if (flags.find(key) != flags.end()) {

                if (key == "replicaof") {
                    flags["role"] = "slave";

                    masterHost = input;
                    masterPort = argv[i+2];
                    continue;
                }

                flags[key] = input;
            } else {
                PRINT_ERROR("No method found for key: " + key);
            }
        }

    }

    void handle_connection(int clientFD) {

        char buffer[1024];
        while (true) {
            size_t bytes_received = recv(clientFD, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) break;

            buffer[bytes_received] = '\0';

            ProtocolID::ProtocolIdentifier protocolId(buffer);

            ProtocolUtils::ReturnObject* rObject = protocolId.getRObject();
            send(clientFD, rObject->return_value.c_str(), rObject->bytes, rObject->behavior);
        }
        close(clientFD);
    }

    void listener(int serverFD) {

        PRINT_SUCCESS("Listener Started");

        std::vector<std::thread> threads {};
        while (true) {
            struct sockaddr_in clientAddr {};
            int clientAddrLen = sizeof(clientAddr);
            int clientFD = accept(serverFD, (struct sockaddr *) &clientAddr, (socklen_t *) &clientAddrLen);

            threads.emplace_back(std::thread(handle_connection, clientFD)).detach();
        }
        close(serverFD);
    }

    ConnectionManager::ConnectionManager(int port, std::string dirName, std::string fileName) :
    serverFD {},
    connectionStatus {true},
    port {port},
    dirName {dirName},
    fileName {fileName}
    {
        createSocket();
        checkAddress();
        checkConnection();

        if (connectionStatus) PRINT_SUCCESS("Connection Stablished");
    }

    ConnectionManager::~ConnectionManager() {
        close(serverFD);
    }

    // GETTERS

    int ConnectionManager::getServerFD() {return serverFD;}

    bool ConnectionManager::getConnectionStatus() {return connectionStatus;}

    std::string ConnectionManager::getDirName() {return dirName;}

    std::string ConnectionManager::getFileName() {return fileName;}

    int ConnectionManager::getPort() {return port;}

    // METHODS

    void ConnectionManager::createSocket() {
        int serverFD = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFD < 0) {
            PRINT_ERROR("Failed to create server socket");
            connectionStatus = false;
            return;
            }

        this->serverFD = serverFD;
    }

    void ConnectionManager::checkAddress() {

        // Since the tester restarts your program quite often, setting SO_REUSEADDR
        // ensures that we don't run into 'Address already in use' errors
        int reuse = 1;
        if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            PRINT_ERROR("setsockopt failed");
            connectionStatus = false;
            return;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(getPort());

        if (bind(serverFD, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
            connectionStatus = false;
            PRINT_ERROR("Failed to bind to port " + getPort());
            return;
        }
    }

    void ConnectionManager::checkConnection() {
        int connection_backlog = 5;
        if (listen(serverFD, connection_backlog) != 0) {
            PRINT_ERROR("listen failed");
            connectionStatus = false;
        }
    }

}

