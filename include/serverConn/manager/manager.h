#include <serverConn/connection/base.h>
#include <serverConn/parser/parser.h>

namespace ConnManager {

    void serverLoop(int argc, char **argv);
    void listener(RomulusConn::BaseConnection* conn, int serverFD);
    void handleConnection(RomulusConn::BaseConnection* conn, int clientFD);
    void responseRouter(
        const char* buffer,
        size_t size,
        RomulusConn::BaseConnection* conn,
        int clientFD
    );
    void handleResponse(
        RomulusConn::BaseConnection* conn,
        std::string rawBuffer,
        RomulusParser::ParseCommand command,
        int clientFD
    );

}