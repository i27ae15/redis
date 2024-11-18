#include <config_manager.h>

namespace Remus {

    std::vector<std::string> splitString(const std::string input, char delimiter) {
        std::vector<std::string> splitedString {};
        std::stringstream ss(input);
        std::string token {};

        while (std::getline(ss, token, delimiter)) {
            splitedString.push_back(token);
        }
        return splitedString;
    }


    ConfigManager* ConfigManager::instance = nullptr;

    std::string ConfigManager::dirName {};
    std::string ConfigManager::fileName {};

    ConfigManager& ConfigManager::getInstance() {
        if (!instance) {
            throw std::runtime_error("Class has not been initialized, you musdt call ConfigManager::initialize(...) firste!");
        }
        return *instance;
    }

    void ConfigManager::initialize(int argc, char** argv) {
        if (instance) {
            throw std::runtime_error("Class is already initialzed, call ConfigManager::getInstance to get an instance of this class!");
        }
        instance = new ConfigManager(argc, argv);
    }

    ConfigManager::ConfigManager(int argc, char** argv) :
    argc{argc}, argv{argv}, arguments {}
    {

        // Initialize method map;
        methodMap["--dir"] = [this](const std::string& input) {this->dirManager(input);};
        methodMap["--dbfilename"] = [this](const std::string& input) {this->dbFileManager(input);};

        methodRouter(argc, argv);
    }

    ConfigManager::~ConfigManager() {}

    // Getters

    std::string ConfigManager::getDirName() {
        return dirName;
    }

    std::string ConfigManager::getFileName() {
        return fileName;
    }

    void ConfigManager::methodRouter(int argc, char** argv) {

        for (int i = 1; i < argc; ++i) {

            std::string key = argv[i];
            if (key.size() < 2 || ( key[0] != '-' && key[1] != '-')) continue;

            std::string input = argv[i+1];

            if (methodMap.find(key) != methodMap.end()) {
                methodMap[key](input);
                i + 2;
            } else {
                PRINT_ERROR("No method found for key: " + key);
            }
        }

    }

    void ConfigManager::dirManager(std::string dirName) {

        std::vector<std::string> dirs = splitString(dirName, '/');

        for (std::string dir : dirs) {

            if (!dir.size()) continue;

            try {
                if (std::filesystem::create_directory(dir)) {
                    PRINT_SUCCESS("Create directory: " + dir);
                } else {
                    if (std::filesystem::exists(dir)) {
                        PRINT_ERROR("File already exists");
                    } else {
                        PRINT_ERROR("Error creating directory");
                    }
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::string eWhat = e.what();
                PRINT_ERROR("Error creating directory" + eWhat);
            }
        }

        this->dirName = dirName;
    }

    void ConfigManager::dbFileManager(std::string fileName) {
        std::string filePath = getDirName() + "/" + fileName;

        try {
            std::ofstream file(filePath);
            if (file.is_open()) {
                file << "Config. \n";
                PRINT_SUCCESS("File created" + filePath);
            } else {
                PRINT_ERROR("Error creating file: " + filePath);
            }
        } catch (const std::exception& e) {
            std::string eWhat = e.what();
            PRINT_ERROR("Error creating file" + eWhat);
        }

        this->fileName = fileName;
    }

}
