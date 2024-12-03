#include <serverConn/cache/cache.h>

namespace Cache {

    std::unordered_map<std::string, CacheValue> DataManager::cache;

    DataManager::DataManager() {}
    DataManager::~DataManager() {}

    void DataManager::setValue(
        std::string key,
        std::string value,
        bool expires,
        size_t expiresIn
    ) {

        if (expires) {
            auto now = std::chrono::system_clock::now();
            cache[key] = {value, now + std::chrono::milliseconds(expiresIn)};
        }
        else {
            cache[key] = {value, std::nullopt};
        }
    }

    std::optional<std::string> DataManager::getValue(std::string key) {
        if (!cache.count(key)) {
            return std::nullopt;
        }

        const CacheValue& cachedValue = cache[key];

        if (cachedValue.expired_date.has_value()) {
            auto now = std::chrono::system_clock::now();

            if (now > cachedValue.expired_date.value()) {
                return std::nullopt;
            }

        }

        return cachedValue.value;

    }

}