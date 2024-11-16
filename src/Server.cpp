#include <server_connection.h>

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  ServerConnection::ConnectionManager conn;
  if (!conn.get_connection_status()) return 1;

  int server_fd = conn.get_server_fd();
  ServerConnection::listener(server_fd);

  return 0;
}
