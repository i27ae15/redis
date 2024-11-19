#include <db/db_manager.h>

namespace RemusDB {

    DbManager::DbManager() {
        dirName = Remus::ConfigManager::getInstance().getDirName();
        fileName = Remus::ConfigManager::getInstance().getFileName();
        openFile();
        parseHeader();
    }

    DbManager::~DbManager() {
        if (file.is_open()) file.close();
    }

    std::string DbManager::getDirName() {return dirName;}
    std::string DbManager::getFileName() {return fileName;}

    void DbManager::openFile() {
        std::string filePath = getDirName() + "/" + getFileName();
        file.open(filePath, std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open the file");
        }

    }

    uint8_t DbManager::readByte() {
        char byte;
        if (!file.read(&byte, 1)) {
            throw std::runtime_error("Failed to read byte from file");
        }

        // std::cout << "Read byte: " << std::hex << static_cast<int>(static_cast<uint8_t>(byte)) << std::endl;
        return static_cast<uint8_t>(byte);
    }

    std::string DbManager::readString() {
        uint8_t sizeByte = readByte();
        size_t length = 0;

        if ((sizeByte >> 6) == 0b00) {
            // 6-bit length
            length = sizeByte & 0x3F;
        } else if ((sizeByte >> 6) == 0b01) {
            // 14-bit length
            length = ((sizeByte & 0x3F) << 8) | readByte();
        } else if ((sizeByte >> 6) == 0b10) {
            // 32-bit length
            length = readLittleEndian(4);
        } else if ((sizeByte >> 6) == 0b11) {
            // Special encoding
            uint8_t encodingType = sizeByte & 0x3F;
            if (encodingType == 0x00) {
                return "";
            } else if (encodingType == 0xC0) {
                // 8-bit integer
                return std::to_string(static_cast<int>(readByte()));
            } else if (encodingType == 0xC1) {
                // 16-bit integer
                return std::to_string(static_cast<int>(readLittleEndian(2)));
            } else if (encodingType == 0xC2) {
                // 32-bit integer
                return std::to_string(static_cast<int>(readLittleEndian(4)));
            } else {
                throw std::runtime_error("Unsupported string encoding: " + std::to_string(encodingType));
            }
        } else {
            throw std::runtime_error("Invalid string encoding");
        }

        // Read the raw string
        std::string result(length, '\0');
        if (!file.read(&result[0], length)) {
            throw std::runtime_error("Failed to read string");
        }
        return result;

    }

    uint64_t DbManager::readLittleEndian(uint8_t size) {
        uint64_t result = 0;
        for (uint8_t i {}; i < size; i++) {
            result |= static_cast<uint64_t>(readByte()) << (i * 8);
        }

        return result;
    }

    // Parsers

    void DbManager::parseHeader() {
        std::string header(9, '\0');

        PRINT_SUCCESS("Header" + header);

        if (!file.read(&header[0], 9)) {
            throw std::runtime_error("Failed to read header");
        }

        if (header.substr(0, 5) != "REDIS") {
            throw std::runtime_error("Invalid redis file");
        }

        PRINT_SUCCESS("Redis version: " + header.substr(5, 4));
    }


    RemusDB::DatabaseBlock DbManager::parseDatabase() {
        RemusDB::DatabaseBlock db;

        // Read RemusDB::DatabaseBlock Start Marker
        uint8_t marker = readByte();
        while (marker != 0xFE) {
            if (marker == 0xFA) {
                // Parse and skip metadata subsection
                std::string metadataKey = readString();
                std::string metadataValue = readString();
                std::cout << "Skipping metadata: " << metadataKey << " = " << metadataValue << std::endl;
            }
            marker = readByte();
        }
        db.index = readByte();

        // Read Hash Table Sizes
        if (readByte() != 0xFB) {
            throw std::runtime_error("Invalid hash table size marker");
        }

        size_t totalKeys = readByte();
        size_t expiryKeys = readByte();

        // Read Key-Value Pairs
        for (size_t i = 0; i < totalKeys; ++i) {
            uint8_t valueType = readByte();
            std::string key = readString();
            std::string value = readString();

            PRINT_COLOR(YELLOW, "Key: " + key + " | Value: " + value);

            RemusDB::InfoBlock kv{key, value};

            // Check for expiry
            uint8_t expireType = readByte();
            if (expireType == 0xFC) {
                kv.expireTime = readLittleEndian(8);
                kv.hasExpire = true;
            } else if (expireType == 0xFD) {
                kv.expireTime = readLittleEndian(4);
                kv.hasExpire = true;
            }

            db.keyValue[key] = kv;
        }

        return db;
    }



}