#include <serverConn/server_connection.h>
#include <serverConn/master.h>
#include <serverConn/slave.h>

#include <db/utils.h>
#include <db/db_manager.h>

#include <protocol/identifier.h>
#include <protocol/utils.h>

namespace ProtocolID {

    ProtocolIdentifier::ProtocolIdentifier(RemusConn::ConnectionManager *conn) :
    conn {conn},
    protocol {},
    cleaned_buffer {},
    checkMethods {
        &ProtocolIdentifier::identifyPing,
        &ProtocolIdentifier::identifyEcho,
        &ProtocolIdentifier::identifySet,
        &ProtocolIdentifier::identifyConfig,
        &ProtocolIdentifier::identifyGet,
        &ProtocolIdentifier::identifyGetNoDB,
        &ProtocolIdentifier::identifyKeys,
        &ProtocolIdentifier::identifyInfo,
        &ProtocolIdentifier::identifyReplConfi,
        &ProtocolIdentifier::identifyPsync
    },
    rObject {new ProtocolUtils::ReturnObject("+\r\n", 0)}
    {}

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

    void ProtocolIdentifier::cleanResponseObject() {
        rObject = new ProtocolUtils::ReturnObject("+\r\n", 0);
    }

    bool ProtocolIdentifier::identifyProtocol(const std::string& buffer, bool clearRObject) {

        if (clearRObject) cleanResponseObject();
        std::regex non_printable("[^\\x20-\\x7E]+");

        // Convert both strings to lowercase
        this->buffer = buffer;
        std::string buffer_data = buffer;

        std::transform(buffer_data.begin(), buffer_data.end(), buffer_data.begin(), ::tolower);

        // Replace non-printable characters with an empty string
        cleaned_buffer = std::regex_replace(buffer_data, non_printable, "");
        for (auto method : checkMethods) {
            if ((this->*method)()) return true;
        }

        PRINT_ERROR("Protocol could not be identified: " + cleaned_buffer);
        return false;

    }

    size_t ProtocolIdentifier::searchProtocol(std::string search_word) {

        size_t index {};
        index = cleaned_buffer.find(search_word);

        if (index != std::string::npos) protocol = search_word;
        return index;

    }

    std::string ProtocolIdentifier::getVariable(
        size_t starts_at, bool cleanFrontDigits, signed short avoidNChars, char listenOnSymbol, char endsOnSymbol
    ) {
        // This should be better implemeted
        // check the cleanedBuffer
        std::regex not_digit("[^0-9]");
        bool listen = false;

        std::string variable {};

        for (size_t i = starts_at; i < cleaned_buffer.size(); i++) {
            char current = cleaned_buffer[i];

            if (!listen && current == listenOnSymbol) {
                listen = true;
                if (listenOnSymbol == '$') continue;
            }

            if (listen && cleanFrontDigits) {
                if (!std::regex_match(std::string(1, current), not_digit)) continue;
                cleanFrontDigits = false;
            }

            if (listen && avoidNChars > 0) {
                avoidNChars--;
                continue;
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

        if (conn->getRole() == "slave") return true;

        RemusConn::Master* masterConn = static_cast<RemusConn::Master*>(conn);
        masterConn->propageProtocolToReplica(buffer);

        return true;
    }

    bool ProtocolIdentifier::identifyGet() {

        // Check if we get a db file.
        if (!conn->getDirName().size()) return false;

        size_t index = searchProtocol("get");
        if (index == std::string::npos) return false;

        PRINT_WARNING(cleaned_buffer);

        index += 3; // adding get
        std::string key = getVariable(index);

        RemusDBUtils::DatabaseBlock* db = conn->getDbManager()->getDB();

        if (db->keyValue[key].expired) {
            PRINT_SUCCESS("This shit expired");
            rObject = new ProtocolUtils::ReturnObject("$-1\r\n", 0);
        } else {
            std::string response = ProtocolUtils::constructProtocol({db->keyValue[key].value}, false);
            rObject = new ProtocolUtils::ReturnObject(response, 0);

        }

        return true;
    }

    bool ProtocolIdentifier::identifyGetNoDB() {

        // Check if we have a db file.
        if (conn->getDirName().size() > 0) return false;

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
            {"dir", conn->getDirName()}, true);

        } else if (var == "dbfilename") {
            response = ProtocolUtils::constructProtocol(
            {"dbfilename", conn->getFileName()}, true);
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

        RemusDBUtils::DatabaseBlock* db = conn->getDbManager()->getDB();

        // returning all the keys
        std::map<std::string, RemusDBUtils::InfoBlock>& keys = db->keyValue;
        for (std::map<std::string, RemusDBUtils::InfoBlock>::iterator it = keys.begin(); it != keys.end(); ++it) {
            dbKeys.push_back(it->first);
        }

        std::string response = ProtocolUtils::constructProtocol(dbKeys, true);
        rObject = new ProtocolUtils::ReturnObject(response, 0);

        return true;
    }

    bool ProtocolIdentifier::identifyInfo() {

        size_t index = searchProtocol("info");
        if (index == std::string::npos) return false;

        std::string response = ProtocolUtils::constructProtocol(
        {"role:" + conn->getRole(), "master_repl_offset:0", "master_replid:8371b4fb1155b71f4a04d3e1bc3e18c4a990aeeb"},
        false, true);
        rObject = new ProtocolUtils::ReturnObject(response, 0);
        return true;
    }

    bool ProtocolIdentifier::identifyReplConfi() {
        size_t index = searchProtocol("replconf");
        if (index == std::string::npos) return false;

        RemusConn::Master* masterConn = static_cast<RemusConn::Master*>(conn);

        if (!masterConn->inHandShakeWithReplica) {
            masterConn->inHandShakeWithReplica = true;
            masterConn->createCurrentReplicaConn();
            // the first replConf is the port
            std::string port = getVariable(index + 14, false, 1);
            masterConn->setCurrentReplicaPort(std::stoi(port));
        }

        std::string response = ProtocolUtils::constructProtocol({"OK"}, false);
        rObject = new ProtocolUtils::ReturnObject(response, 0);
        return true;

    }

    bool ProtocolIdentifier::identifyPsync() {
        size_t index = searchProtocol("psync");
        if (index == std::string::npos) return false;

        rObject = new ProtocolUtils::ReturnObject("+FULLRESYNC "  + conn->getId() + " 0\r\n", 0);
        conn->sendDBFile = true;
        return true;
    }

}