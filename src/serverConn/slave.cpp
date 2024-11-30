#include <serverConn/slave.h>
#include <serverConn/master.h>
#include <serverConn/conn_manager.h>
#include <protocol/utils.h>

namespace RemusConn {

    Slave::Slave(unsigned short port, std::string host, std::string dirName, std::string fileName) :
    ConnectionManager(port, SLAVE, host, dirName, fileName),
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

        unsigned short serverFD = getMasterServerFD();

        inet_pton(AF_INET, getMasterHost().c_str(), &master_addr.sin_addr);
        if (connect(serverFD, (struct sockaddr *) &master_addr, sizeof(master_addr)) != 0) {
            PRINT_ERROR("Error connecting");
            return;
        }

        std::string response = "*1\r\n$4\r\nPING\r\n";
        send(serverFD, response.c_str(), response.size(), 0);

        replicaHand = true;
        std::thread(ConnManager::handleConnection, this, serverFD).detach();
    }

    void Slave::assignMaster(Master* master) {
        this->master = master;
        assignMaster(master->getPort(), master->getServerFD(), master->getHost());
    }

    void Slave::assignMaster(unsigned short port, unsigned short serverFD, std::string host) {
        masterPort = port;
        masterServerFD = serverFD;
        masterHost = host == "localhost" ? "127.0.0.1" : host;
    }

    std::string Slave::getMasterHost() {
        return masterHost;
    }

    unsigned short Slave::getMasterPort() {
        return masterPort;
    }

    unsigned short Slave::getMasterServerFD() {

        if (masterServerFD == -1) {
            masterServerFD = socket(AF_INET, SOCK_STREAM, 0);
        }

        if (masterServerFD < 0) {
            PRINT_ERROR("Failed to create master socket");
        }

        return masterServerFD;
    }

}