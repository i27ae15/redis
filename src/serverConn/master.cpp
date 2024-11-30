#include <serverConn/master.h>
#include <serverConn/structs.h>

namespace RemusConn {
    // Master Class

    Master::Master(unsigned short port, std::string host, std::string dirName, std::string fileName) :
    ConnectionManager(port, MASTER, host, dirName, fileName),
    inHandShakeWithReplica {},
    currentReplicaConn {},
    replicaConns {}
    {}

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

    void Master::propageProtocolToReplica(const std::string& buffer) {

        // rObject = new ProtocolUtils::ReturnObject("$" + std::to_string(fileLength) + "\r\n" + conn->getDbFile(), 0);
        for (RemusConnStructs::replicaConn reConn : replicaConns) {
            send(reConn.serverFD, buffer.c_str(), buffer.size(), 0);
        }
    }

}