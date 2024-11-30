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
        {PING, [this]() { return actionForPing(); }},
        {ECHO, [this]() { return actionForEcho(); }},
        {SET, [this]() { return actionForSet(); }},
        {GET, [this]() { return actionForGet(); }},
        {CONFIG, [this]() { return actionForConfig(); }},
        {KEYS, [this]() { return actionForKeys(); }},
        {INFO, [this]() { return actionForInfo(); }},
        {REPLCONF, [this]() { return actionForReplConf(); }},
        {PSYNC, [this]() { return actionForPsync(); }},
        {FULLRESYNC, [this]() { return actionForFullResync(); }}
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

    std::string ProtocolIdentifier::getIdFromBuffer() {
        if (!splittedBuffer.size()) {
            PRINT_ERROR("NO BUFFER TO WORK WITH");
            throw std::runtime_error("");
        }

        return splittedBuffer[0];
    }

    // Setters

    void ProtocolIdentifier::setInProcess(bool value) {
        inProcess = value;
    }

    void ProtocolIdentifier::setSplitedBuffer() {
        splittedBuffer = RemusUtils::splitString(buffer, " ");
    }

    bool ProtocolIdentifier::identifyProtocol(
        const std::string rawBuffer,
        const std::string command,
        const unsigned short commandSize,
        bool clearObject
    ) {

        setInProcess(true);
        if (clearObject) cleanResponseObject();

        this->buffer = command;
        this->rawBuffer = rawBuffer;

        setSplitedBuffer();

        // PRINT_HIGHLIGHT(getIdFromBuffer() + " : " + std::to_string(commandSize));

        bool foundProtocol {};
        if (checkMethods.count(getIdFromBuffer())) {
            foundProtocol = checkMethods[getIdFromBuffer()]();
        }

        if (!foundProtocol) conn->print("Protocol could not be identified: " + buffer, RED);

        conn->addBytesProcessed(commandSize);
        // PRINT_HIGHLIGHT("After calling method: " + std::to_string(conn->getBytesProcessed()));

        setInProcess(false);
        return foundProtocol;
    }

    bool ProtocolIdentifier::actionForPing() {

        rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::PONG_R);

        if (conn->getRole() == RemusConn::SLAVE) rObject->sendResponse = false;

        return true;
    }

    bool ProtocolIdentifier::actionForEcho() {

        std::string &toEcho = splittedBuffer[1];
        // std::string echo = "$" + std::to_string(pre_echo.size()) + "\r\n" + pre_echo + "\r\n";
        std::string response = ProtocolUtils::constructProtocol({toEcho}, ProtocolTypes::ResponseType::RBSTRING);
        rObject = new ProtocolUtils::ReturnObject(response);

        return true;
    }

    bool ProtocolIdentifier::actionForSet() {
        // *3$3set$9pineapple$6banana

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

        rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::OK_R);
        if (conn->getRole() == RemusConn::SLAVE) {
            rObject->sendResponse = false;
            return true;
        }

        RemusConn::Master* masterConn = static_cast<RemusConn::Master*>(conn);
        masterConn->propageProtocolToReplica(rawBuffer);

        return true;
    }

    bool ProtocolIdentifier::actionForGet() {

        if (conn->getDirName().size() == 0) return actionForGetNoDB();

        return actionForGetWithDB();
    }

    bool ProtocolIdentifier::actionForGetWithDB() {

        std::string key = splittedBuffer[1];
        RemusDBUtils::DatabaseBlock* db = conn->getDbManager()->getDB();

        if (db->keyValue[key].expired) {
            rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::NONE_R);
        } else {
            std::string response = ProtocolUtils::constructProtocol({db->keyValue[key].value}, ProtocolTypes::ResponseType::RBSTRING);
            rObject = new ProtocolUtils::ReturnObject(response, 0);
        }

        return true;
    }

    bool ProtocolIdentifier::actionForGetNoDB() {

        std::string key = splittedBuffer[1];

        Cache::DataManager cache;
        std::optional<std::string> value = cache.getValue(key);

        if (!value.has_value()) {
            PRINT_ERROR("KEY: " + key + " HAS NO VALUE");
            rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::NONE_R);
            return false;
        }

        std::string response = ProtocolUtils::constructProtocol({value.value()}, ProtocolTypes::ResponseType::RBSTRING);
        rObject = new ProtocolUtils::ReturnObject(response, 0);

        return true;
    }

    bool ProtocolIdentifier::actionForConfig() {
        // client: $ redis-cli CONFIG GET dir
        // actionForConfigGet : *3$6config$3get$3dir
        if (splittedBuffer[1] != GET) return false;

        std::string& var = splittedBuffer[2];
        std::vector<std::string> vars {};

        if (var == "dir") {
            vars = {"dir", conn->getDirName()};
        } else if (var == "dbfilename") {
            vars = {"dbfilename", conn->getFileName()};
        }

        std::string response = ProtocolUtils::constructProtocol(vars, ProtocolTypes::ResponseType::ARRAY);
        rObject = new ProtocolUtils::ReturnObject(response, 0);

        return true;
    }

    bool ProtocolIdentifier::actionForKeys() {
        // Example of key
        // *2$4keys$1*

        if (splittedBuffer[1] != "*") return false;

        std::vector<std::string> dbKeys {};
        RemusDBUtils::DatabaseBlock* db = conn->getDbManager()->getDB();

        // returning all the keys
        std::map<std::string, RemusDBUtils::InfoBlock>& keys = db->keyValue;
        for (std::map<std::string, RemusDBUtils::InfoBlock>::iterator it = keys.begin(); it != keys.end(); ++it) {
            dbKeys.push_back(it->first);
         }

        std::string response = ProtocolUtils::constructProtocol(dbKeys, ProtocolTypes::ResponseType::ARRAY);
        rObject = new ProtocolUtils::ReturnObject(response);

        return true;
    }

    bool ProtocolIdentifier::actionForInfo() {

        std::string role = conn->getRole();
        std::transform(role.begin(), role.end(), role.begin(), ::tolower);

        std::string response = ProtocolUtils::constructProtocol(
            {"role:" + role, "master_repl_offset:0", "master_replid:8371b4fb1155b71f4a04d3e1bc3e18c4a990aeeb"},
            ProtocolTypes::ResponseType::BSTRING
        );

        rObject = new ProtocolUtils::ReturnObject(response);

        return true;
    }

    bool ProtocolIdentifier::actionForReplConf() {

        if (conn->getRole() == RemusConn::MASTER) return actionForReplConfMaster();
        if (conn->getRole() == RemusConn::SLAVE) return actionForReplConfSlave();

        return false;
    }

    bool ProtocolIdentifier::actionForReplConfMaster() {

        RemusConn::Master* masterConn = static_cast<RemusConn::Master*>(conn);

        if (!masterConn->inHandShakeWithReplica) {
            masterConn->inHandShakeWithReplica = true;
            masterConn->createCurrentReplicaConn();
            // the first replConf is the port
            std::string port = splittedBuffer[2];
            masterConn->setCurrentReplicaPort(std::stoi(port));
        }

        rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::OK_R);

        return true;
    }

    bool ProtocolIdentifier::actionForReplConfSlave() {

        RemusConn::Slave* masterConn = static_cast<RemusConn::Slave*>(conn);
        if (splittedBuffer[1] != GETACK) return false;

        PRINT_SUCCESS("Activating listener");
        conn->startProcessingBytes();

        std::string response = ProtocolUtils::constructProtocol({"REPLCONF", "ACK", std::to_string(conn->getBytesProcessed())}, ProtocolTypes::ResponseType::ARRAY);
        rObject = new ProtocolUtils::ReturnObject(response, 0);

        return true;
    }

    bool ProtocolIdentifier::actionForPsync() {

        std::string response = ProtocolUtils::constructProtocol({"+FULLRESYNC", conn->getId(), "0"}, ProtocolTypes::ResponseType::SSTRING);
        rObject = new ProtocolUtils::ReturnObject(response);
        conn->sendDBFile = true;

        return true;
    }

    bool ProtocolIdentifier::actionForFullResync() {

        rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::NONE_R, 0, false);
        return true;
    }

}