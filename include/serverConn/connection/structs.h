#pragma once
#include <string>
#include <vector>

#include <utils.h>

namespace RomulusConnStructs {

    struct connConf {
        std::string dirName;
        std::string fileName;
        std::string host;
        std::string role;
        int port;
    };

    struct replicaConn {
        unsigned short port;
        unsigned short serverFD;
    };

    struct connConfigs {
        std::vector<connConf> conns;
        std::string masterHost;
        unsigned short masterPort;
    };

}