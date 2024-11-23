#include <vector>
#include <string>

#include <serverConn/helpers.h>
#include <serverConn/server_connection.h>

#include <protocol/identifier.h>
#include <protocol/utils.h>

#include <utils.h>


namespace RemusConnHelper {

    std::vector<RemusConnHelper::connConf> configInitializer(int argc, char** argv) {

        int port = 6379;
        int masterPort {};

        std::string host = "localhost";
        std::string dirName {}; // This might be a static variable
        std::string fileName {};
        std::string masterHost {};

        std::array<std::string, 4> flagCofig = {"--dir", "--dbfilename", "--port", "--replicaof"};
        std::vector<RemusConnHelper::connConf> connections {};

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
            connections.emplace_back(RemusConnHelper::connConf{dirName, fileName, host, "master", port});
            return connections;
        }
        // connections.emplace_back(RemusConnHelper::connConf{dirName, fileName, masterHost, "master", masterPort});
        connections.emplace_back(RemusConnHelper::connConf{dirName, fileName, host, "slave", port});

        return connections;
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
}