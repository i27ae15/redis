#include <protocol/utils.h>
#include <protocol/identifier.h>

#include <serverConn/connection/master.h>
#include <serverConn/connection/structs.h>

namespace RomulusConn {
    // Master Class

    Master::Master(
        unsigned short port,
        std::string host,
        std::string dirName,
        std::string fileName
    ) :
    BaseConnection(port, host, dirName, fileName),
    inHandShakeWithReplica {},
    currentReplicaConn {},
    replicaConns {},
    nReplicasToACK {},
    nReplicasOscarKilo {}
    {
        if (getConnectionStatus()) PRINT_SUCCESS("Connection Stablished at port: " + std::to_string(port) + " As " +  getRole());
    }

    std::string Master::getRole() const {
        return MASTER;
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

    void Master::incrementReplicasOscarKilo() {
        nReplicasOscarKilo++;
    }

    void Master::setReplicasToAck(unsigned short n) {
        nReplicasToACK = n;
    }

    void Master::setReplicasOscarKilo(unsigned short n) {
        nReplicasOscarKilo = n;
    }


    void Master::createCurrentReplicaConn() {
        currentReplicaConn = RomulusConnStructs::replicaConn();
    }

    void Master::addAndCleanCurrentReplicaConn() {
        replicaConns.push_back(currentReplicaConn);
        currentReplicaConn = RomulusConnStructs::replicaConn();
    }

    void Master::setCurrentReplicaPort(unsigned short value) {
        currentReplicaConn.port = value;
    }

    void Master::setCurrentReplicaServerFd(unsigned short value) {
        currentReplicaConn.serverFD = value;
    }

    void Master::propagueProtocolToReplica(
        ProtocolID::ProtocolIdentifier *idt,
        const std::string& buffer
    ) {

        for (size_t i {}; i < getNumReplicas(); i++) {
            RomulusConnStructs::replicaConn &reConn = replicaConns[i];
            send(reConn.serverFD, buffer.c_str(), buffer.size(), 0);
        }
    }

    void Master::getReplicasACKs() {

        std::string response = ProtocolUtils::constructArray({"REPLCONF", "GETACK", "*"});

        for (size_t i {}; i < getNumReplicas(); i++) {
            RomulusConnStructs::replicaConn &reConn = replicaConns[i];
            send(reConn.serverFD, response.c_str(), response.size(), 0);
        }
    }

}