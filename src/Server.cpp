#include <server_connection.h>

  void listener(int server_fd) {

      PRINT_SUCCESS("Listener Started");

      std::vector<std::thread> threads {};
      while (true) {
          struct sockaddr_in client_addr {};
          int client_addr_len = sizeof(client_addr);
          int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);

          threads.emplace_back(std::thread(handle_connection, client_fd)).detach();
      }
      close(server_fd);
  }

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  ServerConnection::ConnectionManager conn;
  if (!conn.get_connection_status()) return 1;

  int server_fd = conn.get_server_fd();
  listener(server_fd);

  return 0;
}
