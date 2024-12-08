#include <serverConn/connection/base.h>
#include <serverConn/connection/slave.h>
#include <serverConn/connection/master.h>

#include <protocol/identifier.h>
#include <protocol/utils.h>

#include <serverConn/cache/cache.h>

#include <db/db_manager.h>


namespace RomulusConn {

    Cache::DataManager* BaseConnection::cache {};

    BaseConnection::BaseConnection(
        unsigned short port,
        std::string host,
        std::string dirName,
        std::string fileName
    ) :
        rs {},
        serverFD {},
        port {port},
        host {host},
        sendDBFile {},
        replicaHand {},
        bytesProcessed {},
        dirName {dirName},
        fileName {fileName},
        dbManager {nullptr},
        protocolIdr {nullptr},
        connectionStatus {true},
        id {"8371b4fb1155b71f4a04d3e1bc3e18c4a990aeeb"}
    {
        createSocket();
        checkAddress();
        checkConnection();
    }

    BaseConnection::~BaseConnection() {
        close(serverFD);
    }

    /* --------------------------------------------------------- */
    /*                      GETTERS                              */
    /* --------------------------------------------------------- */
    Cache::DataManager* BaseConnection::getCache() {
        if (!cache) {
            cache = new Cache::DataManager();
        }
        return cache;
    }

    unsigned short BaseConnection::getServerFD() {return serverFD;}

    unsigned short BaseConnection::getPort() {return port;}

    unsigned int BaseConnection::getBytesProcessed() {return bytesProcessed;}

    bool BaseConnection::getConnectionStatus() {return connectionStatus;}

    std::string BaseConnection::getDirName() {return dirName;}

    std::string BaseConnection::getFileName() {return fileName;}

    std::string BaseConnection::getHost() {return host;}

    std::string BaseConnection::getId() {return id;}

    std::string BaseConnection::getDbFile() {return dbManager->getDbFile();}

    ProtocolID::ProtocolIdentifier* BaseConnection::getProtocolIdr() {

        if (!protocolIdr->getInProcess()) return protocolIdr;

        for (ProtocolID::ProtocolIdentifier* ptc : protocols) {
            if (!ptc->getInProcess()) {
                protocolIdr = ptc;
                return ptc;
            }
        }

        print("ALL WORKERS ARE BUSY, CREATING NEW ONE", YELLOW);
        setProtocolIdr(new ProtocolID::ProtocolIdentifier(this), true);
        print("PROTCOLS OBJ: " + std::to_string(protocols.size()), YELLOW);
        return protocolIdr;
    }

    RomulusDB::DbManager* BaseConnection::getDbManager() {
        return dbManager;
    }

    /* --------------------------------------------------------- */
    /*                      SETTERS                              */
    /* --------------------------------------------------------- */

    void BaseConnection::addBytesProcessed(unsigned short bytes) {
        // PRINT_HIGHLIGHT("ADDING BYTES " + std::to_string(bytes.size()));
        if (listenToBytes) bytesProcessed += bytes;
    }

    void BaseConnection::startProcessingBytes() {
        listenToBytes = true;
    }

    void BaseConnection::setProtocolIdr(
        ProtocolID::ProtocolIdentifier* protocolIdr, bool override
    ) {

        protocols.push_back(protocolIdr);
        if (this->protocolIdr == nullptr) this->protocolIdr = protocolIdr; return;

        if (!override) {
            PRINT_ERROR("There is already a ProtocolID::ProtocolIndetifier linked to this object!");
            PRINT_ERROR("If you want to override it, set override to true");
            return;
        }

        PRINT_WARNING("Overwritting main ProtocolID::ProtocolIndetifier object!");
        this->protocolIdr = protocolIdr;
    }

    void BaseConnection::setDbManager(
        RomulusDB::DbManager* dbManager, bool override
    ) {
        if (this->dbManager == nullptr) this->dbManager = dbManager; return;

        if (!override) {
            PRINT_ERROR("There is already a RomulusDB::DbManager linked to this object!");
            PRINT_ERROR("If you want to override it, set override to true");
            return;
        }

        PRINT_WARNING("Overwritting and deleting RomulusDB::DbManager object!");
        delete this->dbManager;
        this->dbManager = dbManager;
    }

    /* --------------------------------------------------------- */
    /*                       OTHER                               */
    /* --------------------------------------------------------- */

    void BaseConnection::createSocket() {
        int serverFD = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFD < 0) {
            PRINT_ERROR("Failed to create server socket");
            connectionStatus = false;
            return;
        }

        this->serverFD = serverFD;
    }

    void BaseConnection::checkAddress() {

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

    void BaseConnection::checkConnection() {
        int connection_backlog = 5;
        if (listen(serverFD, connection_backlog) != 0) {
            PRINT_ERROR("listen failed");
            connectionStatus = false;
        }
    }

    void BaseConnection::print(std::string msg, std::string color) {
        if (getRole() == RomulusConn::SLAVE) {
            msg = "REPLICA SAYS: " + msg;
            PRINT_COLOR(color, msg);

        } else if (getRole() == RomulusConn::MASTER) {
            msg = "MASTER SAYS: " + msg;
            PRINT_COLOR(color, msg);
        }
    }

    void BaseConnection::print(std::string msg) {
        if (getRole() == RomulusConn::SLAVE) {
            print(msg, BLUE);
        } else if (getRole() == RomulusConn::MASTER) {
            print(msg, WHITE);
        }
    }
}

