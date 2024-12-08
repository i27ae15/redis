#pragma once
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>
#include <functional>
#include <map>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>

#include <serverConn/cache/cache.h>
#include <utils.h>
#include <regex>

namespace ProtocolUtils {
    struct ReturnObject;
    struct CommandObj;
}

namespace RomulusConn {
    class BaseConnection;
}
/**
 * @namespace ProtocolID
 * @brief Contains constants and classes related to the protocol identification and processing system.
 */
namespace ProtocolID {

    // Protocol Commands
    constexpr const char* GET        = "GET";       ///< Command to get a value.
    constexpr const char* SET        = "SET";       ///< Command to set a value.
    constexpr const char* PING       = "PING";      ///< Command to ping the server.
    constexpr const char* ECHO       = "ECHO";      ///< Command to echo a value.
    constexpr const char* KEYS       = "KEYS";      ///< Command to list keys.
    constexpr const char* INFO       = "INFO";      ///< Command to get server info.
    constexpr const char* WAIT       = "WAIT";      ///< Command to wait for replicas.
    constexpr const char* INCR       = "INCR";      ///< Command to increment a value.
    constexpr const char* EXEC       = "EXEC";      ///< Command to execute a transaction.
    constexpr const char* PSYNC      = "PSYNC";     ///< Command to synchronize data.
    constexpr const char* MULTI      = "MULTI";     ///< Command to start a transaction.
    constexpr const char* CONFIG     = "CONFIG";    ///< Command to get/set server configuration.
    constexpr const char* DISCARD    = "DISCARD";   ///< Command to discard a transaction.
    constexpr const char* REPLCONF   = "REPLCONF";  ///< Command to configure replication.
    constexpr const char* FULLRESYNC = "FULLRESYNC";///< Command for full resynchronization.

    // Protocol Responses
    constexpr const char* OK         = "OK";        ///< Response indicating success.
    constexpr const char* PONG       = "PONG";      ///< Response to a PING command.
    constexpr const char* GETACK     = "GETACK";    ///< Response for acknowledgement in replication.

    /**
     * @class ProtocolIdentifier
     * @brief Handles protocol identification, execution, and response generation.
     */
    class ProtocolIdentifier {
    public:
        /**
         * @brief Constructs a ProtocolIdentifier object.
         *
         * @param conn A pointer to the connection handling the protocol.
         */
        ProtocolIdentifier(RomulusConn::BaseConnection* conn);

        /**
         * @brief Destroys the ProtocolIdentifier object, releasing resources.
         */
        ~ProtocolIdentifier();

        // GETTERS

        /**
         * @brief Checks if a protocol is currently being processed.
         * @return True if a protocol is in process; otherwise, false.
         */
        bool getInProcess();

        /**
         * @brief Checks if the protocol is waiting for a response.
         * @return True if waiting; otherwise, false.
         */
        static bool getProIsWaiting();

        /**
         * @brief Retrieves the protocol identifier from the buffer.
         * @return The protocol identifier as a string.
         */
        std::string getIdFromBuffer();

        /**
         * @brief Gets the protocol string.
         * @return The protocol string.
         */
        std::string getProtocol();

        /**
         * @brief Retrieves the current response object.
         * @return A pointer to the current ReturnObject.
         */
        ProtocolUtils::ReturnObject* getRObject();

        /**
         * @brief Retrieves the command queue for the current client.
         * @return A reference to the command queue.
         */
        std::queue<ProtocolUtils::CommandObj>& getCommandQueue();

        // SETTERS

        /**
         * @brief Sets the in-process status of the protocol.
         * @param value True to set in-process; false otherwise.
         */
        void setInProcess(bool value);

        /**
         * @brief Sets the number of replicas required for acknowledgment.
         * @param n The number of replicas.
         */
        void setReplicasOscarKilo(unsigned short n);

        /**
         * @brief Cleans the response object, resetting it to a default state.
         */
        void cleanResponseObject();

        /**
         * @brief Interrupts the current wait operation.
         */
        void interruptWait();

