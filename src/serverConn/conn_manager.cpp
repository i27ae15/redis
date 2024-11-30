#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <ctype.h>
#include <stdexcept>
#include <queue>
#include <mutex>

#include <serverConn/structs.h>
#include <serverConn/server_connection.h>
#include <serverConn/master.h>

#include <protocol/identifier.h>
#include <protocol/utils.h>

#include <serverConn/initializers.h>
#include <serverConn/slave.h>
#include <serverConn/parser.h>

#include <utils.h>


namespace ConnManager {
    std::mutex qMutex;
    std::queue<RemusParser::ParseCommand> commandQueue;

    bool replicaHandShake(RemusConn::ConnectionManager* conn, std::string buffer, int clientFD) {
        std::string response {};

        if (conn->rs == 0) {
            // Response after pong
            if (buffer.substr(0, 4) != ProtocolID::PONG) return false;
            response = ProtocolUtils::constructProtocol({"REPLCONF", "listening-port", std::to_string(conn->getPort())}, ProtocolTypes::ResponseType::ARRAY);
            send(clientFD, response.c_str(), response.size(), 0);
            conn->rs++;

        } else if (conn->rs == 1) {
            // Response after first OK
            response = ProtocolUtils::constructProtocol({"REPLCONF", "capa", "psync2"}, ProtocolTypes::ResponseType::ARRAY);
            send(clientFD, response.c_str(), response.size(), 0);
            conn->rs++;

        } else if (conn->rs == 2) {
            // Response after second OK
            conn->rs++;
            conn->replicaHand = false;
            response = ProtocolUtils::constructProtocol({"PSYNC", "?", "-1"}, ProtocolTypes::ResponseType::ARRAY);
            send(clientFD, response.c_str(), response.size(), 0);
            PRINT_SUCCESS("Hand shake stablished: " + std::to_string(clientFD));
        }

        return true;
    }

    void handleResponse(RemusConn::ConnectionManager* conn, std::string rawBuffer, RemusParser::ParseCommand command, int clientFD) {

        if (conn->replicaHand) {
            if (replicaHandShake(conn, command.command, clientFD)) return;
        }

        conn->getProtocolIdr()->identifyProtocol(rawBuffer, command.command, command.size);
        ProtocolUtils::ReturnObject* rObject = conn->getProtocolIdr()->getRObject();

        if (rObject->sendResponse) send(clientFD, rObject->return_value.c_str(), rObject->bytes, rObject->behavior);

        if (conn->sendDBFile) {
            unsigned short fileLength = conn->getDbFile().size();
            rObject = new ProtocolUtils::ReturnObject("$" + std::to_string(fileLength) + "\r\n" + conn->getDbFile(), 0);
            send(clientFD, rObject->return_value.c_str(), rObject->bytes, rObject->behavior);
            conn->sendDBFile = false;

            RemusConn::Master* masterConn = static_cast<RemusConn::Master*>(conn);
            masterConn->setCurrentReplicaServerFd(clientFD);
            masterConn->addAndCleanCurrentReplicaConn();
            masterConn->inHandShakeWithReplica = false;
        }
    }

    void responseRouter(const char* buffer, size_t size, RemusConn::ConnectionManager* conn, int clientFD) {

        RemusParser::ParseCommand command {};

        for (unsigned short i {}; i < size; i++) {

            unsigned char byte = static_cast<unsigned char>(buffer[i]);

            if (byte == ProtocolTypes::ARRAY) {
                command = RemusParser::parserArray(++i, buffer);
            }
            else if (byte == ProtocolTypes::SSTRING) {
                command = RemusParser::parserString(++i, buffer, size);
            } else {
                continue;
            }

            // TODO: Add possible to listen on $
            std::lock_guard<std::mutex> lock(qMutex);
            commandQueue.push(command);
        }

        // Launch a thread to process the queue
        std::thread([conn, clientFD, buffer]() {
            while (!commandQueue.empty()) {
                RemusParser::ParseCommand command;
                {
                    std::lock_guard<std::mutex> lock(qMutex);
                    if (!commandQueue.empty()) {
                        command = commandQueue.front();
                        commandQueue.pop();
                    }
                }
                handleResponse(conn, buffer, command, clientFD);
            }
        }).detach();
    }

    void handleConnection(RemusConn::ConnectionManager* conn, int clientFD) {
        constexpr size_t BUFFER_SIZE = 1048;

        char buffer[BUFFER_SIZE];
        size_t bytesReceived {};

        std::string parsedBuffer {};

        while (true) {
            bytesReceived = recv(clientFD, buffer, BUFFER_SIZE, 0);
            if (bytesReceived <= 0) {
                PRINT_ERROR("Breaking connection");
                break;
            }

            if (bytesReceived < BUFFER_SIZE) buffer[bytesReceived] = '\0';

            // RemusUtils::printMixedBytes(buffer, bytesReceived);
            responseRouter(buffer, bytesReceived, conn, clientFD);
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

            threads.emplace_back(std::thread(handleConnection, conn, clientFD)).detach();
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