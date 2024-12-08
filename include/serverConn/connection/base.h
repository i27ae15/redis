#pragma once
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>
#include <atomic>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <utils.h>

namespace Cache {
    class DataManager;
}

namespace RomulusDB {
    class DbManager;
}

namespace ProtocolID {
    class ProtocolIdentifier;
}

/**
 * @namespace RomulusConn
 * @brief Manages connections for Master and Slave nodes in the Romulus system.
 */
namespace RomulusConn {

    constexpr const char* MASTER = "MASTER"; ///< Constant for identifying Master role.
    constexpr const char* SLAVE  = "SLAVE";  ///< Constant for identifying Slave role.

    /**
     * @class BaseConnection
     * @brief Abstract base class to manage connections for Slave and Master nodes.
     */
    class BaseConnection {

        public:

        /**
         * @brief Constructs a BaseConnection object.
         *
         * @param port The port number for the connection.
         * @param host The hostname or IP address for the connection.
         * @param dirName The directory name (optional, default: "").
         * @param fileName The file name (optional, default: "").
         */
        BaseConnection(
            unsigned short port,
            std::string host,
            std::string dirName = "",
            std::string fileName = ""
        );

        /**
         * @brief Destroys the BaseConnection object and cleans up resources.
         */
        virtual ~BaseConnection();

        /* --------------------------------------------------------- */
        /*                     VARIABLES                             */
        /* --------------------------------------------------------- */

        int rs; ///< Replica status flag.
        bool sendDBFile; ///< Indicates if a database file should be sent.
        bool replicaHand; ///< Indicates if the replica handshake is active.

        /* --------------------------------------------------------- */
        /*                      METHODS                              */
        /* --------------------------------------------------------- */

        /* --------------------------------------------------------- */
        /*                      GETTERS                              */
        /* --------------------------------------------------------- */

        /**
         * @brief Retrieves the cache manager instance. Only one instance will server
         * for all the connections under the same run.
         *
         * @return A pointer to the cache manager.
         */
        static Cache::DataManager* getCache();

        /**
         * @brief Gets the role of the connection (MASTER or SLAVE).
         *
         * @return A string indicating the role of the connection.
         */
        virtual std::string getRole() const = 0;

        /**
         * @brief Retrieves the connection status.
         *
         * @return True if the connection is active; otherwise, false.
         */
        bool getConnectionStatus();

        /**
         * @brief Retrieves the port number for the connection.
         *
         * @return The port number.
         */
        unsigned short getPort();

        /**
         * @brief Retrieves the server file descriptor.
         *
         * @return The server file descriptor.
         */
        unsigned short getServerFD();

        /**
         * @brief Retrieves the total number of bytes processed.
         *
         * @return The number of bytes processed.
         */
        unsigned int getBytesProcessed();

        /**
         * @brief Retrieves the unique identifier for the connection.
         *
         * @return A string representing the unique ID.
         */
        std::string getId();

        /**
         * @brief Retrieves the hostname or IP address for the connection.
         *
         * @return The hostname or IP address as a string.
         */
        std::string getHost();

        /**
         * @brief Retrieves the database file name.
         *
         * @return The database file name as a string.
         */
        std::string getDbFile();

        /**
         * @brief Retrieves the directory name.
         *
         * @return The directory name as a string.
         */
        std::string getDirName();

        /**
         * @brief Retrieves the file name.
         *
         * @return The file name as a string.
         */
        std::string getFileName();

        /**
         * @brief Retrieves the database manager instance.
         *
         * @return A pointer to the database manager.
         */
        RomulusDB::DbManager* getDbManager();

        /**
         * @brief Retrieves the protocol identifier instance.
         *
         * @return A pointer to the protocol identifier.
         */
        ProtocolID::ProtocolIdentifier* getProtocolIdr();

        /* --------------------------------------------------------- */
        /*                      SETTERS                              */
        /* --------------------------------------------------------- */

        /**
         * @brief Sets the protocol identifier for the connection.
         *
         * @param protocolIdr A pointer to the protocol identifier.
         * @param override Indicates whether to override the existing protocol identifier (default: false).
         */
        void setProtocolIdr(
            ProtocolID::ProtocolIdentifier* protocolIdr,
            bool override = false
        );

        /**
         * @brief Sets the database manager for the connection.
         *
         * @param dbManager A pointer to the database manager.
         * @param override Indicates whether to override the existing database manager (default: false).
         */
        void setDbManager(
            RomulusDB::DbManager* dbManager,
            bool override = false
        );

        /**
         * @brief Starts tracking the number of bytes processed by the connection.
         */
        void startProcessingBytes();

        /**
         * @brief Adds to the total number of bytes processed.
         *
         * @param bytes The number of bytes to add.
         */
        void addBytesProcessed(unsigned short bytes);

        /* --------------------------------------------------------- */
        /*                       UTILS                               */
        /* --------------------------------------------------------- */

        /**
         * @brief Prints a message to the console, prefixed by the connection role.
         *
         * @param msg The message to print.
         */
        void print(std::string msg);

        /**
         * @brief Prints a message to the console in the specified color, prefixed by the connection role.
         *
         * @param msg The message to print.
         * @param color The color code for the message.
         */
        void print(std::string msg, std::string color);

        /* --------------------------------------------------------- */
        /*                      PRIVATE                              */
        /* --------------------------------------------------------- */

        private:

        /* --------------------------------------------------------- */
        /*                     VARIABLES                             */
        /* --------------------------------------------------------- */

        static Cache::DataManager* cache; ///< Static cache manager instance.

        unsigned short port; ///< Port number for the connection.
        unsigned short serverFD; ///< File descriptor for the server socket.
        unsigned int bytesProcessed; ///< Total number of bytes processed.

        bool listenToBytes; ///< Flag to indicate if byte tracking is enabled.
        bool connectionStatus; ///< Indicates if the connection is active.

        const std::string id; ///< Unique identifier for the connection.

        std::vector<ProtocolID::ProtocolIdentifier*> protocols; ///< List of protocol handlers.

        RomulusDB::DbManager* dbManager; ///< Pointer to the database manager.
        ProtocolID::ProtocolIdentifier* protocolIdr; ///< Pointer to the protocol identifier.

        std::string dirName; ///< Directory name for the connection.
        std::string fileName; ///< File name for the connection.
        std::string host; ///< Hostname or IP address for the connection.

        /* --------------------------------------------------------- */
        /*                      METHODS                              */
        /* --------------------------------------------------------- */

        /**
         * @brief Creates a socket for the connection.
         */
        void createSocket();

        /**
         * @brief Checks the socket address and binds it to the connection.
         */
        void checkAddress();

        /**
         * @brief Checks the connection status and sets it to active or inactive.
         */
        void checkConnection();
    };
}
