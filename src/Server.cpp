#include <serverConn/helpers.h>
#include <serverConn/server_connection.h>

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::vector<RemusConn::ConnectionManager*> conns = RemusConnHelper::initializeConnections(argc, argv);

  return 0;
}
