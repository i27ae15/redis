#pragma once
#include <cstdint>
#include <string>
#include <map>

namespace RomulusDbStructs {

    struct InfoBlock {
        std::string key;
        std::string value;
        uint64_t expireTime = 0;
        bool hasExpire = false;
        bool expireTimeInMs = false;
        bool expired = false;
    };

    struct DatabaseBlock {
        uint8_t index;
        std::map<std::string, InfoBlock> keyValue;
    };
}