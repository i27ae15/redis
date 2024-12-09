#pragma once
#include <serverConn/connection/structs.h>
#include <serverConn/connection/base.h>
#include <serverConn/connection/slave.h>

namespace RomulusConn {
    class BaseConnection;
}

namespace ConnInitializer {
    void replicaHandShake(
        RomulusConn::Slave* conn,
        std::string buffer,
        int clientFD
    );
    RomulusConnStructs::connConfigs configInitializer(int argc, char** argv);
    std::vector<RomulusConn::BaseConnection*> initializeConnections(int argc, char **argv);

}