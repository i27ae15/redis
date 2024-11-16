#include "server_connection.h"

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  ServerConnection::ConnectionManager conn;
  if (!conn.get_connection_status()) return 1;

  conn.listener();

  return 0;
}
