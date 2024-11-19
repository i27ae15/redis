#include <protocol/identifier.h>

namespace ProtocolID {

    ProtocolIdentifier::ProtocolIdentifier(std::string buffer) :
    protocol {},
    cleaned_buffer {},
    checkMethods {
        &ProtocolIdentifier::identifyPing,
        &ProtocolIdentifier::identifyEcho,
        &ProtocolIdentifier::identifySet,
        &ProtocolIdentifier::identifyConfig,
        &ProtocolIdentifier::identifyGet,
        &ProtocolIdentifier::identifyGetNoDB,
        &ProtocolIdentifier::identifyKeys
    },
    buffer{buffer}
    {
        rObject = new ProtocolUtils::ReturnObject("+\r\n", 0);
        identifyProtocol();

        if (!protocol.size()) PRINT_ERROR("Protocol not idenfied");
    }

    ProtocolIdentifier::~ProtocolIdentifier() {
        delete rObject;
    }

    // GETTERS

    ProtocolUtils::ReturnObject* ProtocolIdentifier::getRObject() {
        return rObject;
    }

    std::string ProtocolIdentifier::getProtocol() {
        return protocol;
    }

    bool ProtocolIdentifier::identifyProtocol() {
        std::regex non_printable("[^\\x20-\\x7E]+");

        // Convert both strings to lowercase
        std::string buffer_data = buffer;

        std::transform(buffer_data.begin(), buffer_data.end(), buffer_data.begin(), ::tolower);

        // Replace non-printable characters with an empty string
        cleaned_buffer = std::regex_replace(buffer_data, non_printable, "");
        for (auto method : checkMethods) {
            if ((this->*method)()) return true;
        }
        return false;

    }

    size_t ProtocolIdentifier::searchProtocol(std::string search_word) {

        size_t index {};
        index = cleaned_buffer.find(search_word);

        if (index != std::string::npos) protocol = search_word;
        return index;

    }

    std::string ProtocolIdentifier::getVariable(
        size_t starts_at, char listenOnSymbol, char endsOnSymbol
    ) {
        // This should be better implemeted
        // check the cleanedBuffer
        std::regex not_digit("[^0-9]");
        bool listen = false;

        std::string variable {};
        bool cleanFrontDigits = false;

        for (size_t i = starts_at; i < cleaned_buffer.size(); i++) {
            char current = cleaned_buffer[i];

            if (!listen && current == listenOnSymbol) {
                listen = true;
                if (listenOnSymbol == '$') {
                    cleanFrontDigits = true;
                    continue;
                }
            }

            if (listen && cleanFrontDigits) {
                if (!std::regex_match(std::string(1, current), not_digit)) continue;
                cleanFrontDigits = false;
            }

            if (listen && current == endsOnSymbol) break;
            if (listen) variable += current;
        }

        return variable;
    }

    std::pair<bool, size_t> ProtocolIdentifier::getExpireTime() {
        // Example
        // *5$3set$6orange$9blueberry$2px$3100
        size_t index = searchProtocol("px");
        if (index == std::string::npos) return {false, 0};

        index += 4; // px.size + $n.size
        std::string nStr {};
        for (size_t i = index; i < cleaned_buffer.size(); i++) {
            nStr += cleaned_buffer[i];
        }

        return {true, std::stoi(nStr)};
    }

    bool ProtocolIdentifier::identifyPing() {

        size_t index = searchProtocol("ping");
        if (index == std::string::npos) return false;

        rObject = new ProtocolUtils::ReturnObject("+PONG\r\n", 0);
        return true;
    }

    bool ProtocolIdentifier::identifyEcho() {

        size_t index = searchProtocol("echo");
        if (index == std::string::npos) return false;

        std::regex not_digit("[^0-9]");
        index += 4; // Plus echo.size

        std::string pre_echo = getVariable(index);
        std::string echo = "$" + std::to_string(pre_echo.size()) + "\r\n" + pre_echo + "\r\n";
        rObject = new ProtocolUtils::ReturnObject(echo, 0);

        return true;
    }

