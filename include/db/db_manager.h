#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <map>

#include <utils.h>
#include <db/structs.h>

namespace RomulusConn {
    class BaseConnection;
}

namespace RomulusDB {

    class DbManager {

        public:

        DbManager(RomulusConn::BaseConnection* conn);
        ~DbManager();
        void parseHeader();

        RomulusDbStructs::DatabaseBlock* getDB();

        std::string getDirName();
        std::string getFileName();
        std::string getDbFile();

        private:

        RomulusDbStructs::DatabaseBlock* db;
        void parseDatabase();

        RomulusConn::BaseConnection* conn;

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