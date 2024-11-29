#pragma once
#include <serverConn/server_connection.h>

namespace RemusConn {

    class Master;

    class Slave : public ConnectionManager {

        public:
            Slave(signed short port, std::string host,
             std::string dirName = "", std::string fileName = "");

            void assignMaster(Master* master);
            void assignMaster(signed short port, signed short serverFD, std::string host);
            void handShakeWithMaster();

            signed short getMasterPort();
            signed short getMasterServerFD();

            bool isInHandShake();

            std::string getMasterHost();

        private:

            Master* master;

            std::string masterHost;

            bool handShakedWithMaster;

            signed short handShakeStep;
            signed short masterPort;
            short masterServerFD;

    };

}