        /**
         * @brief Processes a protocol command from a client.
         *
         * @param clientFD The file descriptor of the client.
         * @param rawBuffer The raw command buffer.
         * @param command The parsed command string.
         * @param commandSize The size of the command.
         * @param clearObject Indicates whether to clear the response object before processing.
         */
        void processProtocol(
            unsigned short clientFD,
            const std::string rawBuffer,
            const std::string command,
            const unsigned short commandSize,
            bool clearObject = true
        );

    private:
        /* --------------------------------------------------------- */
        /*                     VARIABLES                             */
        /* --------------------------------------------------------- */

        static bool pWrite; ///< Indicates if a write operation is in progress.
        static bool pIsWaiting; ///< Indicates if the protocol is waiting for a response.
        static std::unordered_map<unsigned short, std::queue<ProtocolUtils::CommandObj>> mCommands; ///< Stores commands per client.

        bool inProcess; ///< Indicates if a protocol is being processed.
        bool isExecute; ///< Indicates if the protocol is in the execution phase.

        unsigned short currentClient; ///< Current client ID.
        unsigned short currentCommandSize; ///< Current command size.

        std::atomic<bool> interruptFlag; ///< Atomic flag to handle interrupt operations.
        std::mutex mtx; ///< Mutex for synchronization.
        std::condition_variable cv; ///< Condition variable for waiting mechanisms.

        std::unordered_map<unsigned short, bool> allowExecutionOnClient; ///< Tracks client execution permissions.
        std::unordered_map<unsigned short, bool> multiCalledOnClient; ///< Tracks MULTI calls per client.

        RomulusConn::BaseConnection* conn; ///< Pointer to the connection object.
        ProtocolUtils::ReturnObject* rObject; ///< Pointer to the current response object.

        std::string buffer; ///< Buffer for the current command.
        std::string protocol; ///< The current protocol.
        std::string rawBuffer; ///< Raw buffer from the client.
        std::vector<std::string> splittedBuffer; ///< Splitted command arguments.

        std::unordered_map<std::string, std::function<bool()>> checkMethods; ///< Map of protocol commands to their handlers.

        /* --------------------------------------------------------- */
        /*                      METHODS                              */
        /* --------------------------------------------------------- */

        /**
         * @brief Identifies the protocol command and executes the corresponding action.
         * @param clearObject Indicates whether to clear the response object.
         * @return True if successful; false otherwise.
         */
        bool identifyProtocol(bool clearObject = true);

        // Action methods for specific commands
        bool actionForPing();
        bool actionForEcho();
        bool actionForSet();
        bool actionForGet();
        bool actionForGetNoDB();
        bool actionForGetWithDB();
        bool actionForConfig();
        bool actionForKeys();
        bool actionForInfo();
        bool actionForReplConf();
        bool actionForReplConfMaster();
        bool actionForReplConfSlave();
        bool actionForPsync();
        bool actionForFullResync();
        bool actionForWait();
        bool actionForIncr();
        bool actionForMulti();
        bool actionForExec();
        bool actionForDiscard();

        /**
         * @brief Processes special commands like MULTI, EXEC, and DISCARD.
         * @return True if a special command was processed; false otherwise.
         */
        bool processSpecialCommands();

        /**
         * @brief Processes the protocol command in detail.
         *
         * @param clientFD The file descriptor of the client.
         * @param rawBuffer The raw command buffer.
         * @param command The parsed command string.
         * @param commandSize The size of the command.
         * @param clearObject Indicates whether to clear the response object before processing.
         */
        void _processProtocol(
            const unsigned short clientFD,
            const std::string rawBuffer,
            const std::string command,
            const unsigned short commandSize,
            bool clearObject = true
        );

        /**
         * @brief Splits the current buffer into arguments.
         */
        void setSplitedBuffer();

        /**
         * @brief Processes all commands in the queue for the current client.
         */
        void processCommandQueue();

        /**
         * @brief Processes the database file for the current client.
         * @param clientFD The file descriptor of the client.
         */
        void processDBFile(unsigned short clientFD);

        /**
         * @brief Sends a response to the client.
         *
         * @param commandSize The size of the command.
         * @param clientFD The file descriptor of the client.
         * @param rObj The response object to send.
         */
        void sendResponse(
            unsigned short commandSize,
            unsigned short clientFD,
            ProtocolUtils::ReturnObject* rObj
        );
    };
}
