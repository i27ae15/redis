#include <serverConn/server_connection.h>

namespace ConnManager {

    void handle_connection(RemusConn::ConnectionManager* conn, int clientFD);
    void listener(RemusConn::ConnectionManager* conn, int serverFD);
    void serverLoop(int argc, char **argv);
}