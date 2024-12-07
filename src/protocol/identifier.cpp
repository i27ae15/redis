#include <algorithm>
#include <thread>
#include <chrono>
#include <memory>
#include <queue>

#include <serverConn/connection/base.h>
#include <serverConn/connection/master.h>
#include <serverConn/connection/slave.h>

#include <db/structs.h>
#include <db/db_manager.h>

#include <protocol/identifier.h>
#include <protocol/utils.h>

namespace ProtocolID {

    std::queue<ProtocolUtils::CommandObj> ProtocolIdentifier::qCommands {};
    bool ProtocolIdentifier::pWrite {};
    bool ProtocolIdentifier::pIsWaiting {};

    ProtocolIdentifier::ProtocolIdentifier(RomulusConn::BaseConnection *conn) :
    conn {conn},
    protocol {},
    splittedBuffer {},
    rawBuffer {},
    interruptFlag {},
    isExecute {},
    allowExecutionOnClient {},
    multiCalledOnClient {},
    currentCommandSize {},
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
        {FULLRESYNC, [this]() { return actionForFullResync(); }},
        {WAIT, [this]() { return actionForWait(); }},
        {INCR, [this]() { return actionForIncr(); }},
        {MULTI, [this]() { return actionForMulti(); }},
        {EXEC, [this]() { return actionForExec(); }},
        {DISCARD, [this]() { return actionForDiscard(); }},
    },
    rObject {new ProtocolUtils::ReturnObject("+\r\n", 0)}
    {
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

    bool ProtocolIdentifier::getProIsWaiting() {
        return pIsWaiting;
    }

    // Setters

    void ProtocolIdentifier::setReplicasOscarKilo(unsigned short n) {
        RomulusConn::Master* mConn = static_cast<RomulusConn::Master*>(conn);
        mConn->setReplicasOscarKilo(0);
    }

    void ProtocolIdentifier::setInProcess(bool value) {
        inProcess = value;
    }

    void ProtocolIdentifier::setSplitedBuffer() {
        splittedBuffer = RomulusUtils::splitString(buffer, " ");
    }

    // METHODS

    void ProtocolIdentifier::processProtocol(
        unsigned short clientFD,
        const std::string rawBuffer,
        const std::string command,
        const unsigned short commandSize,
        bool clearObject
    ) {

        setInProcess(true);
        _processProtocol(clientFD, rawBuffer, command, commandSize, clearObject);
        setInProcess(false);
    }

    void ProtocolIdentifier::_processProtocol(
        unsigned short clientFD,
        const std::string rawBuffer,
        const std::string command,
        const unsigned short commandSize,
        bool clearObject
    ) {

        currentClient = clientFD;

        this->buffer = command;
        this->rawBuffer = rawBuffer;
        this->currentCommandSize = commandSize;
        setSplitedBuffer();

        // Separate MULTI and EXEC
        if (processSpecialCommands()) return;

        if (!allowExecutionOnClient.count(clientFD)) allowExecutionOnClient[clientFD] = true;
        bool& cExecute = allowExecutionOnClient[clientFD];

        if (cExecute && !isExecute) {
            identifyProtocol(clearObject);
            sendResponse(commandSize, clientFD, rObject);

            if (conn->sendDBFile) {
                processDBFile(clientFD);
                sendResponse(0, clientFD, rObject);
            }

            return;
        }

        if (cExecute) {
            isExecute = false;
            processCommandQueue();
            return;
        }

        if (!cExecute) {
            qCommands.push(
                ProtocolUtils::CommandObj{splittedBuffer, commandSize, clientFD}
            );
            rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::QUEUE_R);
            sendResponse(0, clientFD, rObject);
        }

    }

    bool ProtocolIdentifier::processSpecialCommands() {

        bool toReturn {};

        if (getIdFromBuffer() == EXEC) {
            toReturn = !(actionForExec());

        } else if (getIdFromBuffer() == MULTI) {
            actionForMulti();
            toReturn = true;

        } else if (getIdFromBuffer() == DISCARD) {
            actionForDiscard();
            toReturn = true;
        }

        return toReturn;
    }

    void ProtocolIdentifier::processDBFile(unsigned short clientFD) {
        unsigned short fileLength = conn->getDbFile().size();
        rObject = new ProtocolUtils::ReturnObject("$" + std::to_string(fileLength) + "\r\n" + conn->getDbFile(), 0);
        conn->sendDBFile = false;

        RomulusConn::Master* masterConn = static_cast<RomulusConn::Master*>(conn);
        masterConn->setCurrentReplicaServerFd(clientFD);
        masterConn->addAndCleanCurrentReplicaConn();
        masterConn->inHandShakeWithReplica = false;
    }

    bool ProtocolIdentifier::identifyProtocol(bool clearObject) {
        if (clearObject) cleanResponseObject();

        // PRINT_HIGHLIGHT(getIdFromBuffer() + " : " + std::to_string(commandSize));
        if (checkMethods.count(getIdFromBuffer())) {
            if (!checkMethods[getIdFromBuffer()]()) {

                rObject = new ProtocolUtils::ReturnObject(
                    ProtocolUtils::constructError("PROTOCOL: Could Not Be Identified")
                );
                conn->print("Protocol could not be identified: " + buffer, RED);

                return false;
            }
        }

        return true;
    }

    bool ProtocolIdentifier::actionForPing() {

        rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::PONG_R);

        if (conn->getRole() == RomulusConn::SLAVE) rObject->sendResponse = false;

        return true;
    }

    bool ProtocolIdentifier::actionForEcho() {

        std::string &toEcho = splittedBuffer[1];
        // std::string echo = "$" + std::to_string(pre_echo.size()) + "\r\n" + pre_echo + "\r\n";
        std::string response = ProtocolUtils::constructRestBulkString({toEcho});
        rObject = new ProtocolUtils::ReturnObject(response);

        return true;
    }

    bool ProtocolIdentifier::actionForSet() {
        // *3$3set$9pineapple$6banana
        pWrite = true;
        RomulusConn::Master* mConn = static_cast<RomulusConn::Master*>(conn);
        mConn->setReplicasOscarKilo(0);

        std::string &key = splittedBuffer[1];
        std::string &value = splittedBuffer[2];

        // Check if has px
        std::pair<bool, size_t> expireTime {};

        if (splittedBuffer.size() > 3 && splittedBuffer[3] == "px") {
            expireTime.second = std::stoi(splittedBuffer[4]);
            expireTime.first = true;
        }

        conn->getCache()->saveValue(key, value, expireTime.first, expireTime.second);

        // PRINT_SUCCESS("VALUE SET FOR KEY: " + key + " : " + value);

        rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::OK_R);
        if (conn->getRole() == RomulusConn::SLAVE) {
            rObject->sendResponse = false;
            return true;
        }

        std::thread(
            &RomulusConn::Master::propagueProtocolToReplica,
            mConn,
            this,
            rawBuffer
        ).detach();

        return true;
    }

    bool ProtocolIdentifier::actionForGet() {

        if (conn->getDirName().size() == 0) return actionForGetNoDB();
        if (conn->getDirName().size() > 0) return actionForGetWithDB();

        PRINT_ERROR("GETTING TO END OF AFCTION_FOR_GET");
        return false;
    }

    bool ProtocolIdentifier::actionForGetWithDB() {

        std::string key = splittedBuffer[1];
        RomulusDbStructs::DatabaseBlock* db = conn->getDbManager()->getDB();

        if (db->keyValue[key].expired) {
            rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::NONE_R);
        } else {
            std::string response = ProtocolUtils::constructRestBulkString(
                {db->keyValue[key].value}
            );
            rObject = new ProtocolUtils::ReturnObject(response, 0);
        }

        return true;
    }

    bool ProtocolIdentifier::actionForGetNoDB() {

        std::string key = splittedBuffer[1];

        std::optional<std::string> value = conn->getCache()->getValue(key);

        if (!value.has_value()) {
            // PRINT_ERROR("KEY: " + key + " HAS NO VALUE");
            rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::NONE_R);
        } else {
            std::string response {};
            response = ProtocolUtils::constructSimpleString({value.value()});
            rObject = new ProtocolUtils::ReturnObject(response, 0);
            return true;
            // PRINT_SUCCESS("KEY: " + key + " HAS VALUE");
            // if (RomulusUtils::canConvertToInt(value.value())) {
            //     response = ProtocolUtils::constructInteger(value.value());
            // } else {
            //     response = ProtocolUtils::constructRestBulkString({value.value()});
            // }
            // rObject = new ProtocolUtils::ReturnObject(response, 0);
        }

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

        std::string response = ProtocolUtils::constructArray(vars);
        rObject = new ProtocolUtils::ReturnObject(response, 0);

        return true;
    }

    bool ProtocolIdentifier::actionForKeys() {
        // Example of key
        // *2$4keys$1*

        if (splittedBuffer[1] != "*") return false;

        std::vector<std::string> dbKeys {};
        RomulusDbStructs::DatabaseBlock* db = conn->getDbManager()->getDB();

        // returning all the keys
        std::map<std::string, RomulusDbStructs::InfoBlock>& keys = db->keyValue;
        for (std::map<std::string, RomulusDbStructs::InfoBlock>::iterator it = keys.begin(); it != keys.end(); ++it) {
            dbKeys.push_back(it->first);
         }

        std::string response = ProtocolUtils::constructArray(dbKeys);
        rObject = new ProtocolUtils::ReturnObject(response);

        return true;
    }

    bool ProtocolIdentifier::actionForInfo() {

        std::string role = conn->getRole();
        std::transform(role.begin(), role.end(), role.begin(), ::tolower);

        std::string response = ProtocolUtils::constructBulkString(
            {
                "role:" + role,
                "master_repl_offset:0",
                "master_replid:8371b4fb1155b71f4a04d3e1bc3e18c4a990aeeb"
            }
        );

        rObject = new ProtocolUtils::ReturnObject(response);

        return true;
    }

    bool ProtocolIdentifier::actionForReplConf() {

        if (conn->getRole() == RomulusConn::MASTER) return actionForReplConfMaster();
        if (conn->getRole() == RomulusConn::SLAVE) return actionForReplConfSlave();

        PRINT_ERROR("WTF IS THE ROLE FOR THIS CONN?");
        return false;
    }

    bool ProtocolIdentifier::actionForReplConfMaster() {

        RomulusConn::Master* masterConn = static_cast<RomulusConn::Master*>(conn);

        if (!masterConn->inHandShakeWithReplica && splittedBuffer[1] != "ACK") {
            masterConn->inHandShakeWithReplica = true;
            masterConn->createCurrentReplicaConn();
            // the first replConf is the port
            std::string port = splittedBuffer[2];
            masterConn->setCurrentReplicaPort(std::stoi(port));
        }

        rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::OK_R);
        if (splittedBuffer[1] == "ACK") {
            RomulusConn::Master* mConn = static_cast<RomulusConn::Master*>(conn);
            mConn->incrementReplicasOscarKilo();
            rObject->sendResponse = false;
        };

        return true;
    }

    bool ProtocolIdentifier::actionForReplConfSlave() {

        RomulusConn::Slave* masterConn = static_cast<RomulusConn::Slave*>(conn);
        if (splittedBuffer[1] != GETACK) return false;

        PRINT_SUCCESS("Activating listener");
        conn->startProcessingBytes();

        std::string response = ProtocolUtils::constructArray(
            {"REPLCONF", "ACK", std::to_string(conn->getBytesProcessed())}
        );
        rObject = new ProtocolUtils::ReturnObject(response, 0);

        return true;
    }

    bool ProtocolIdentifier::actionForPsync() {

        std::string response = ProtocolUtils::constructSimpleString(
            {"+FULLRESYNC", conn->getId(), "0"}
        );
        rObject = new ProtocolUtils::ReturnObject(response);
        conn->sendDBFile = true;

        // Make this work
        // processDBFile(currentClient);
        // sendResponse(0, currentClient, rObject);

        return true;
    }

    bool ProtocolIdentifier::actionForFullResync() {

        rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::NONE_R, 0, false);

        return true;
    }

    bool ProtocolIdentifier::actionForWait() {

        // WAIT 1 500

        pIsWaiting = true;

        RomulusConn::Master* mConn = static_cast<RomulusConn::Master*>(conn);
        mConn->setReplicasToAck(std::stoi(splittedBuffer[1]));
        unsigned short milliseconds = std::stoi(splittedBuffer[2]);

        std::unique_lock<std::mutex> lock(mtx);
        std::thread(&RomulusConn::Master::getReplicasACKs, mConn).detach();

        // Better this to interrupt before timeout
        if (mConn->getReplicasToAck() > 0) {
            // PRINT_HIGHLIGHT("TO WAIT " + splittedBuffer[1]);

            if (!cv.wait_for(lock, std::chrono::milliseconds(milliseconds), [this, mConn] {
                return interruptFlag.load();
            })) {
                // PRINT_ERROR("TIMEOUT REACHED");
            }

            // PRINT_HIGHLIGHT("BREAKING WAITING");
        } else {
            setReplicasOscarKilo(mConn->getReplicasToAck());
        }

        interruptFlag.store(false);

        unsigned short ACKs = mConn->getReplicasOscarKilo();
        if (!pWrite) ACKs = mConn->getNumReplicas();

        std::string response = ProtocolUtils::constructInteger(std::to_string(ACKs));
        rObject = new ProtocolUtils::ReturnObject(response);

        mConn->setReplicasToAck(0);

        pWrite = false; // This should not be here, but whatever
        pIsWaiting = false;

        return true;
    }

    bool ProtocolIdentifier::actionForIncr() {

        std::string key = splittedBuffer[1];

        if (!conn->getCache()->incrementValue(key)) {
            rObject = new ProtocolUtils::ReturnObject(
                ProtocolUtils::constructError("value is not an integer or out of range")
            );
        } else {
            rObject = new ProtocolUtils::ReturnObject(
                ProtocolUtils::constructInteger(conn->getCache()->getValue(key).value())
            );
        }

        return true;
    }

    bool ProtocolIdentifier::actionForMulti() {

        allowExecutionOnClient[currentClient] = false;
        multiCalledOnClient[currentClient] = true;
        rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::OK_R);
        sendResponse(currentCommandSize, currentClient, rObject);

        return true;
    }

    bool ProtocolIdentifier::actionForExec() {
        allowExecutionOnClient[currentClient] = true;

        if (!multiCalledOnClient[currentClient]) {
            rObject = new ProtocolUtils::ReturnObject(
                ProtocolUtils::constructError("EXEC without MULTI")
            );
            sendResponse(currentCommandSize, currentClient, rObject);

            return false;
        }

        multiCalledOnClient[currentClient] = false;
        isExecute = true;

        return true;
    }

    bool ProtocolIdentifier::actionForDiscard() {

        if (!multiCalledOnClient[currentClient]) {
            rObject = new ProtocolUtils::ReturnObject(
                ProtocolUtils::constructError("DISCARD without MULTI")
            );
        } else {
            qCommands = {};

            multiCalledOnClient[currentClient] = false;
            allowExecutionOnClient[currentClient] = true;

            rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::OK_R);
        }

        sendResponse(currentCommandSize, currentClient, rObject);

        return true;
    }

    void ProtocolIdentifier::sendResponse(
        unsigned short commandSize,
        unsigned short clientFD,
        ProtocolUtils::ReturnObject* rObj
    ) {
        conn->addBytesProcessed(commandSize);

        if (!rObj->sendResponse) return;
        send(clientFD, rObj->return_value.c_str(), rObj->bytes, rObj->behavior);
    }

    void ProtocolIdentifier::processCommandQueue() {

        std::vector<std::string> responses {};

        if (qCommands.empty()) {
            rObject = new ProtocolUtils::ReturnObject(ProtocolTypes::EMPTY_ARRAY);
            sendResponse(0, currentClient, rObject);
            return;
        }

        while (!qCommands.empty()) {

            ProtocolUtils::CommandObj& cObj = qCommands.front();
            splittedBuffer = cObj.splittedBuffer;
            identifyProtocol();

            std::string& rValue = rObject->return_value;
            if (!rValue.empty() && (rValue[0] == '+' || rValue[0] == '$')) {
                rValue = rValue.substr(1); // Remove the first character

                // Step 2: Remove trailing '\r\n'
                if (rValue.size() >= 2 && rValue.substr(rValue.size() - 2) == "\r\n") {
                    rValue = rValue.substr(0, rValue.size() - 2); // Remove the last two characters
                }
            }


            responses.push_back(rValue);
            qCommands.pop();
        }

        rObject = new ProtocolUtils::ReturnObject(
            ProtocolUtils::constructArray(responses)
        );

        // PRINT_HIGHLIGHT("HERE: " + rObject->return_value + " | FROM: " + getIdFromBuffer());
        sendResponse(rObject->bytes, currentClient, rObject);

    }

    void ProtocolIdentifier::interruptWait() {
        PRINT_SUCCESS("INTERRUPTING WAIT");
        std::lock_guard<std::mutex> lock(mtx);
        interruptFlag.store(true);
        cv.notify_all();
    }


}