#include <helper.h>
#include <utils.h>
#include <regex>

namespace Helper {

    ReturnObject::ReturnObject(std::string return_value, int behavior) {
        this->return_value = return_value;
        this->behavior = behavior;
        this->bytes = return_value.size();
    }

    ProtocolIdentifier::ProtocolIdentifier(std::string buffer) :
    protocol {}
    {
        this->buffer = buffer;
        rObject = new ReturnObject("\r\n", 0);
        identify_protocol();

        if (protocol.size()) {
            PRINT_SUCCESS("Protocol identified as " + protocol);
        } else {
            PRINT_ERROR("Protocol not identified");
        }
    }

    ProtocolIdentifier::~ProtocolIdentifier() {
        delete rObject;
    }

    ReturnObject* ProtocolIdentifier::getRObject() {
        return rObject;
    }

    std::string ProtocolIdentifier::getProtocol() {
        return protocol;
    }

    bool ProtocolIdentifier::identify_protocol() {

        // Convert both strings to lowercase
        std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::tolower);
        std::string output {};

        if (identify_ping()) return true;
        if (identify_echo()) return true;

        return false;

    }

    bool ProtocolIdentifier::identify_ping() {

        std::string search_word = "ping";

        if (buffer.find(search_word) != std::string::npos) {
            // ping found return simply response
            rObject = new ReturnObject("+PONG\r\n", 0);
            protocol = search_word;
            return true;
        }

        return false;
    }

    bool ProtocolIdentifier::identify_echo() {

        std::string search_word = "echo";

        std::regex non_printable("[^\\x20-\\x7E]+");
        std::regex not_digit("[^0-9]");

        // Replace non-printable characters with an empty string
        std::string buffer_data = buffer;
        std::string cleaned_buffer = std::regex_replace(buffer_data, non_printable, "");

        // Output the result
        PRINT_WARNING(cleaned_buffer);

        size_t index = cleaned_buffer.find(search_word);
        std::string pre_echo;

        if (index != std::string::npos) {
            index += 5; // Plus echo.size + $

            bool listen = false;
            for (size_t i = index; i < cleaned_buffer.size(); i++) {
                char current = cleaned_buffer[i];

                if (!listen && std::regex_match(std::string(1, current), not_digit)) {
                    listen = true;
                }

                if (listen) {
                    pre_echo += current;
                    continue;
                }
            }

            //$3\r\nhey\r\n
            std::string echo = "$" + std::to_string(pre_echo.size()) + "\r\n" + pre_echo + "\r\n";
            rObject = new ReturnObject(echo, 0);
            protocol = search_word;

            return true;
        }

        return false;
    }

    std::string construct_protocol(size_t num_args, std::vector<std::pair<char, std::string>> args) {

        std::string protocol = "*" + std::to_string(num_args) + "\r\n$4\r\nping\r\n";

        for (std::pair<size_t, std::string> arg : args) {

            protocol += "\r\n$" + std::to_string(arg.first) + "\r\n" + arg.second;
        }

        protocol += "\r\n";
        return protocol;

    }

}