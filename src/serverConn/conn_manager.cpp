#include <vector>
#include <string>
#include <memory>

#include <serverConn/structs.h>
#include <serverConn/server_connection.h>
#include <serverConn/master.h>

#include <protocol/identifier.h>
#include <protocol/utils.h>

#include <serverConn/initializers.h>

#include <utils.h>


namespace ConnManager {

    void handle_connection(RemusConn::ConnectionManager* conn, int clientFD) {

        char buffer[128];
        while (true) {
            size_t bytes_received = recv(clientFD, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) break;

            buffer[bytes_received] = '\0';
            conn->getProtocolIdr()->identifyProtocol(buffer);

            ProtocolUtils::ReturnObject* rObject = conn->getProtocolIdr()->getRObject();
            if (rObject->sendResponse) send(clientFD, rObject->return_value.c_str(), rObject->bytes, rObject->behavior);

            if (conn->sendDBFile) {
                signed short fileLength = conn->getDbFile().size();
                rObject = new ProtocolUtils::ReturnObject("$" + std::to_string(fileLength) + "\r\n" + conn->getDbFile(), 0);
                send(clientFD, rObject->return_value.c_str(), rObject->bytes, rObject->behavior);
                conn->sendDBFile = false;

                RemusConn::Master* masterConn = static_cast<RemusConn::Master*>(conn);
                masterConn->setCurrentReplicaServerFd(clientFD);
                masterConn->addAndCleanCurrentReplicaConn();
                masterConn->inHandShakeWithReplica = false;
            }

        }
        close(clientFD);
    }

    void listener(RemusConn::ConnectionManager* conn, int serverFD) {

        PRINT_SUCCESS("Listener Started On ServerFD: " + std::to_string(serverFD));

        std::vector<std::thread> threads {};
        while (true) {
            struct sockaddr_in clientAddr {};
            int clientAddrLen = sizeof(clientAddr);
            int clientFD = accept(serverFD, (struct sockaddr *) &clientAddr, (socklen_t *) &clientAddrLen);

            threads.emplace_back(std::thread(handle_connection, conn, clientFD)).detach();
        }
        close(serverFD);
    }

    void serverLoop(int argc, char **argv) {

        std::vector<std::thread> connThreads {};
        std::vector<RemusConn::ConnectionManager*> connections = ConnInitializer::initializeConnections(argc, argv);

        for (RemusConn::ConnectionManager* conn : connections) {
            connThreads.emplace_back(std::thread(listener, conn, conn->getServerFD()));
        }

        while (true) {}
    }
}