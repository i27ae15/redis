#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <map>

#include <utils.h>
#include <db/utils.h>

#include <server_connection.h>

namespace RemusDB {

    class DbManager {

        public:

        DbManager(RemusConn::ConnectionManager* conn);
        ~DbManager();
        void parseHeader();
        DatabaseBlock parseDatabase();

        private:

        RemusConn::ConnectionManager* conn;

        std::ifstream file;

        // Methods
        uint8_t readByte();
        uint64_t readLittleEndian(uint8_t size);

        std::string readString();

        void get(std::string key);
        void save(std::string key, std::string value);

        void openFile();

        std::string getDirName();
        std::string getFileName();

    };

}