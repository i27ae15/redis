#include <sstream>

#include <utils.h>
#include <serverConn/cache/cache.h>

namespace Cache {

    unsigned long DataManager::lastStreamMiliseconds;
    unsigned short DataManager::lastSequenceNumber;

    std::unordered_map<std::string, strCacheValue> DataManager::strCache;
    std::unordered_map<std::string, intCacheValue> DataManager::intCache;

    std::unordered_map<
        StreamID,
        StreamEntry*, StreamIDHash, StreamIDEqual
    > DataManager::streamIdIndex;

    std::unordered_map<std::string, StreamValue> DataManager::streamKeyIndex;

    std::string StreamID::strRepresentation() const {
        return std::to_string(milliseconds) + "-" + std::to_string(sequenceNumber);
    }

    DataManager::DataManager() {}
    DataManager::~DataManager() {}

    bool DataManager::canConvertToInt(const std::string& str) {
        std::istringstream iss(str);
        int num;
        // Try to read an integer and ensure no leftover characters
        return (iss >> num) && (iss.eof());
    }

    bool DataManager::incrementValue(std::string key) {
        if (strCache.count(key)) return false;

        if (!intCache.count(key)) {
            saveValueAsInt(key, "1", false);
        } else {
            intCache[key].value++;
        }

        return true;
    }

    void DataManager::saveValue(
        std::string key,
        std::string value,
        bool expires,
        size_t expiresIn
    ) {

        if (canConvertToInt(value)) return saveValueAsInt(key, value, expires, expiresIn);
        saveValueAsStr(key, value, expires, expiresIn);
    }

    void DataManager::saveValueAsInt(
        std::string key,
        std::string value,
        bool expires,
        size_t expiresIn
    ) {
        setValue(key, value, expires, expiresIn, INT_CACHE);
    }

    void DataManager::saveValueAsStr(
        std::string key,
        std::string value,
        bool expires,
        size_t expiresIn
    ) {
        setValue(key, value, expires, expiresIn, STR_CACHE);
    }

    OperationResult DataManager::validateStreamID(
        const StreamID& streamId
    ) {

        if (!streamId.milliseconds && !streamId.sequenceNumber) return {false, "The ID specified in XADD must be greater than 0-0"};

        if (
            (lastStreamMiliseconds > streamId.milliseconds) ||
            (lastStreamMiliseconds == streamId.milliseconds && lastSequenceNumber >= streamId.sequenceNumber)
        ) {
            return {false, "The ID specified in XADD is equal or smaller than the target stream top item"};
        }

        lastStreamMiliseconds = streamId.milliseconds;
        lastSequenceNumber = streamId.sequenceNumber;

        return {true, ""};
    }

    StreamID DataManager::createFullStreamId() {
        uint64_t milliseconds = RomulusUtils::getUnixTimeMilliseconds();
        uint16_t sequenceNumber {};

        return StreamID{milliseconds, sequenceNumber};
    }

    StreamID DataManager::createPartialStreamId(const std::string& key, const uint64_t milliseconds) {

        uint16_t sequenceNumber {};
        // Check if the milliseconds is already a key on the stream
        if (streamKeyIndex.count(key)) {
            const StreamID& lastId = streamKeyIndex[key].lastID;
            if (lastId.milliseconds == milliseconds) {
                sequenceNumber = lastId.sequenceNumber + 1;
            }
        } else {
            sequenceNumber = 1;
        }

        return StreamID{milliseconds, sequenceNumber};
    }

    StreamIdResult DataManager::createStreamId(
        const std::string& key,
        const std::string& rawId
    ) {

        // Check if we have to create the entire key
        // We don't need to validate it since we know that this is correct;
        if (rawId == "*") return {{true, ""}, createFullStreamId()};

        StreamID streamId {};
        uint64_t milliseconds {};
        uint16_t sequenceNumber {};

        std::vector<std::string> splitedId = RomulusUtils::splitString(rawId, "-");
        std::string& sequence = splitedId[1];

        milliseconds = std::stol(splitedId[0]);


        if (sequence == "*") {
            streamId = createPartialStreamId(key, milliseconds);
        } else {
            sequenceNumber = std::stoi(splitedId[1]);
            streamId = {milliseconds, sequenceNumber};
        }

        OperationResult oResult = validateStreamID(streamId);
        return StreamIdResult{oResult, streamId};
    }

