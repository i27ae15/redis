#include <serverConn/slave.h>
#include <serverConn/master.h>
#include <serverConn/conn_manager.h>
#include <protocol/utils.h>

namespace RemusConn {

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

        PRINT_HIGHLIGHT("Master fd: " + std::to_string(getMasterServerFD()));
        PRINT_HIGHLIGHT("Master port: " + std::to_string(getMasterPort()));
        PRINT_HIGHLIGHT("Master host: " + getMasterHost());

        signed short serverFD = getMasterServerFD();

        inet_pton(AF_INET, getMasterHost().c_str(), &master_addr.sin_addr);
        if (connect(serverFD, (struct sockaddr *) &master_addr, sizeof(master_addr)) != 0) {
            PRINT_ERROR("Error connecting");
            return;
        }

        std::string response = "*1\r\n$4\r\nPING\r\n";
        send(serverFD, response.c_str(), response.size(), 0);

        replicaHand = true;
        std::thread(ConnManager::handle_connection, this, serverFD).detach();
    }

    void Slave::assignMaster(Master* master) {
        this->master = master;
        assignMaster(master->getPort(), master->getServerFD(), master->getHost());
    }

    void Slave::assignMaster(signed short port, short serverFD, std::string host) {
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