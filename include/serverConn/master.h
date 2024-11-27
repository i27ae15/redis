#pragma once
#include <serverConn/server_connection.h>

namespace RemusConn {

    class Master : public ConnectionManager {

        public:
            Master(signed short port, std::string host,
             std::string dirName = "", std::string fileName = "");


            void propageProtocolToReplica(const std::string& buffer);

            void createCurrentReplicaConn();
            void setCurrentReplicaPort(signed short value);
            void setCurrentReplicaServerFd(signed short value);
            void addAndCleanCurrentReplicaConn();

            bool inHandShakeWithReplica;

        private:
            RemusConnHelper::replicaConn currentReplicaConn;
            std::vector<RemusConnHelper::replicaConn> replicaConns;

    };
}