#include <algorithm>

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
    splittedBuffer {},
    rawBuffer {},
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
        &ProtocolIdentifier::identifyPsync,
        &ProtocolIdentifier::identifyFullResync
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

    bool ProtocolIdentifier::getInProcess() {
        return inProcess;
    }

    void ProtocolIdentifier::cleanResponseObject() {
        rObject = new ProtocolUtils::ReturnObject("+\r\n", 0, false);
    }

    // Setters

    void ProtocolIdentifier::setInProcess(bool value) {
        inProcess = value;
    }

    void ProtocolIdentifier::setSplitedBuffer() {
        splittedBuffer = RemusUtils::splitString(buffer, " ");
    }

    bool ProtocolIdentifier::identifyProtocol(const std::string rawBuffer, const std::string buffer, bool clearRObject) {

        setInProcess(true);

        if (clearRObject) cleanResponseObject();

        // Convert both strings to lowercase
        this->buffer = buffer;
        this->rawBuffer = rawBuffer;
        setSplitedBuffer();

        // We should use a map for this
        for (auto method : checkMethods) {
            if ((this->*method)()) {
                setInProcess(false);
                return true;
            }
        }

        conn->print("Protocol could not be identified: " + buffer, RED);
        setInProcess(false);
        return false;

    }

    // DEPRECATED
    size_t ProtocolIdentifier::searchProtocol(std::string search_word) {

        size_t index {};
        index = buffer.find(search_word);

        if (index != std::string::npos) protocol = search_word;
        return index;

    }

    // DEPRECATED
    std::string ProtocolIdentifier::getVariable(
        size_t starts_at, bool cleanFrontDigits, signed short avoidNChars,
        char listenOnSymbol, char endsOnSymbol
    ) {
        // This should be better implemeted
        // check the cleanedBuffer
        std::regex not_digit("[^0-9]");
        bool listen = false;

        std::string variable {};

        for (size_t i = starts_at; i < buffer.size(); i++) {
            char current = buffer[i];

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
        for (size_t i = index; i < buffer.size(); i++) {
            nStr += buffer[i];
        }

        return {true, std::stoi(nStr)};
    }

    bool ProtocolIdentifier::identifyPing() {

        if (splittedBuffer[0] != PING) return false;
        rObject = new ProtocolUtils::ReturnObject("+PONG\r\n", 0);
        return true;
    }

    bool ProtocolIdentifier::identifyEcho() {

        if (splittedBuffer[0] != ECHO) return false;

        std::string &pre_echo = splittedBuffer[1];
        std::string echo = "$" + std::to_string(pre_echo.size()) + "\r\n" + pre_echo + "\r\n";
        rObject = new ProtocolUtils::ReturnObject(echo, 0);

        return true;
    }

    bool ProtocolIdentifier::identifySet() {
        // *3$3set$9pineapple$6banana
        if (splittedBuffer[0] != SET) return false;

        std::string &key = splittedBuffer[1];
        std::string &value = splittedBuffer[2];

        // Check if has px
        std::pair<bool, size_t> expireTime {};

        if (splittedBuffer.size() > 3 && splittedBuffer[3] == "px") {
            expireTime.second = std::stoi(splittedBuffer[4]);
            expireTime.first = true;
        }

        Cache::DataManager cache;
        cache.setValue(key, value, expireTime.first, expireTime.second);

        // PRINT_SUCCESS("VALUE SET FOR KEY: " + key + " : " + value);

        rObject = new ProtocolUtils::ReturnObject("+OK\r\n", 0);
        if (conn->getRole() == RemusConn::SLAVE) return true;

        RemusConn::Master* masterConn = static_cast<RemusConn::Master*>(conn);
        masterConn->propageProtocolToReplica(rawBuffer);

        return true;
    }

    bool ProtocolIdentifier::identifyGet() {

        // Check if we get a db file.
        if (conn->getDirName().size() == 0 || splittedBuffer[0] != GET) return false;

        std::string key = splittedBuffer[1];
        RemusDBUtils::DatabaseBlock* db = conn->getDbManager()->getDB();

        if (db->keyValue[key].expired) {
            rObject = new ProtocolUtils::ReturnObject("$-1\r\n", 0);
        } else {
            std::string response = ProtocolUtils::constructProtocol({db->keyValue[key].value}, false);
            rObject = new ProtocolUtils::ReturnObject(response, 0);
        }

        return true;
    }

    bool ProtocolIdentifier::identifyGetNoDB() {

        // Check if we have a db file.
        if (conn->getDirName().size() > 0 || splittedBuffer[0] != GET) return false;

        std::string key = splittedBuffer[1];

        Cache::DataManager cache;
        std::optional<std::string> value = cache.getValue(key);

        if (!value.has_value()) {
            PRINT_ERROR("KEY: " + key + " HAS NO VALUE");
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
        if (splittedBuffer[0] != CONFIG || splittedBuffer[1] != GET) return false;

        std::string& var = splittedBuffer[2];
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

        if (splittedBuffer[0] != KEYS || splittedBuffer[1] != "*") return false;

        std::vector<std::string> dbKeys {};
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

        if (splittedBuffer[0] != INFO) return false;

        std::string role = conn->getRole();
        std::transform(role.begin(), role.end(), role.begin(), ::tolower);

        std::string response = ProtocolUtils::constructProtocol(
        {"role:" + role, "master_repl_offset:0", "master_replid:8371b4fb1155b71f4a04d3e1bc3e18c4a990aeeb"},
        false, true);
        rObject = new ProtocolUtils::ReturnObject(response, 0);
        return true;
    }

    bool ProtocolIdentifier::identifyReplConfi() {

        if (splittedBuffer[0] != REPLCONF) return false;

        if (conn->getRole() == RemusConn::MASTER) {
            RemusConn::Master* masterConn = static_cast<RemusConn::Master*>(conn);

            if (!masterConn->inHandShakeWithReplica) {
                masterConn->inHandShakeWithReplica = true;
                masterConn->createCurrentReplicaConn();
                // the first replConf is the port
                std::string port = splittedBuffer[2];
                masterConn->setCurrentReplicaPort(std::stoi(port));
            }

        } else if (conn->getRole() == RemusConn::SLAVE) {
            RemusConn::Slave* masterConn = static_cast<RemusConn::Slave*>(conn);
            if (splittedBuffer[1] != GETACK) return false;
            std::string response = ProtocolUtils::constructProtocol({"REPLCONF", "ACK", "0"}, true);
            response = "*3\r\n$8\r\nREPLCONF\r\n$3\r\nACK\r\n$1\r\n0\r\n";
            rObject = new ProtocolUtils::ReturnObject(response, 0);
            return true;
        }


        std::string response = ProtocolUtils::constructProtocol({"OK"}, false);
        rObject = new ProtocolUtils::ReturnObject(response, 0);
        return true;

    }

    bool ProtocolIdentifier::identifyPsync() {

        if (splittedBuffer[0] != PSYNC) return false;

        rObject = new ProtocolUtils::ReturnObject("+FULLRESYNC "  + conn->getId() + " 0\r\n", 0);
        conn->sendDBFile = true;
        return true;
    }

    bool ProtocolIdentifier::identifyFullResync() {

        if (splittedBuffer[0] != FULLRESYNC) return false;

        std::string response = ProtocolUtils::constructProtocol({"REPLCONF", "ACK", "0"}, true);
        rObject = new ProtocolUtils::ReturnObject(response, 0, true);
        return true;
    }

}