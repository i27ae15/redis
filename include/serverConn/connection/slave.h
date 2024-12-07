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
            void assignMaster(
                unsigned short port,
                short serverFD,
                std::string host
            );
            void handShakeWithMaster();

            unsigned short getMasterPort();
            short getMasterServerFD();

            bool isInHandShake();

            std::string getMasterHost();
            std::string getRole() const override;

        private:

            Master* master;

            std::string masterHost;

            bool handShakedWithMaster;

            unsigned short handShakeStep;
            unsigned short masterPort;
            short masterServerFD;
    };

}