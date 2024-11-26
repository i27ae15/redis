#include <serverConn/server_connection.h>
#include <protocol/identifier.h>
#include <protocol/utils.h>
#include <db/db_manager.h>

namespace RemusConn {

    ConnectionManager::ConnectionManager(
        signed short port, std::string role, std::string host,
        std::string dirName, std::string fileName) :
    serverFD {},
    connectionStatus {true},
    port {port},
    dirName {dirName},
    fileName {fileName},
    role {role},
    host {host},
    dbManager {nullptr},
    protocolIdr {nullptr},
    id {"8371b4fb1155b71f4a04d3e1bc3e18c4a990aeeb"}
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

    signed short ConnectionManager::getServerFD() {return serverFD;}

    signed short ConnectionManager::getPort() {return port;}

    bool ConnectionManager::getConnectionStatus() {return connectionStatus;}

    std::string ConnectionManager::getDirName() {return dirName;}

    std::string ConnectionManager::getFileName() {return fileName;}

    std::string ConnectionManager::getRole() {return role;}

    std::string ConnectionManager::getHost() {return host;}

    std::string ConnectionManager::getId() {return id;}

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

    Master::Master(signed short port, std::string host, std::string dirName, std::string fileName) :
    ConnectionManager(port, "master", host, dirName, fileName) {}

    Slave::Slave(signed short port, std::string host, std::string dirName, std::string fileName) :
    ConnectionManager(port, "slave", host, dirName, fileName),
    handShakedWithMaster {},
    master {nullptr}
    {
    }

    bool Slave::isInHandShake() {
        if (!handShakedWithMaster && handShakeStep > 0) return true;
        return false;
    }

    void Slave::handShakeWithMaster() {
        if (handShakedWithMaster) return;
        handShakeStep = 1;

        // First Step
        struct sockaddr_in master_addr;
        master_addr.sin_family = AF_INET;
        master_addr.sin_port = htons(getMasterPort());
        master_addr.sin_addr.s_addr = INADDR_ANY;

        inet_pton(AF_INET, getMasterHost().c_str(), &master_addr.sin_addr);
        if (connect(getMasterServerFD(), (struct sockaddr *) &master_addr, sizeof(master_addr)) != 0) {
            PRINT_ERROR("Error connecting");
            return;
        }

        std::string response = "*1\r\n$4\r\nPING\r\n";
        send(getMasterServerFD(), response.c_str(), response.size(), 0);

        // Wait for "PONG" from the master
        char buffer[1024] = {0};
        signed int bytesReceived {};

        bytesReceived = recv(getMasterServerFD(), buffer, sizeof(buffer) - 1, 0);
        buffer[bytesReceived] = '\0';
        response = ProtocolUtils::constructProtocol({"REPLCONF", "listening-port", std::to_string(getPort())}, true);
        send(getMasterServerFD(), response.c_str(), response.size(), 0);

        bytesReceived = recv(getMasterServerFD(), buffer, sizeof(buffer) - 1, 0);
        buffer[bytesReceived] = '\0';
        response = ProtocolUtils::constructProtocol({"REPLCONF", "capa", "psync2"}, true);
        send(getMasterServerFD(), response.c_str(), response.size(), 0);

        handShakedWithMaster = true;
        PRINT_SUCCESS("Hand shake stablished");

        // Step 3

        bytesReceived = recv(getMasterServerFD(), buffer, sizeof(buffer) - 1, 0);
        buffer[bytesReceived] = '\0';
        response = ProtocolUtils::constructProtocol({"PSYNC", "?", "-1"}, true);
        send(getMasterServerFD(), response.c_str(), response.size(), 0);

    }

    void Slave::assignMaster(Master* master) {
        this->master = master;
        assignMaster(master->getPort(), master->getServerFD(), master->getHost());
    }

    void Slave::assignMaster(signed short port, signed short serverFD, std::string host) {
        masterPort = port;
        masterServerFD = serverFD;
        masterHost = host == "localhost" ? "127.0.0.1" : host;
    }

    std::string Slave::getMasterHost() {
        return masterHost;
    }

    signed short Slave::getMasterPort() {
        return masterPort;
    }

    signed short Slave::getMasterServerFD() {

        if (masterServerFD == -1) {
            masterServerFD = socket(AF_INET, SOCK_STREAM, 0);
        }

        if (masterServerFD < 0) {
            PRINT_ERROR("Failed to create master socket");
        }

        return masterServerFD;
    }

}

