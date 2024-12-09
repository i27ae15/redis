#pragma once
#include <serverConn/connection/base.h>
#include <serverConn/connection/structs.h>

/**
 * @namespace RomulusConn
 * @brief Contains classes and utilities for managing Master and Slave connections in the Romulus system.
 */
namespace RomulusConn {

    /**
     * @class Master
     * @brief Represents a Master node in the Romulus system, managing replica connections and propagating protocols.
     *
     * The Master class extends the BaseConnection class to include functionality for managing and interacting
     * with replicas. It tracks replicas, manages acknowledgements, and propagates protocol commands to replicas.
     */
    class Master : public BaseConnection {

    public:
        /**
         * @brief Constructs a Master connection.
         *
         * @param port The port number to bind the Master connection to.
         * @param host The hostname or IP address for the Master connection.
         * @param dirName The directory name for database files (optional, default: "").
         * @param fileName The database file name (optional, default: "").
         */
        Master(
            unsigned short port,
            std::string host,
            std::string dirName = "",
            std::string fileName = ""
        );

        /**
         * @brief Indicates if the Master is currently in a handshake with a replica.
         */
        bool inHandShakeWithReplica;

        /**
         * @brief Increments the number of replicas that have acknowledged the last operation.
         */
        void incrementReplicasOscarKilo();

        /**
         * @brief Sets the number of replicas required to acknowledge an operation.
         *
         * @param n The number of replicas to wait for acknowledgements.
         */
        void setReplicasToAck(unsigned short n);

        /**
         * @brief Sets the number of replicas that have acknowledged the last operation.
         *
         * @param n The number of replicas that have acknowledged.
         */
        void setReplicasOscarKilo(unsigned short n);

        /**
         * @brief Retrieves the total number of connected replicas.
         *
         * @return The number of replicas as an unsigned short.
         */
        unsigned short getNumReplicas();

        /**
         * @brief Retrieves the number of replicas required to acknowledge an operation.
         *
         * @return The number of replicas to acknowledge.
         */
        unsigned short getReplicasToAck();

        /**
         * @brief Retrieves the number of replicas that have acknowledged the last operation.
         *
         * @return The number of acknowledged replicas.
         */
        unsigned short getReplicasOscarKilo();

        /**
         * @brief Propagates a protocol command to all connected replicas.
         *
         * @param idt A pointer to the ProtocolIdentifier instance.
         * @param buffer The protocol command to propagate as a string.
         */
        void propagueProtocolToReplica(
            ProtocolID::ProtocolIdentifier *idt,
            const std::string& buffer
        );

        /**
         * @brief Creates a new connection object for the current replica being processed.
         */
        void createCurrentReplicaConn();

        /**
         * @brief Sets the port number for the current replica connection.
         *
         * @param value The port number for the replica.
         */
        void setCurrentReplicaPort(unsigned short value);

        /**
         * @brief Sets the server file descriptor for the current replica connection.
         *
         * @param value The server file descriptor for the replica.
         */
        void setCurrentReplicaServerFd(unsigned short value);

        /**
         * @brief Adds the current replica connection to the list of managed replicas and resets the current replica.
         */
        void addAndCleanCurrentReplicaConn();

        /**
         * @brief Sends acknowledgment requests to all connected replicas.
         */
        void getReplicasACKs();

        /**
         * @brief Retrieves the role of this connection as a Master.
         *
         * @return A string representing the role ("MASTER").
         */
        std::string getRole() const override;

    private:

        /* --------------------------------------------------------- */
        /*                     VARIABLES                             */
        /* --------------------------------------------------------- */

        unsigned short nReplicasToACK; ///< The number of replicas required to acknowledge operations.
        unsigned short nReplicasOscarKilo; ///< The number of replicas that have acknowledged the last operation.

        RomulusConnStructs::replicaConn currentReplicaConn; ///< Connection details for the current replica.
        std::vector<RomulusConnStructs::replicaConn> replicaConns; ///< List of all connected replicas.

    };
}