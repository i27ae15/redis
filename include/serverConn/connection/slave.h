#pragma once
#include <serverConn/connection/base.h>

/**
 * @namespace RomulusConn
 * @brief Contains classes and utilities for managing Master and Slave connections in the Romulus system.
 */
namespace RomulusConn {

    class Master; ///< Forward declaration of the Master class.

    /**
     * @class Slave
     * @brief Represents a Slave node in the Romulus system, which connects to a Master node for replication.
     *
     * The Slave class extends the BaseConnection class and includes functionality for interacting with
     * a Master node, performing handshakes, and managing replication connections.
     */
    class Slave : public BaseConnection {

    public:
        /**
         * @brief Constructs a Slave connection.
         *
         * @param port The port number for the Slave connection.
         * @param host The hostname or IP address for the Slave connection.
         * @param dirName The directory name for database files (optional, default: "").
         * @param fileName The database file name (optional, default: "").
         */
        Slave(
            unsigned short port,
            std::string host,
            std::string dirName = "",
            std::string fileName = ""
        );

        /**
         * @brief Assigns a Master object to the Slave connection.
         *
         * @param master A pointer to the Master object.
         */
        void assignMaster(Master* master);

        /**
         * @brief Assigns a Master connection to the Slave by specifying its details.
         *
         * @param port The port number of the Master.
         * @param serverFD The server file descriptor of the Master.
         * @param host The hostname or IP address of the Master.
         */
        void assignMaster(
            unsigned short port,
            short serverFD,
            std::string host
        );

        /**
         * @brief set handshaked with master completed
         */
        void setHandShakedWithMaster();

        /**
         * @brief get handshaked with master.
         * @return true if slave has complted handshaked with master, false if not.
         */
        bool getHandShakedWithMaster();

        /**
         * @brief Initiates a handshake process with the Master node.
         */
        void handShakeWithMaster();

        /**
         * @brief Retrieves the port number of the connected Master.
         *
         * @return The port number of the Master.
         */
        unsigned short getMasterPort();

        /**
         * @brief Retrieves the server file descriptor of the connected Master.
         *
         * @return The server file descriptor of the Master.
         */
        short getMasterServerFD();

        /**
         * @brief Checks if the Slave is currently in the handshake process with the Master.
         *
         * @return True if the Slave is in the handshake process; otherwise, false.
         */
        bool isInHandShake();

        /**
         * @brief Retrieves the hostname or IP address of the connected Master.
         *
         * @return The hostname or IP address of the Master.
         */
        std::string getMasterHost();

        /**
         * @brief Retrieves the role of this connection as a Slave.
         *
         * @return A string representing the role ("SLAVE").
         */
        std::string getRole() const override;

    private:

        /* --------------------------------------------------------- */
        /*                     VARIABLES                             */
        /* --------------------------------------------------------- */

        Master* master; ///< Pointer to the associated Master object.

        std::string masterHost; ///< Hostname or IP address of the connected Master.

        bool handShakedWithMaster; ///< Indicates whether the Slave has completed the handshake with the Master.

        unsigned short handShakeStep; ///< Current step in the handshake process.
        unsigned short masterPort; ///< Port number of the connected Master.
        short masterServerFD; ///< Server file descriptor of the connected Master.
    };

}
