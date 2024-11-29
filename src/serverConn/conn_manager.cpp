#include <vector>
#include <string>
#include <memory>

#include <serverConn/structs.h>
#include <serverConn/server_connection.h>
#include <serverConn/master.h>

#include <protocol/identifier.h>
#include <protocol/utils.h>

#include <serverConn/initializers.h>
#include <serverConn/slave.h>

#include <utils.h>


namespace ConnManager {

    void replicaHandShake(RemusConn::ConnectionManager* conn, std::string buffer, int clientFD) {
        std::string response {};
        if (conn->rs == 0) {
            // Response after pong
            response = ProtocolUtils::constructProtocol({"REPLCONF", "listening-port", std::to_string(conn->getPort())}, true);
            send(clientFD, response.c_str(), response.size(), 0);
            conn->rs++;

        } else if (conn->rs == 1) {
            // Response after first OK
            response = ProtocolUtils::constructProtocol({"REPLCONF", "capa", "psync2"}, true);
            send(clientFD, response.c_str(), response.size(), 0);
            conn->rs++;

        } else if (conn->rs == 2) {
            // Response after second OK
            response = ProtocolUtils::constructProtocol({"PSYNC", "?", "-1"}, true);
            send(clientFD, response.c_str(), response.size(), 0);
            PRINT_SUCCESS("Hand shake stablished: " + std::to_string(clientFD));
            conn->rs++;
        } else if (conn->rs == 3) {
            // Response for receiving the file
            response = ProtocolUtils::constructProtocol({"REPLCONF", "ACK", "0"}, true);
            send(clientFD, response.c_str(), response.size(), 0);
            conn->replicaHand = false;
            PRINT_SUCCESS("File received");
        }
    }

    void handleResponse(RemusConn::ConnectionManager* conn, std::string buffer, int clientFD) {

        if (conn->replicaHand) {
            replicaHandShake(conn, buffer, clientFD);
            return;
        }

        conn->getProtocolIdr()->identifyProtocol(buffer);
        ProtocolUtils::ReturnObject* rObject = conn->getProtocolIdr()->getRObject();
        send(clientFD, rObject->return_value.c_str(), rObject->bytes, rObject->behavior);

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

    void handle_connection(RemusConn::ConnectionManager* conn, int clientFD) {
        // std::string buffer;

        char buffer[1024];

        size_t bytesReceived {};

        while (true) {
            bytesReceived = recv(clientFD, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived <= 0) {
                PRINT_ERROR("ME CAGO EN TUS MUERTOS");
                break;
            }
            buffer[bytesReceived] = '\0';
            PRINT_SUCCESS("Current client: " + std::to_string(clientFD) + " | nBytes: " + std::to_string(bytesReceived));

            // buffer += buffer;
            std::thread(handleResponse, conn, buffer, clientFD).detach();
            bytesReceived = 0;

        }
        close(clientFD);
    }

    void listener(RemusConn::ConnectionManager* conn, int serverFD) {
        return;
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