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

#include <serverConn/connection/structs.h>
#include <serverConn/connection/base.h>
#include <serverConn/connection/master.h>
#include <serverConn/connection/slave.h>

#include <protocol/identifier.h>
#include <protocol/utils.h>

#include <serverConn/manager/initializers.h>
#include <serverConn/parser/parser.h>

#include <utils.h>


namespace ConnManager {
    std::mutex qMutex;
    std::queue<RomulusParser::ParseCommand> commandQueue;

    void handleResponse(
        RomulusConn::BaseConnection* conn,
        std::string rawBuffer,
        RomulusParser::ParseCommand command,
        int clientFD
    ) {

        if (conn->replicaHand) {
            ConnInitializer::replicaHandShake(
                static_cast<RomulusConn::Slave*>(conn),
                command.command,
                clientFD
            );
            return;
        }

        // PRINT_HIGHLIGHT("CLIENT_FD: " + std::to_string(clientFD) + " | COMMAND: " + command.command);
        conn->getProtocolIdr()->processProtocol(clientFD, rawBuffer, command.command, command.size);
    }

    void responseRouter(const char* buffer, size_t size, RomulusConn::BaseConnection* conn, int clientFD) {

        RomulusParser::ParseCommand command {};

        for (unsigned short i {}; i < size; i++) {

            unsigned char byte = static_cast<unsigned char>(buffer[i]);

            if (byte == ProtocolTypes::ARRAY) {
                command = RomulusParser::parserArray(++i, buffer);
            }
            else if (byte == ProtocolTypes::SSTRING || byte == ProtocolTypes::BSTRING) {
                command = RomulusParser::parserString(++i, buffer, size);
            }
            else {
                continue;
            }

            if (command.isEmpty()) return;

            std::lock_guard<std::mutex> lock(qMutex);
            commandQueue.push(command);
            command = {};
        }

        // Launch a thread to process the queue
        std::thread([conn, clientFD, buffer]() {
            while (!commandQueue.empty()) {
                RomulusParser::ParseCommand command;
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

    void handleConnection(RomulusConn::BaseConnection* conn, int clientFD) {
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

            // RomulusUtils::printMixedBytes(buffer, bytesReceived);
            responseRouter(buffer, bytesReceived, conn, clientFD);
        }
        close(clientFD);
    }

    void waitForHandShake(RomulusConn::Slave* conn) {

        while (true) {
            // Wait for 50 milliseconds
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if (conn->getHandShakedWithMaster()) break;
        }
    }

    void listener(RomulusConn::BaseConnection* conn, int serverFD) {

        if (conn->getRole() == RomulusConn::SLAVE) {
            waitForHandShake(static_cast<RomulusConn::Slave*>(conn));
        }

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
        std::vector<RomulusConn::BaseConnection*> connections = ConnInitializer::initializeConnections(argc, argv);

        for (RomulusConn::BaseConnection* conn : connections) {
            connThreads.emplace_back(std::thread(listener, conn, conn->getServerFD()));
        }

        while (true) {}
    }
}