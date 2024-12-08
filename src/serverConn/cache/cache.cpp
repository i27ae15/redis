#include <sstream>

#include <utils.h>
#include <serverConn/cache/cache.h>

namespace Cache {

    std::unordered_map<std::string, strCacheValue> DataManager::strCache;
    std::unordered_map<std::string, intCacheValue> DataManager::intCache;

    DataManager::DataManager() {}
    DataManager::~DataManager() {}

    bool DataManager::canConvertToInt(const std::string& str) {
        std::istringstream iss(str);
        int num;
        // Try to read an integer and ensure no leftover characters
        return (iss >> num) && (iss.eof());
    }

    bool DataManager::incrementValue(std::string key) {
        if (strCache.count(key)) return false;

        if (!intCache.count(key)) {
            saveValueAsInt(key, "1", false);
        } else {
            intCache[key].value++;
        }

        return true;
    }

    void DataManager::saveValue(
        std::string key,
        std::string value,
        bool expires,
        size_t expiresIn
    ) {

        if (canConvertToInt(value)) return saveValueAsInt(key, value, expires, expiresIn);
        saveValueAsStr(key, value, expires, expiresIn);
    }

    void DataManager::saveValueAsInt(
        std::string key,
        std::string value,
        bool expires,
        size_t expiresIn
    ) {
        setValue(key, value, expires, expiresIn, INT_CACHE);
    }

    void DataManager::saveValueAsStr(
        std::string key,
        std::string value,
        bool expires,
        size_t expiresIn
    ) {
        setValue(key, value, expires, expiresIn, STR_CACHE);
    }

    void DataManager::setValue(
        std::string key,
        std::string value,
        bool expires,
        size_t expiresIn,
        unsigned short cacheToUse
    ) {

        std::optional<std::chrono::time_point<std::chrono::system_clock>> expireDate {};

        if (expires) {
            auto now = std::chrono::system_clock::now();
            expireDate = now + std::chrono::milliseconds(expiresIn);
        }

        switch (cacheToUse) {
            case INT_CACHE:
                intCache[key] = {std::stoi(value), expireDate};
                break;

            case STR_CACHE:
                strCache[key] = {value, expireDate};
                break;

            default:
                break;
        }

        // PRINT_HIGHLIGHT("VALUE SET FOR: " + key);
    }

    bool DataManager::hasExpired(
        std::optional<std::chrono::time_point<std::chrono::system_clock>> dt
    ) {
        if (dt.has_value()) {
            auto now = std::chrono::system_clock::now();

            if (now > dt.value()) {
                return true;
            }
        }

        return false;
    }

    std::optional<std::string> DataManager::checkCache(strCacheValue cachedValue) {
        if (hasExpired(cachedValue.expiredDate)) return std::nullopt;

        return cachedValue.value;
    }

    std::optional<std::string> DataManager::checkCache(intCacheValue cachedValue) {
        if (hasExpired(cachedValue.expiredDate)) return std::nullopt;

        return std::to_string(cachedValue.value);
    }

    std::optional<std::string> DataManager::getValue(std::string key) {

        if (strCache.count(key)) return checkCache(strCache[key]);
        if (intCache.count(key)) return checkCache(intCache[key]);

        return std::nullopt;
    }

}