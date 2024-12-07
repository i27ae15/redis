#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include <chrono>


namespace Cache {

    constexpr unsigned short STR_CACHE = 0;
    constexpr unsigned short INT_CACHE = 1;

    struct strCacheValue {
        std::string value;
        std::optional<std::chrono::time_point<std::chrono::system_clock>> expiredDate;
    };

    struct intCacheValue {
        int value;
        std::optional<std::chrono::time_point<std::chrono::system_clock>> expiredDate;
    };

    class DataManager {

        public:

        DataManager();
        ~DataManager();

        void saveValue(
            std::string key,
            std::string value,
            bool expires,
            size_t expiresIn = 0
        );

        void saveValueAsStr(
            std::string key,
            std::string value,
            bool expires,
            size_t expiresIn = 0
        );

        void saveValueAsInt(
            std::string key,
            std::string value,
            bool expires,
            size_t expiresIn = 0
        );

        void setValue(
            std::string key,
            std::string value,
            bool expires,
            size_t expiresIn = 0,
            unsigned short cacheToUse = STR_CACHE
        );

        bool incrementValue(std::string key);

        std::optional<std::string> getValue(std::string key);

        private:

        bool canConvertToInt(const std::string& str);

        bool hasExpired (std::optional<std::chrono::time_point<std::chrono::system_clock>> dt);
        std::optional<std::string> checkCache(strCacheValue value);
        std::optional<std::string> checkCache(intCacheValue value);

        // Implement a better cache to accept int, float, bool
        // among more complex data structures like vectors and maps
        static std::unordered_map<std::string, strCacheValue> strCache;
        static std::unordered_map<std::string, intCacheValue> intCache;
    };

}