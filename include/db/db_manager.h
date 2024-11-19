#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <map>

#include <config_manager.h>
#include <utils.h>
#include <db/utils.h>

namespace RemusDB {

    class DbManager {

        public:

        DbManager();
        ~DbManager();
        void parseHeader();
        DatabaseBlock parseDatabase();

        private:

        std::ifstream file;

        std::string dirName;
        std::string fileName;

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