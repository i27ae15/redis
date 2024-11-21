#pragma once
#include <utils.h>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <filesystem>
#include <fstream>

// Deprecated

namespace RemusConfig {

    std::vector<std::string> splitString(const std::string input, char delimiter);
    class ConfigManager {

        public:

        ConfigManager(int argc, char** argv);
        ~ConfigManager();

        // Getters
        std::string getDirName();
        std::string getFileName();
        int getPort();

        private:

        std::string dirName;
        std::string fileName;
        std::string role;

        int port;

        std::vector<std::pair<std::string, std::string>> arguments;
        int argc;
        char** argv;

        std::unordered_map<std::string, std::function<void(const std::string&)>> methodMap;
        // Methods to react to the parameters passed
        void methodRouter(int argc, char** argv);

        void dirManager(std::string dirName);
        void dbFileManager(std::string fileName);
        void portManager(std::string port);
    };
}