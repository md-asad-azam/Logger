#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>


/*
* This class is used to parse the config files. 
* All required functions are static so there is no need to create its instances.
* 
* To use this parser, first parse the config to create a cache of the config and the use
* the getter to get the config.
*/

class ConfigParser {
private:
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> settings;

    ConfigParser(const std::string& filename) {}

public:

    static void parseCfgFile(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;

        std::string currentSection;
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') {
                continue;
            }

            // Check if the line represents a section
            if (line[0] == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.size() - 2);
            }
            else {
                std::istringstream iss(line);
                std::string key, value;
                std::getline(iss, key, '=');
                std::getline(iss, value);

                settings[currentSection][key] = value;
            }
        }
    }

    static std::string getSetting(const std::string& setting, const std::string& defaultVal="") {

        size_t delimiterIndex = setting.find_last_of('.');
        std::string sectionName = setting.substr(0, delimiterIndex);
        std::string settingName = setting.substr(delimiterIndex+1);

        std::cout << sectionName << "\n" << settingName;

        auto sectionIter = settings.find(sectionName);
        if (sectionIter != settings.end()) {
           auto settingIter = sectionIter->second.find(settingName);
           if (settingIter != sectionIter->second.end()) {
                return settingIter->second;
           }
        }
        return defaultVal; // Return default value if setting not found
    }
};

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> ConfigParser::settings;
