#pragma once
#include <serverConn/connection/structs.h>
#include <serverConn/connection/base.h>

namespace RomulusConn {
    class BaseConnection;
}

namespace ConnInitializer {
    RomulusConnStructs::connConfigs configInitializer(int argc, char** argv);
    std::vector<RomulusConn::BaseConnection*> initializeConnections(int argc, char **argv);
}