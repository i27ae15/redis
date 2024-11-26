#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include <utils.h>
#include <db/utils.h>

namespace RemusConn {
    class ConnectionManager;
}

namespace RemusDB {

    class DbManager {

        public:

        DbManager(RemusConn::ConnectionManager* conn);
        ~DbManager();
        void parseHeader();

        RemusDBUtils::DatabaseBlock* getDB();

        std::string getDirName();
        std::string getFileName();
        std::string getDbFile();

        private:

        RemusDBUtils::DatabaseBlock* db;
        void parseDatabase();

        RemusConn::ConnectionManager* conn;

        std::ifstream file;

        // Methods
        uint8_t readByte();
        uint64_t readLittleEndian(uint8_t size);

        std::string readString();

        void get(std::string key);
        void save(std::string key, std::string value);

        void openFile();


    };

}