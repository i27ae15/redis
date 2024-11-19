#pragma once
#include <string>
#include <map>

namespace RemusDB {

    struct InfoBlock {
        std::string key;
        std::string value;
        uint64_t expireTime = 0;
        bool hasExpire = false;
    };

    struct DatabaseBlock {
        uint8_t index;
        std::map<std::string, InfoBlock> keyValue;
    };
}