    bool ProtocolIdentifier::identifySet() {
        // *3$3set$9pineapple$6banana
        size_t index = searchProtocol("set");
        if (index == std::string::npos) return false;

        index += 3; // adding set

        std::string key = getVariable(index);
        std::string value = getVariable(index + key.size()); // Not correct the sum, but will work

        // Check if has px
        std::pair<bool, size_t> expireTime = getExpireTime();

        Cache::DataManager cache;
        cache.setValue(key, value, expireTime.first, expireTime.second);

        rObject = new ProtocolUtils::ReturnObject("+OK\r\n", 0);
        return true;
    }

    bool ProtocolIdentifier::identifyGet() {

        // Check if we get a db file.
        if (!RemusConfig::ConfigManager::getInstance().getDirName().size()) return false;

        size_t index = searchProtocol("get");
        if (index == std::string::npos) return false;

        PRINT_WARNING(cleaned_buffer);

        index += 3; // adding get
        std::string key = getVariable(index);

        RemusDB::DbManager dbManager;
        RemusDB::DatabaseBlock db {};
        db = dbManager.parseDatabase();

        std::string response = ProtocolUtils::constructProtocol({db.keyValue[key].value}, false);
        rObject = new ProtocolUtils::ReturnObject(response, 0);

        return true;
    }

    bool ProtocolIdentifier::identifyGetNoDB() {

        // Check if we have a db file.
        if (RemusConfig::ConfigManager::getInstance().getDirName().size() > 0) return false;

        // For the first stage, legacy for reading from a db file.
        size_t index = searchProtocol("get");
        if (index == std::string::npos) return false;

        index += 3; // adding get
        std::string key = getVariable(index);

        Cache::DataManager cache;
        std::optional<std::string> value = cache.getValue(key);

        if (!value.has_value()) {
            rObject = new ProtocolUtils::ReturnObject("$-1\r\n", 0);
            return false;
        }

        std::string response = ProtocolUtils::constructProtocol({value.value()}, false);
        rObject = new ProtocolUtils::ReturnObject(response, 0);

        return true;
    }

    bool ProtocolIdentifier::identifyConfig() {
        // client: $ redis-cli CONFIG GET dir
        // identifyConfigGet : *3$6config$3get$3dir
        size_t index {};

        index = searchProtocol("config");
        if (index == std::string::npos) return false;

        index = searchProtocol("get");
        if (index == std::string::npos) return false;

        index += 3; // Adding get;
        std::string var = getVariable(index);
        std::string response {};
        if (var == "dir") {
            response = ProtocolUtils::constructProtocol(
            {"dir", RemusConfig::ConfigManager::getInstance().getDirName()}, true);

        } else if (var == "dbfilename") {
            response = ProtocolUtils::constructProtocol(
            {"dbfilename", RemusConfig::ConfigManager::getInstance().getFileName()}, true);
        }

        rObject = new ProtocolUtils::ReturnObject(response, 0);
        return true;
    }

    bool ProtocolIdentifier::identifyKeys() {
        // Example of key
        // *2$4keys$1*

        size_t index = searchProtocol("keys");
        if (index == std::string::npos) return false;

        std::string var = getVariable(index);
        std::vector<std::string> dbKeys {};

        if (var != "*") return false;

        RemusDB::DbManager dbManager;
        RemusDB::DatabaseBlock db {};

        // returning all the keys
        db = dbManager.parseDatabase();
        std::map<std::string, RemusDB::InfoBlock>& keys = db.keyValue;
        for (std::map<std::string, RemusDB::InfoBlock>::iterator it = keys.begin(); it != keys.end(); ++it) {
            dbKeys.push_back(it->first);
        }

        std::string response = ProtocolUtils::constructProtocol(dbKeys, true);
        rObject = new ProtocolUtils::ReturnObject(response, 0);

        return true;
    }

}