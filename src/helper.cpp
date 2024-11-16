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
    protocol {},
    cleaned_buffer {}
    {
        this->buffer = buffer;
        rObject = new ReturnObject("+\r\n", 0);
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

    size_t ProtocolIdentifier::searchProtocol(std::string search_word) {

        PRINT_WARNING(search_word);
        PRINT_WARNING("In searchProtocol " + cleaned_buffer);
        int index {};
        index = cleaned_buffer.find(search_word);

        PRINT_WARNING(std::to_string(index));

        if (index != std::string::npos) protocol = search_word;
        return index;

    }

    std::string ProtocolIdentifier::getVariable(
        size_t starts_at, char listenOnSymbol, char endsOnSymbol
    ) {
        std::regex not_digit("[^0-9]");
        bool listen = false;

        std::string variable {};

        auto rule = not_digit;

        bool cleanFrontDigits = false;

        for (size_t i = starts_at; i < cleaned_buffer.size(); i++) {
            char current = cleaned_buffer[i];

            if (!listen && current == listenOnSymbol) {
                listen = true;
                if (listenOnSymbol == '$') {
                    cleanFrontDigits = true;
                    continue;
                }
            }

            if (listen && cleanFrontDigits) {
                if (!std::regex_match(std::string(1, current), not_digit)) continue;
                cleanFrontDigits = false;
            }

            if (listen && current == endsOnSymbol) break;
            if (listen) variable += current;
        }

        return variable;
    }

    bool ProtocolIdentifier::identify_protocol() {
        std::regex non_printable("[^\\x20-\\x7E]+");

        // Convert both strings to lowercase
        std::string buffer_data = buffer;

        std::transform(buffer_data.begin(), buffer_data.end(), buffer_data.begin(), ::tolower);

        // Replace non-printable characters with an empty string
        cleaned_buffer = std::regex_replace(buffer_data, non_printable, "");

        // Output the result
        PRINT_WARNING(cleaned_buffer);

        if (identifyPing()) return true;
        if (identifyEcho()) return true;
        if (identifySet()) return true;
        if (identifyGet()) return true;

        return false;

    }

    bool ProtocolIdentifier::identifyPing() {

        size_t index = searchProtocol("ping");
        if (index == std::string::npos) return false;

        rObject = new ReturnObject("+PONG\r\n", 0);
        return true;
    }

    bool ProtocolIdentifier::identifyEcho() {

        size_t index = searchProtocol("echo");
        if (index == std::string::npos) return false;

        std::regex not_digit("[^0-9]");
        index += 4; // Plus echo.size

        std::string pre_echo = getVariable(index);
        std::string echo = "$" + std::to_string(pre_echo.size()) + "\r\n" + pre_echo + "\r\n";
        rObject = new ReturnObject(echo, 0);

        return true;
    }

    bool ProtocolIdentifier::identifySet() {
        // *3$3set$9pineapple$6banana
        size_t index = searchProtocol("set");
        if (index == std::string::npos) return false;

        index += 3; // adding set

        std::string key = getVariable(index);
        std::string value = getVariable(index + key.size()); // Not correct the sum, but will work

        Cache::DataManager cache;
        cache.setValue(key, value);

        rObject = new ReturnObject("+OK\r\n", 0);
        return true;
    }

    bool ProtocolIdentifier::identifyGet() {
        size_t index = searchProtocol("get");
        if (index == std::string::npos) return false;

        index += 3; // adding get

        std::string key = getVariable(index);

        PRINT_SUCCESS("key " + key);

        Cache::DataManager cache;
        std::optional<std::string> value = cache.getValue(key);

        if (!value.has_value()) {
            rObject = new ReturnObject("$-1\r\n", 0);
            return false;
        }

        std::string response = constructProtocol({value.value()}, false);
        PRINT_SUCCESS(response);
        rObject = new ReturnObject(response, 0);

        return true;
    }

    std::string ProtocolIdentifier::constructProtocol(std::vector<std::string> args, bool isArray) {

        std::string protocol {};
        if (isArray) {
            std::string protocol = "*" + std::to_string(args.size());
        }

        for (std::string arg : args) {

            protocol += "$" + std::to_string(arg.size()) + "\r\n" + arg + "\r\n";
        }

        return protocol;
    }

}