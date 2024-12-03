#pragma once
#include <serverConn/connection/base.h>

namespace RomulusConn {

    class Master;

    class Slave : public BaseConnection {

        public:
            Slave(
                unsigned short port,
                std::string host,
                std::string dirName = "",
                std::string fileName = ""
            );

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
            unsigned short masterServerFD;
    };

}