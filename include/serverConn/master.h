#pragma once
#include <serverConn/server_connection.h>
#include <serverConn/structs.h>

namespace RemusConn {

    class Master : public ConnectionManager {

        public:
            Master(unsigned short port, std::string host,
             std::string dirName = "", std::string fileName = "");


            void propageProtocolToReplica(const std::string& buffer);

            void createCurrentReplicaConn();
            void setCurrentReplicaPort(unsigned short value);
            void setCurrentReplicaServerFd(unsigned short value);
            void addAndCleanCurrentReplicaConn();

            bool inHandShakeWithReplica;

        private:
            RemusConnStructs::replicaConn currentReplicaConn;
            std::vector<RemusConnStructs::replicaConn> replicaConns;

    };
}