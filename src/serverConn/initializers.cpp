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

#include <serverConn/initializers.h>
#include <serverConn/structs.h>
#include <serverConn/master.h>
#include <serverConn/slave.h>

#include <db/db_manager.h>
#include <protocol/identifier.h>


namespace ConnInitializer {

    RemusConnStructs::connConfigs configInitializer(int argc, char** argv) {

        int port = 6379;
        int masterPort {};

        std::string host = "localhost";
        std::string dirName {}; // This might be a static variable
        std::string fileName {};
        std::string masterHost {};
        RemusConnStructs::connConfigs configs {};

        std::array<std::string, 4> flagCofig = {"--dir", "--dbfilename", "--port", "--replicaof"};

        for (int i = 1; i < argc; ++i) {
            std::string key = argv[i];
            if (key.size() < 2 || ( key[0] != '-' && key[1] != '-')) continue;

            std::string input = argv[i+1];
            if (std::find(flagCofig.begin(), flagCofig.end(), key) != flagCofig.end()) {

                PRINT_COLOR(GREEN, key);
                PRINT_COLOR(BLUE, input);

                if (key == "--replicaof") {

                    std::vector<std::string> mValues = RemusUtils::splitString(input, " ");

                    masterHost = mValues[0];
                    masterPort = std::stoi(mValues[1]);

                } else if (key == "--dir") { dirName = input;
                } else if (key == "--dbfilename") { fileName = input;
                } else if (key == "--port") port = std::stoi(input);

            } else {
                PRINT_ERROR("No method found for key: " + key);
            }
        }

        // Check if we don't have a master
        if (!masterPort) {
            configs.conns.emplace_back(RemusConnStructs::connConf{dirName, fileName, host, "master", port});
            masterPort = port;
            masterHost = host;
        } else {
            configs.conns.emplace_back(RemusConnStructs::connConf{dirName, fileName, host, "slave", port});
        }

        configs.masterHost = masterHost;
        configs.masterPort = masterPort;

        return configs;
    }

    std::vector<RemusConn::ConnectionManager*> initializeConnections(int argc, char **argv) {

        RemusConnStructs::connConfigs connConfigs = configInitializer(argc, argv);
        std::vector<RemusConn::ConnectionManager*> conns {};

        for (RemusConnStructs::connConf connConf : connConfigs.conns) {

            RemusConn::ConnectionManager* newConn {nullptr};

            if (connConf.role == "master") {
                newConn = new RemusConn::Master(connConf.port, connConf.host, connConf.dirName, connConf.fileName);
            } else {
                RemusConn::Slave* slaveConn = new RemusConn::Slave(connConf.port, connConf.host, connConf.dirName, connConf.fileName);
                slaveConn->assignMaster(connConfigs.masterPort, -1, connConfigs.masterHost);
                slaveConn->handShakeWithMaster();
                newConn = slaveConn;
            }

            if (!newConn->getConnectionStatus()) throw std::runtime_error("Connection error");

            // We need to create a protocol id and a dbManager
            newConn->setDbManager(new RemusDB::DbManager(newConn));
            newConn->setProtocolIdr(new ProtocolID::ProtocolIdentifier(newConn));

            conns.push_back(newConn);
        }

        return conns;

    }
}

