#include <serverConn/master.h>

namespace RemusConn {
    // Master Class

    Master::Master(signed short port, std::string host, std::string dirName, std::string fileName) :
    ConnectionManager(port, "master", host, dirName, fileName),
    inHandShakeWithReplica {},
    currentReplicaConn {},
    replicaConns {}
    {}

    void Master::createCurrentReplicaConn() {
        currentReplicaConn = RemusConnHelper::replicaConn();
    }

    void Master::addAndCleanCurrentReplicaConn() {
        replicaConns.push_back(currentReplicaConn);
        currentReplicaConn = RemusConnHelper::replicaConn();
    }

    void Master::setCurrentReplicaPort(signed short value) {
        currentReplicaConn.port = value;
    }

    void Master::setCurrentReplicaServerFd(signed short value) {
        currentReplicaConn.serverFD = value;
    }

    void Master::propageProtocolToReplica(const std::string& buffer) {

        // rObject = new ProtocolUtils::ReturnObject("$" + std::to_string(fileLength) + "\r\n" + conn->getDbFile(), 0);
        for (RemusConnHelper::replicaConn reConn : replicaConns) {
            send(reConn.serverFD, buffer.c_str(), buffer.size(), 0);
        }
    }

}