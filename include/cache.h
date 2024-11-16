#include <unordered_map>
#include <string>
#include <optional>
#include <chrono>


namespace Cache {

    struct CacheValue {
        std::string value;
        std::optional<std::chrono::time_point<std::chrono::system_clock>> expired_date;
    };

    class DataManager {

        public:

        DataManager();
        ~DataManager();

        void setValue(std::string key, std::string value, bool expires, size_t expiresIn = 0);
        std::optional<std::string> getValue(std::string key);

        private:

        // Implement a better cache to accpet int, float, bool
        // among more complex data structures like vectors and maps
        static std::unordered_map<std::string, CacheValue> cache;
    };

}