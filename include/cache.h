#include <unordered_map>
#include <string>
#include <optional>

namespace Cache {

    class DataManager {

        public:

        DataManager();
        ~DataManager();

        void setValue(std::string key, std::string value);
        std::optional<std::string> getValue(std::string key);


        private:

        // Implement a better cache to accpet int, float, bool
        // among more complex data structures like vectors and maps
        static std::unordered_map<std::string, std::string> cache;
    };

}