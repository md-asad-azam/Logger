#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>


/*
* 
* This class is used to parse the config files. 
* All required functions are static so there is no need to create its instances.
* 
* To use this parser, first parse the config to create a cache of the config in the Settings map
* and the use the getter to get the config.
* 
* Currently supported  return types are int, double, string, bool, vector<string> 
* ',' or ';' seperated configs can be retreived as the vector<string>
* 
*/

class ConfigParser {
private:
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> Settings;

    ConfigParser(const std::string& filename) {}

public:

    static void ParseCfgFile(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open()) {
            std::cerr << "Unable to open the config file." << std::endl;
            return;
        }

        std::string currentSection;
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') {
                continue;
            }

            {   // Removing the starting and trailing spaces
                auto start = std::find_if_not(line.begin(), line.end(), ::isspace);
                auto end = std::find_if_not(line.rbegin(), line.rend(), ::isspace).base();
                if (start < end) {
                    line = std::string(start, end);
                }
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

                Settings[currentSection][key] = value;
            }
        }
    }

    template<typename T>
    static T Get(const std::string& setting, const T& defaultVal = T()) {

        size_t delimiterIndex = setting.find_last_of('.');
        std::string sectionName = setting.substr(0, delimiterIndex);
        std::string settingName = setting.substr(delimiterIndex+1);

        auto sectionIter = Settings.find(sectionName);
        if (sectionIter != Settings.end()) {
           auto settingIter = sectionIter->second.find(settingName);
           if (settingIter != sectionIter->second.end()) {
                return convert<T>(settingIter->second);
           }
        }
        return defaultVal; // Return default value if setting not found
    }

private:

    template <typename T>
    static T convert(const std::string& value);

    template <>
    static int convert<int>(const std::string& val) {
        return std::stoi(val);
    }

    template <>
    static double convert<double>(const std::string& value) {
        return std::stod(value);
    }

    template <>
    static bool convert<bool>(const std::string& val) {
        return val == "true" || val == "1";
    }

    template <>
    static std::string convert<std::string>(const std::string& val) {
        return val;
    }

    template <>
    static std::vector<std::string> convert(const std::string& val) {
        std::vector<std::string> tokens;
        std::string token;
        for (char ch : val) {
            if (ch == ',' || ch == ';') {
                if (!token.empty()) { tokens.push_back(token); }
                token.clear();
            }
            else { token += ch; }
        }
        if (!token.empty()) { tokens.push_back(token); }
        return tokens; 
    }
};

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> ConfigParser::Settings;
