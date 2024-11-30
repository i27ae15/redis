#include <serverConn/server_connection.h>
#include <serverConn/parser.h>

namespace ConnManager {

    bool replicaHandShake(RemusConn::ConnectionManager* conn, std::string buffer, int clientFD);
    void handleConnection(RemusConn::ConnectionManager* conn, int clientFD);
    void handleResponse(RemusConn::ConnectionManager* conn, std::string rawBuffer, RemusParser::ParseCommand command, int clientFD);
    void listener(RemusConn::ConnectionManager* conn, int serverFD);
    void serverLoop(int argc, char **argv);
    void responseRouter(const char* buffer, size_t size, RemusConn::ConnectionManager* conn, int clientFD);
}