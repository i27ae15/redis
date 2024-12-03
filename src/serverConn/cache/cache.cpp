#include <sstream>

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
        if (!intCache.count(key)) return false;
        intCache[key].value++;
        return true;
    }

    void DataManager::setValue(
        std::string key,
        std::string value,
        bool expires,
        size_t expiresIn
    ) {

        std::optional<std::chrono::time_point<std::chrono::system_clock>> expireDate {};

        if (expires) {
            auto now = std::chrono::system_clock::now();
            expireDate = now + std::chrono::milliseconds(expiresIn);
        }

        // Check value type

        if (canConvertToInt(value)) {
            intCache[key] = {std::stoi(value), expireDate};
        }
        else {
            strCache[key] = {value, expireDate};
        }
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