    OperationResult DataManager::saveMultipleValuesToStream(
        std::string& key,
        std::string& rawId,
        std::vector<std::pair<std::string, std::string>>& values
    ) {

        StreamIdResult streamIdResult = createStreamId(key, rawId);

        OperationResult& vResult = streamIdResult.result;
        StreamID& streamId = streamIdResult.streamId;

        if (!vResult.isValid) return vResult;

        for (std::pair<std::string, std::string> v : values) {
            saveValueToStream(key, streamId, v);
        }

        return vResult;
    }

    void DataManager::saveValueToStream(
        const std::string& key,
        const StreamID& streamId,
        std::pair<std::string, std::string>& value
    ) {

        // Add the value to the StreamKeyIndex

        // Create the stream value to insert
        StreamValue& streamValue = streamKeyIndex[key];
        // Ensure the streamId exists in the values map
        if (!streamValue.values[streamId]) {
            streamValue.values[streamId] = new StreamEntry{streamId, key, {}};
        }

        StreamEntry* streamEntry = streamValue.values[streamId];

        streamEntry->id = streamId;
        streamEntry->streamKey = key;
        streamEntry->fields[value.first] = value.second;

        streamValue.lastID = streamId;

        // Add value to the streamIdIndex
        streamIdIndex[streamId] = streamEntry;

        // PRINT_HIGHLIGHT("KEY: " + key + " SAVE INTO STREAM WITH KEY: " + value.first + " AND VALUE " + value.second);
    }

    StreamIdResult DataManager::saveValueToStream(
        std::string& key,
        std::string& rawId,
        std::pair<std::string, std::string> value
    ) {

        StreamIdResult streamIdResult = createStreamId(key, rawId);
        OperationResult& vResult = streamIdResult.result;
        StreamID& streamId = streamIdResult.streamId;

        if (!vResult.isValid) return streamIdResult;
        saveValueToStream(key, streamId, value);

        return streamIdResult;
    }

    void DataManager::setValue(
        std::string key,
        std::string value,
        bool expires,
        size_t expiresIn,
        unsigned short cacheToUse
    ) {

        std::optional<std::chrono::time_point<std::chrono::system_clock>> expireDate {};

        if (expires) {
            auto now = std::chrono::system_clock::now();
            expireDate = now + std::chrono::milliseconds(expiresIn);
        }

        switch (cacheToUse) {
            case INT_CACHE:
                intCache[key] = {std::stoi(value), expireDate};
                break;

            case STR_CACHE:
                strCache[key] = {value, expireDate};
                break;

            default:
                break;
        }

        // PRINT_HIGHLIGHT("VALUE SET FOR: " + key);
    }

    bool DataManager::hasExpired(
        std::optional<std::chrono::time_point<std::chrono::system_clock>> dt
    ) {
        if (dt.has_value()) {
            auto now = std::chrono::system_clock::now();

            if (now > dt.value()) {
                return true;
            }
        }

        return false;
    }

    std::optional<std::string> DataManager::checkCache(strCacheValue cachedValue) {
        if (hasExpired(cachedValue.expiredDate)) return std::nullopt;

        return cachedValue.value;
    }

    std::optional<std::string> DataManager::checkCache(intCacheValue cachedValue) {
        if (hasExpired(cachedValue.expiredDate)) return std::nullopt;

        return std::to_string(cachedValue.value);
    }

    std::optional<std::string> DataManager::getValue(std::string key) {

        if (strCache.count(key)) return checkCache(strCache[key]);
        if (intCache.count(key)) return checkCache(intCache[key]);

        return std::nullopt;
    }

    std::string DataManager::getKeyType(std::string key) {
        if (strCache.count(key) > 0 || intCache.count(key) > 0) return STRING;
        if (streamKeyIndex.count(key) > 0) return STREAM;

        return NONE;
    }
}