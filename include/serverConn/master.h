#pragma once
#include <serverConn/server_connection.h>
#include <serverConn/structs.h>

namespace RemusConn {

    class Master : public ConnectionManager {

        public:
            Master(
                unsigned short port,
                std::string host,
                std::string dirName = "",
                std::string fileName = ""
            );

            void incrementReplicasOscarKilo();
            void setReplicasToAck(unsigned short n);
            void setReplicasOscarKilo(unsigned short n);

            unsigned short getNumReplicas();
            unsigned short getReplicasToAck();
            unsigned short getReplicasOscarKilo();

            void propagueProtocolToReplica(ProtocolID::ProtocolIdentifier *idt, const std::string& buffer);

            void createCurrentReplicaConn();
            void setCurrentReplicaPort(unsigned short value);
            void setCurrentReplicaServerFd(unsigned short value);
            void addAndCleanCurrentReplicaConn();
            void getReplicasACKs();

            bool inHandShakeWithReplica;

        private:
            unsigned short nReplicasToACK;
            unsigned short nReplicasOscarKilo;

            RemusConnStructs::replicaConn currentReplicaConn;
            std::vector<RemusConnStructs::replicaConn> replicaConns;

    };
}