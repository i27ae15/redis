#include <serverConn/server_connection.h>
#include <protocol/identifier.h>
#include <db/db_manager.h>

namespace RemusConn {

    ConnectionManager::ConnectionManager(int port, std::string dirName, std::string fileName, std::string role) :
    serverFD {},
    connectionStatus {true},
    port {port},
    dirName {dirName},
    fileName {fileName},
    role {role},
    dbManager {nullptr},
    protocolIdr {nullptr}
    {   

        createSocket();
        checkAddress();
        checkConnection();

        if (connectionStatus) PRINT_SUCCESS("Connection Stablished at port: " + std::to_string(port) + " As " + role);
    }

    ConnectionManager::~ConnectionManager() {
        close(serverFD);
    }

    // GETTERS

    int ConnectionManager::getServerFD() {return serverFD;}
    
    int ConnectionManager::getPort() {return port;}

    bool ConnectionManager::getConnectionStatus() {return connectionStatus;}

    std::string ConnectionManager::getDirName() {return dirName;}

    std::string ConnectionManager::getFileName() {return fileName;}

    std::string ConnectionManager::getRole() {return role;}

    ProtocolID::ProtocolIdentifier* ConnectionManager::getProtocolIdr() {
        return protocolIdr;
    }
    RemusDB::DbManager* ConnectionManager::getDbManager() {
        return dbManager;
    }

    // Setters

    void ConnectionManager::setProtocolIdr(
        ProtocolID::ProtocolIdentifier* protocolIdr, bool overWrite
    ) {
        
        if (this->protocolIdr == nullptr) this->protocolIdr = protocolIdr; return;

        if (!overWrite) {
            PRINT_ERROR("There is already a ProtocolID::ProtocolIndetifier linked to this object!");
            PRINT_ERROR("If you want to overwrite it, set overWrite to true");
            return;
        }

        PRINT_WARNING("Overwritting and deleting ProtocolID::ProtocolIndetifier object!");
        delete this->protocolIdr;
        this->protocolIdr = protocolIdr;
    }

    void ConnectionManager::setDbManager(
        RemusDB::DbManager* dbManager, bool overWrite
    ) {
        if (this->dbManager == nullptr) this->dbManager = dbManager; return;

        if (!overWrite) {
            PRINT_ERROR("There is already a RemusDB::DbManager linked to this object!");
            PRINT_ERROR("If you want to overwrite it, set overWrite to true");
            return;
        }

        PRINT_WARNING("Overwritting and deleting RemusDB::DbManager object!");
        delete this->dbManager;
        this->dbManager = dbManager;
    }

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
            PRINT_ERROR("Failed to bind to port " + std::to_string(getPort()) + " | serverFD: " + std::to_string(serverFD));
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

