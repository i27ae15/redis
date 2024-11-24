#pragma once
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>
#include <unordered_map>
#include <array>
#include <algorithm>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <utils.h>

namespace RemusConn {
    class ConnectionManager;
}

namespace RemusConnHelper {

    struct connConf {
        std::string dirName;
        std::string fileName;
        std::string host;
        std::string role;
        int port;
    };

    struct connConfigs {
        std::vector<connConf> conns;
        std::string masterHost;
        signed short masterPort;
    };

    connConfigs configInitializer(int argc, char** argv);
    std::vector<RemusConn::ConnectionManager*> initializeConnections(int argc, char **argv);

    void handle_connection(RemusConn::ConnectionManager* conn, int clientFD);
    void listener(RemusConn::ConnectionManager* conn);
    void initializeListener(RemusConn::ConnectionManager* conn);

}