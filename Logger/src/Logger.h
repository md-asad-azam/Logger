#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <direct.h>
#include <mutex>
#include "ConfigParser.h"



namespace Constant {

	enum LogPriority {
		trace,
		debug,
		info,
		warn,
		fatl
	};

	enum Clock {
		year,
		month,
		day,
		date,
		hours,
		minutes,
		seconds,
		microseconds,
		time
	};

	struct LogLevel {
		static std::string as_string(LogPriority priority) {
			switch (priority) {
			case LogPriority::trace: return "Trce";
			case LogPriority::debug: return "Dbug";
			case LogPriority::info: return "Info";
			case LogPriority::warn: return "Warn";
			case LogPriority::fatl: return "Fatl";
			default: return "Unknown";
			}
		}
	};
}

class Logger {

private:
	Logger() {}
	~Logger() {
		foutput << "\n--- Log Closed ---\n\n";
	}

	inline std::string getDateTime(Constant::Clock clock) {
		std::tm tm;
		std::ostringstream timeString;
		auto currentTime = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::to_time_t(currentTime);
		localtime_s(&tm, &time);

		switch (clock) {
			case Constant::Clock::date: { 
				timeString << std::put_time(&tm, "%d-%m-%Y");
				break;
			}
			case Constant::Clock::year: { 
				timeString << std::put_time(&tm, "%Y");
				break; 
			}
			case Constant::Clock::month: { 

				timeString << std::put_time(&tm, "%m");
				break; 
			}
			case Constant::Clock::day: { 
				timeString << std::put_time(&tm, "%d");
				break; 
			}
			case Constant::Clock::time: { 
				auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(currentTime.time_since_epoch()).count() % 1000000;
				timeString << std::put_time(&tm, "%H:%M:%S") << ":" << std::setw(6) << std::setfill('0') << microseconds;
				break;
			}
			case Constant::Clock::hours: { 
				timeString << std::put_time(&tm, "%H");
				break; 
			}
			case Constant::Clock::minutes: { 
				timeString << std::put_time(&tm, "%M");
				break; 
			}
			case Constant::Clock::seconds: { 
				timeString << std::put_time(&tm, "%S");
				break; 
			}
			case Constant::Clock::microseconds: { 
				auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(currentTime.time_since_epoch()).count() % 1000000;
				timeString << std::setw(6) << std::setfill('0') << microseconds;
				break;
			}
		}
		return timeString.str();
	}
	
	template<typename... Args>
	void log(Constant::LogPriority level, std::string msg, Args... args) {
		std::unique_lock<std::mutex> Lock(mtx);
		if (m_fileLoggingEnabled && level >= m_loggingLevel) {
			foutput << getDateTime(Constant::Clock::time) << "\t";
			foutput << Constant::LogLevel::as_string(level) << "\t";
			foutput << std::this_thread::get_id()<< "\t";
			foutput << msg << "\n";
		} else {
			std::cout << getDateTime(Constant::Clock::time) << "\t";
			std::cout << Constant::LogLevel::as_string(level) << "\t";
			std::cout << std::this_thread::get_id() << "\t";
			std::cout << msg << "\n";
		}
		Lock.unlock();
	}


	bool checkOrCreateFilePath(std::string& file_path) {
		size_t dotPos = file_path.find_last_of('.');
		size_t slashPos = file_path.find_last_of('/');
		if (dotPos == std::string::npos || slashPos == std::string::npos) {
			std::cerr << "Error: Incorrect file path.";
			return false;
		}
		// modifying file path to add /dd-mm-yyyy/filename-hh.log
		file_path = file_path.substr(0, slashPos) + "/" + getDateTime(Constant::Clock::date) + file_path.substr(slashPos, dotPos - slashPos) + "-" + getDateTime(Constant::Clock::hours) + file_path.substr(dotPos);
		
		struct stat buffer;
		size_t lastSlashPos = file_path.find_last_of('/');
		std::string directoryPath = file_path.substr(0, lastSlashPos);
		if (stat(directoryPath.c_str(), &buffer) == 0) {
			std::cout << "File path: " << directoryPath << " exists." << std::endl;
			return true;
		}
		
		if (_mkdir(directoryPath.c_str()) != 0) {
			std::cerr << "Error: Failed to create directory: " << directoryPath << std::endl;
			return false;
		}
		
		return true;
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

	void RegisterLogger(std::string filePath, std::string configPath) {

		file_path = filePath;

		ConfigParser::parseCfgFile(configPath);
		
		if (m_fileLoggingEnabled) {

			if (!checkOrCreateFilePath(file_path)) {
				std::cerr << "Error: file path was not present and couldn't be created.\n";
				return;
			}

			foutput.open(file_path, std::ios::app);
			if (foutput.is_open()) {
				std::cout << "Opened " << file_path << " for logging.";
				foutput << "--- New log started at " << getDateTime(Constant::Clock::time) << " ---" << "\n\n";
			}
			else {
				std::cerr << "Error: Couldn't open the file stream.\n";
			}
		} else {
			std::cout << "File logging is disabled, didn't register file path.\n";
		}
	}
	
	template<typename... Args>
	void TRACEsc(std::string msg, Args... args) {
		log(Constant::LogPriority::trace, msg, args...);
	}

	template<typename... Args>
	void DEBUGsc(std::string msg, Args... args) {
		log(Constant::LogPriority::debug, msg, args...);
	}

	template<typename... Args>
	void INFOsc(std::string msg, Args... args) {
		log(Constant::LogPriority::info, msg, args...);
	}

	template<typename... Args>
	void WARNsc(std::string msg, Args... args) {
		log(Constant::LogPriority::warn, msg, args...);
	}

	template<typename... Args>
	void FATLsc(std::string msg, Args... args) {
		log(Constant::LogPriority::fatl, msg, args...);
	}

private:
	static std::mutex mtx;
	static std::ofstream foutput;
	static std::string file_path;
	static std::string config_path;
	static bool m_fileLoggingEnabled;
	static Constant::LogPriority m_loggingLevel;
};

std::mutex Logger::mtx;
std::ofstream Logger::foutput;
std::string Logger::file_path;
std::string Logger::config_path;
bool Logger::m_fileLoggingEnabled = true;
Constant::LogPriority Logger::m_loggingLevel = Constant::LogPriority::info;