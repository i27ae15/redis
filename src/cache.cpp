#include <cache.h>

namespace Cache {

    std::unordered_map<std::string, std::string> DataManager::cache;

    DataManager::DataManager() {}
    DataManager::~DataManager() {}

    void DataManager::setValue(std::string key, std::string value) {
        cache[key] = value;
    }

    std::optional<std::string> DataManager::getValue(std::string key) {
        if (!cache.count(key)) {
            return std::nullopt;
        }
        return cache[key];
    }

}