#include <serverConn/conn_manager.h>

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  ConnManager::serverLoop(argc, argv);
  return 0;
}
