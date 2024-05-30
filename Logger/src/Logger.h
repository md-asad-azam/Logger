#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <direct.h>
#include <mutex>
#include <algorithm>
#include "ConfigParser.h"
#include "Utils.h"



#define TRACE(msg,...) Logger::getInstance().TraceImpl(__FILE__, __func__, __LINE__, msg, __VA_ARGS__)
#define DEBUG(msg,...) Logger::getInstance().DebugImpl(__FILE__, __func__, __LINE__, msg, __VA_ARGS__)
#define INFO(msg,...) Logger::getInstance().InfoImpl(__FILE__, __func__, __LINE__, msg, __VA_ARGS__)
#define WARN(msg,...) Logger::getInstance().WarnImpl(__FILE__, __func__, __LINE__, msg, __VA_ARGS__)
#define FATL(msg,...) Logger::getInstance().FatlImpl(__FILE__, __func__, __LINE__, msg, __VA_ARGS__)


class Logger {

private:
	Logger() {
		Logger::m_filePath = "";
		Logger::m_lengthFileFunc = 10;
		Logger::m_fileLoggingEnabled = false;
		Logger::m_loggingLevel = Constant::LogPriority::info;
	}
	~Logger() {
		std::string logMsg = "\n--- Log Closed at " + getDateTime(Constant::Clock::time) + " ---\n\n\n";
		if (foutput.is_open()) {
			foutput << logMsg;
			foutput.close();
		}
		else {
			std::cout << logMsg;
		}
		std::cout << "Logging process ended\n\n\n";
	}
	
	template<typename... Args>
	void log(Constant::LogPriority level, std::string fileName, std::string funcName, int line, std::string msg, Args... args) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (level >= m_loggingLevel) { return; }

		std::unique_lock<std::mutex> Lock(mtx);
		std::ostringstream oss;
		oss << msg;
		(void)std::initializer_list<int>{(oss << args, 0)...};

		size_t pos = fileName.find_last_of('/\\');
		if (pos != std::string::npos) { fileName = fileName.substr(pos + 1); }
		if (fileName.length() > m_lengthFileFunc) { fileName = fileName.substr(0, m_lengthFileFunc); }
		if (funcName.length() > m_lengthFileFunc) { funcName = funcName.substr(0, m_lengthFileFunc); }
		
		std::ostream& output = (m_fileLoggingEnabled) ? foutput : std::cout;
		output << getDateTime(Constant::Clock::time) << "  |  ";
		output << Constant::LogLevel::unmapLoggingLevel(level) << "  |  ";
		output << std::this_thread::get_id() << "  |  ";
		output << "[" << fileName << ":" << funcName << ":" << line << "]" << "  |  ";
		output << oss.str() << "\n";
		Lock.unlock();
	}

public:
	Logger(Logger&) = delete;
	void operator=(const Logger&) = delete;

	static Logger& getInstance() {
		static Logger singleton_instance;
		return singleton_instance;
	}

	inline void setLoggingLevel(Constant::LogPriority level) {
		m_loggingLevel = level;
	}

	void RegisterLogger(std::string configPath) {

		ConfigParser::ParseCfgFile(configPath);
		m_fileLoggingEnabled = ConfigParser::Get<bool>("Logger.EnableFileLogging", m_fileLoggingEnabled);
		std::string lvl = ConfigParser::Get<std::string>("Logger.LoggingLevel", "info");
		m_lengthFileFunc = ConfigParser::Get<int>("Logger.FileFuncDisplayLength", m_lengthFileFunc);

		//transforms the debug level to lowercase
		std::transform(lvl.begin(), lvl.end(), lvl.begin(), [](unsigned char c) {
			return std::tolower(c);
		});

		m_loggingLevel = Constant::LogLevel::mapLoggingLevel(lvl);

		if (m_fileLoggingEnabled) {
			m_filePath = ConfigParser::Get<std::string>("Logger.FilePath", "");
			if (!checkOrCreateFilePath(m_filePath)) {
				std::cerr << "Error: file path was not present and couldn't be created.\n";
				return;
			}

			foutput.open(m_filePath, std::ios::app);
			if (foutput.is_open()) {
				std::cout << "Opened file: " << m_filePath << " for logging." << std::endl;
				foutput << "--- New log started at " << getDateTime(Constant::Clock::time) << " ---\n\n";
			}
			else {
				std::cerr << "Error: Couldn't open the file stream.\n";
			}
		} else {
			std::cout << "--- New log started at " << getDateTime(Constant::Clock::time) << " ---\n\n";
			std::cout << "File logging is disabled, didn't register file path.\n";
		}
	}
	

	template<typename... Args>
	void TraceImpl(std::string fileName, std::string funcName, int line, std::string msg, Args... args) {
		log(Constant::LogPriority::trace, fileName, funcName, line, msg, args...);
	}

	template<typename... Args>
	void DebugImpl(std::string fileName, std::string funcName, int line, std::string msg, Args... args) {
		log(Constant::LogPriority::debug, fileName, funcName, line, msg, args...);
	}

	template<typename... Args>
	void InfoImpl(std::string fileName, std::string funcName, int line, std::string msg, Args... args) {
		log(Constant::LogPriority::info, fileName, funcName, line, msg, args...);
	}

	template<typename... Args>
	void WarnImpl(std::string fileName, std::string funcName, int line, std::string msg, Args... args) {
		log(Constant::LogPriority::warn, fileName, funcName, line, msg, args...);
	}

	template<typename... Args>
	void FatlImpl(std::string fileName, std::string funcName, int line, std::string msg, Args... args) {
		log(Constant::LogPriority::fatl, fileName, funcName, line, msg, args...);
	}

private:
	std::mutex mtx;
	int m_lengthFileFunc;
	std::ofstream foutput;
	std::string m_filePath;
	bool m_fileLoggingEnabled;
	Constant::LogPriority m_loggingLevel;
};
