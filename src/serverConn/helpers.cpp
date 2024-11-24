#include <vector>
#include <string>
#include <memory>

#include <serverConn/helpers.h>
#include <serverConn/server_connection.h>

#include <protocol/identifier.h>
#include <protocol/utils.h>

#include <db/db_manager.h>

#include <protocol/identifier.h>

#include <utils.h>


namespace RemusConnHelper {

    connConfigs configInitializer(int argc, char** argv) {

        int port = 6379;
        int masterPort {};

        std::string host = "localhost";
        std::string dirName {}; // This might be a static variable
        std::string fileName {};
        std::string masterHost {};
        connConfigs configs {};

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
            configs.conns.emplace_back(RemusConnHelper::connConf{dirName, fileName, host, "master", port});
            masterPort = port;
            masterHost = host;
        } else {
            configs.conns.emplace_back(RemusConnHelper::connConf{dirName, fileName, host, "slave", port});
        }

        configs.masterHost = masterHost;
        configs.masterPort = masterPort;

        return configs;
    }

    void handle_connection(RemusConn::ConnectionManager* conn, int clientFD) {

        char buffer[1024];
        while (true) {
            size_t bytes_received = recv(clientFD, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) break;

            buffer[bytes_received] = '\0';

            conn->getProtocolIdr()->identifyProtocol(buffer);

            ProtocolUtils::ReturnObject* rObject = conn->getProtocolIdr()->getRObject();
            send(clientFD, rObject->return_value.c_str(), rObject->bytes, rObject->behavior);
        }
        close(clientFD);
    }

    void listener(RemusConn::ConnectionManager* conn) {

        PRINT_SUCCESS("Listener Started");

        std::vector<std::thread> threads {};
        while (true) {
            struct sockaddr_in clientAddr {};
            int clientAddrLen = sizeof(clientAddr);
            int clientFD = accept(conn->getServerFD(), (struct sockaddr *) &clientAddr, (socklen_t *) &clientAddrLen);

            threads.emplace_back(std::thread(handle_connection, conn, clientFD)).detach();
        }
        close(conn->getServerFD());
    }

    void initializeListener(RemusConn::ConnectionManager* conn) {
        std::vector<std::thread> threads {};
        threads.emplace_back(std::thread(listener, conn));
    }

    std::vector<RemusConn::ConnectionManager*> initializeConnections(int argc, char **argv) {

        connConfigs connConfigs = configInitializer(argc, argv);
        std::vector<RemusConn::ConnectionManager*> conns {};

        for (RemusConnHelper::connConf connConf : connConfigs.conns) {

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

            RemusConnHelper::listener(newConn);
            conns.push_back(newConn);
        }

        return conns;

    }
}