#include <serverConn/structs.h>

namespace RemusConn {
    class ConnectionManager;
}

namespace ConnInitializer {
    RemusConnStructs::connConfigs configInitializer(int argc, char** argv);
    std::vector<RemusConn::ConnectionManager*> initializeConnections(int argc, char **argv);
}