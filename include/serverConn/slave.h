#pragma once
#include <serverConn/server_connection.h>

namespace RemusConn {

    class Master;

    class Slave : public ConnectionManager {

        public:
            Slave(unsigned short port, std::string host,
             std::string dirName = "", std::string fileName = "");

            void assignMaster(Master* master);
            void assignMaster(unsigned short port, unsigned short serverFD, std::string host);
            void handShakeWithMaster();

            unsigned short getMasterPort();
            unsigned short getMasterServerFD();

            bool isInHandShake();

            std::string getMasterHost();

        private:

            Master* master;

            std::string masterHost;

            bool handShakedWithMaster;

            unsigned short handShakeStep;
            unsigned short masterPort;
            short masterServerFD;

    };

}