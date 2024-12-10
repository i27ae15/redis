#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include <chrono>
#include <limits>

/**
 * @namespace Cache
 * @brief Contains classes and utilities for managing an in-memory cache system.
 */
namespace Cache {

    // Cache type identifiers
    constexpr unsigned short STR_CACHE = 0; ///< Identifier for the string cache.
    constexpr unsigned short INT_CACHE = 1; ///< Identifier for the integer cache.

    constexpr const char* STRING = "string";
    constexpr const char* LIST = "list";
    constexpr const char* SET = "set";
    constexpr const char* ZSET = "zset";
    constexpr const char* HASH = "hash";
    constexpr const char* STREAM = "stream";
    constexpr const char* NONE = "none";

    // STRING, LIST, SET, ZSET, HASH, STREAM or NONE

    /**
     * @struct strCacheValue
     * @brief Represents a cached value of type string, along with its expiration metadata.
     */
    struct strCacheValue {
        std::string value; ///< The cached string value.
        std::optional<std::chrono::time_point<std::chrono::system_clock>> expiredDate; ///< Expiration date of the cached value, if applicable.
    };

    /**
     * @struct intCacheValue
     * @brief Represents a cached value of type integer, along with its expiration metadata.
     */
    struct intCacheValue {
        int value; ///< The cached integer value.
        std::optional<std::chrono::time_point<std::chrono::system_clock>> expiredDate; ///< Expiration date of the cached value, if applicable.
    };

    /**
     * @class DataManager
     * @brief Manages an in-memory cache for storing string and integer values.
     */
    class DataManager {

    public:

        DataManager();
        ~DataManager();

        /**
         * @brief Saves a value to the appropriate cache (string or integer) based on its type.
         *
         * @param key The key under which the value will be cached.
         * @param value The value to cache.
         * @param expires Indicates whether the value has an expiration time.
         * @param expiresIn The expiration duration in milliseconds (default: 0).
         */
        void saveValue(
            std::string key,
            std::string value,
            bool expires,
            size_t expiresIn = 0
        );

        /**
         * @brief Saves a value as a string in the string cache.
         *
         * @param key The key under which the value will be cached.
         * @param value The value to cache as a string.
         * @param expires Indicates whether the value has an expiration time.
         * @param expiresIn The expiration duration in milliseconds (default: 0).
         */
        void saveValueAsStr(
            std::string key,
            std::string value,
            bool expires,
            size_t expiresIn = 0
        );

        /**
         * @brief Saves a value as an integer in the integer cache.
         *
         * @param key The key under which the value will be cached.
         * @param value The value to cache as an integer.
         * @param expires Indicates whether the value has an expiration time.
         * @param expiresIn The expiration duration in milliseconds (default: 0).
         */
        void saveValueAsInt(
            std::string key,
            std::string value,
            bool expires,
            size_t expiresIn = 0
        );

        /**
         * @brief Saves a value to the specified cache (string or integer).
         *
         * @param key The key under which the value will be cached.
         * @param value The value to cache.
         * @param expires Indicates whether the value has an expiration time.
         * @param expiresIn The expiration duration in milliseconds (default: 0).
         * @param cacheToUse Indicates which cache to use (STR_CACHE or INT_CACHE, default: STR_CACHE).
         */
        void setValue(
            std::string key,
            std::string value,
            bool expires,
            size_t expiresIn = 0,
            unsigned short cacheToUse = STR_CACHE
        );

        /**
         * @brief Increments the value associated with the given key if it exists in the integer cache.
         * If not, it will create they with a initialized value of 1. If the value if on the strCache,
         * It won't be able to be incremented, so false will be returned.
         *
         * @param key The key of the value to increment.
         * @return True if the value was incremented successfully; otherwise, false.
         */
        bool incrementValue(std::string key);

        /**
         * @brief Retrieves a value from the cache by key (Could be strCache or intCache), if it exists
         * and the value has not expired.
         *
         * @param key The key of the cached value.
         * @return An optional containing the cached value if it exists and is valid; otherwise, std::nullopt.
         */
        std::optional<std::string> getValue(std::string key);

        /**
         * @brief Return the type of value the key is storing.
         *
         * @param key The key of the cached value.
         * @return A string of want of the followings: STRING, LIST, SET, ZSET, HASH, STREAM or NONE.
         * NONE, will be returned if this key does exist.
         */
        std::string getKeyType(std::string key);

    private:
        /**
         * @brief Determines whether a given string can be converted to an integer.
         *
         * @param str The string to check.
         * @return True if the string can be converted to an integer; otherwise, false.
         */
        bool canConvertToInt(const std::string& str);

        /**
         * @brief Checks if a cached value has expired.
         *
         * @param dt The expiration date of the cached value.
         * @return True if the value has expired; otherwise, false.
         */
        bool hasExpired(std::optional<std::chrono::time_point<std::chrono::system_clock>> dt);

        /**
         * @brief Checks a string cache value for validity and retrieves it if it has not expired.
         *
         * @param value The string cache value to check.
         * @return An optional containing the cached value if it is valid; otherwise, std::nullopt.
         */
        std::optional<std::string> checkCache(strCacheValue value);

        /**
         * @brief Checks an integer cache value for validity and retrieves it if it has not expired.
         *
         * @param value The integer cache value to check.
         * @return An optional containing the cached value as a string if it is valid; otherwise, std::nullopt.
         */
        std::optional<std::string> checkCache(intCacheValue value);

        // Static caches
        static std::unordered_map<std::string, strCacheValue> strCache; ///< Cache for string values.
        static std::unordered_map<std::string, intCacheValue> intCache; ///< Cache for integer values.
    };
}
