#include <db/db_manager.h>

namespace RemusDB {

    DbManager::DbManager() {
        dirName = RemusConfig::ConfigManager::getInstance().getDirName();
        fileName = RemusConfig::ConfigManager::getInstance().getFileName();
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
                // 8-bit integer
                int8_t val = static_cast<int8_t>(readByte());
                return std::to_string(val);
            } else if (encodingType == 0x01) {
                // 16-bit integer
                int16_t val = static_cast<int16_t>(readLittleEndian(2));
                return std::to_string(val);
            } else if (encodingType == 0x02) {
                // 32-bit integer
                int32_t val = static_cast<int32_t>(readLittleEndian(4));
                return std::to_string(val);
            } else {
                throw std::runtime_error("Unsupported string encoding: " + std::to_string(encodingType));
            }
        } else {
            throw std::runtime_error("Invalid string encoding");
        }

        // Read the raw string
        if (length > 0) {
            std::string result(length, '\0');
            if (!file.read(&result[0], length)) {
                throw std::runtime_error("Failed to read string");
            }
            return result;
        } else {
            return "";
        }
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

    uint64_t getCurrentUnixTimeInMs() {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    }

    uint64_t getCurrentUnixTimeInSeconds() {
        return getCurrentUnixTimeInMs() / 1000;
    }

    RemusDB::DatabaseBlock DbManager::parseDatabase() {
        RemusDB::DatabaseBlock db;

        // Read Database Start Marker
        uint8_t marker = readByte();
        while (marker != 0xFE) {
            if (marker == 0xFA) {
                // Parse and skip metadata subsection
                std::string metadataKey = readString();
                std::string metadataValue = readString();
                std::cout << "Skipping metadata: " << metadataKey << " = " << metadataValue << std::endl;
            } else {
                throw std::runtime_error("Unexpected marker: " + std::to_string(marker));
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
            RemusDB::InfoBlock kv;

            // Check for optional expire information
            uint8_t peekByte = file.peek();
            if (peekByte == 0xFC || peekByte == 0xFD) {
                uint8_t expireType = readByte();
                kv.hasExpire = true;
                if (expireType == 0xFC) {
                    kv.expireTime = readLittleEndian(8);
                    kv.expireTimeInMs = true;
                } else if (expireType == 0xFD) {
                    kv.expireTime = readLittleEndian(4);
                    kv.expireTimeInMs = false;
                }
            }

            // Determine if the key has expired
            if (kv.hasExpire) {
                uint64_t currentTime;
                if (kv.expireTimeInMs) {
                    currentTime = getCurrentUnixTimeInMs();
                } else {
                    currentTime = getCurrentUnixTimeInSeconds();
                }

                if (kv.expireTime <= currentTime) {
                    kv.expired = true;
                }
            }


            // Read Value Type
            uint8_t valueType = readByte();

            // Read Key and Value
            kv.key = readString();
            kv.value = readString();

            db.keyValue[kv.key] = kv;
        }

        return db;
    }



}