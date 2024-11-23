#include <serverConn/helpers.h>
#include <serverConn/server_connection.h>

#include <db/db_manager.h>

#include <protocol/identifier.h>

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  std::vector<RemusConnHelper::connConf> connConfigs = RemusConnHelper::configInitializer(argc, argv);
  std::vector<RemusConn::ConnectionManager*> conns {};

  for (RemusConnHelper::connConf connConf : connConfigs) {
    RemusConn::ConnectionManager* newConn = new RemusConn::ConnectionManager(
      connConf.port, connConf.dirName, connConf.fileName, connConf.role
    );

    if (!newConn->getConnectionStatus()) return 1;

    // We need to create a protocol id and a dbManager
    newConn->setDbManager(new RemusDB::DbManager(newConn));
    newConn->setProtocolIdr(new ProtocolID::ProtocolIdentifier(newConn));

    RemusConnHelper::listener(newConn);
    conns.push_back(newConn);
  }

  return 0;
}
