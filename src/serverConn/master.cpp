#include <protocol/utils.h>
#include <protocol/identifier.h>

#include <serverConn/master.h>
#include <serverConn/structs.h>

namespace RemusConn {
    // Master Class

    Master::Master(unsigned short port, std::string host, std::string dirName, std::string fileName) :
    ConnectionManager(port, MASTER, host, dirName, fileName),
    inHandShakeWithReplica {},
    currentReplicaConn {},
    replicaConns {},
    nReplicasToACK {},
    nReplicasOscarKilo {}
    {}

    void Master::incrementReplicasOscarKilo() {
        PRINT_HIGHLIGHT("INCREMENTING OSCAR KILO");
        nReplicasOscarKilo++;
    }

    void Master::setReplicasToAck(unsigned short n) {
        nReplicasToACK = n;
    }

    void Master::setReplicasOscarKilo(unsigned short n) {
        nReplicasOscarKilo = n;
    }

    unsigned short Master::getNumReplicas() {
        return replicaConns.size();
    }

    unsigned short Master::getReplicasToAck() {
        return nReplicasToACK;
    }

    unsigned short Master::getReplicasOscarKilo() {
        return nReplicasOscarKilo;
    }

    void Master::createCurrentReplicaConn() {
        currentReplicaConn = RemusConnStructs::replicaConn();
    }

    void Master::addAndCleanCurrentReplicaConn() {
        replicaConns.push_back(currentReplicaConn);
        currentReplicaConn = RemusConnStructs::replicaConn();
    }

    void Master::setCurrentReplicaPort(unsigned short value) {
        currentReplicaConn.port = value;
    }

    void Master::setCurrentReplicaServerFd(unsigned short value) {
        currentReplicaConn.serverFD = value;
    }

    void Master::propagueProtocolToReplica(ProtocolID::ProtocolIdentifier *idt, const std::string& buffer) {

        for (size_t i {}; i < getNumReplicas(); i++) {
            RemusConnStructs::replicaConn &reConn = replicaConns[i];
            send(reConn.serverFD, buffer.c_str(), buffer.size(), 0);
        }
    }

    void Master::getReplicasACKs() {

        std::string response = ProtocolUtils::constructArray({"REPLCONF", "GETACK", "*"});

        for (size_t i {}; i < getNumReplicas(); i++) {
            RemusConnStructs::replicaConn &reConn = replicaConns[i];
            send(reConn.serverFD, response.c_str(), response.size(), 0);
        }
    }

}