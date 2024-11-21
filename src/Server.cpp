#include <server_connection.h>
#include <db/db_manager.h>

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  RemusConfig::ConfigManager::initialize(argc, argv);

  ServerConnection::ConnectionManager conn(&RemusConfig::ConfigManager::getInstance());
  if (!conn.getConnectionStatus()) return 1;

  int server_fd = conn.getServerFD();
  ServerConnection::listener(server_fd);

  return 0;
}
