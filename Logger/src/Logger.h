#pragma once
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <direct.h>
#include <mutex>
#include <algorithm>
#include <queue>
#include <condition_variable>
#include "ConfigParser.h"
#include "Utils.h"



#define TRACE(msg,...) Logger::getInstance().AddLog(Constant::LogPriority::trace, __FILE__, __func__, __LINE__, std::this_thread::get_id(), msg)
#define DEBUG(msg,...) Logger::getInstance().AddLog(Constant::LogPriority::debug, __FILE__, __func__, __LINE__, std::this_thread::get_id(), msg)
#define INFO(msg,...) Logger::getInstance().AddLog(Constant::LogPriority::info, __FILE__, __func__, __LINE__, std::this_thread::get_id(), msg)
#define WARN(msg,...) Logger::getInstance().AddLog(Constant::LogPriority::warn, __FILE__, __func__, __LINE__, std::this_thread::get_id(), msg)
#define FATL(msg,...) Logger::getInstance().AddLog(Constant::LogPriority::fatl, __FILE__, __func__, __LINE__, std::this_thread::get_id(), msg)


struct LogEntry {
	LogEntry(Constant::LogPriority lvl, std::string fileName, std::string funcName, int line, std::thread::id id, std::string msg) :
		FileName(fileName),
		FuncName(funcName),
		Msg(msg), Line(line),
		Level(lvl) 
	{
		std::stringstream ss;
		ss << id;
		t_id = ss.str();
	}
	
	int Line;
	std::string Msg;
	std::string t_id;
	std::string FileName;
	std::string FuncName;
	Constant::LogPriority Level;
};


class Logger {

private:
	Logger() {
		Logger::m_activeLogger = true;
		Logger::m_filePath = "";
		Logger::m_logFileName = false;
		Logger::m_lengthFile = 0;
		Logger::m_lengthFunc = 3;
		Logger::m_lengthThreadId = 10;
		Logger::m_fileLoggingEnabled = true;
		Logger::m_loggingLevel = Constant::LogPriority::info;
	}

	~Logger() {
		std::string logMsg = "\n--- Log Closed at " + Util::getDateTime(Constant::Clock::time) + " ---\n\n\n";
		if (foutput.is_open()) {
			foutput << logMsg;
			foutput.close();
		}
		else {
			std::cout << logMsg;
		}

		stopLogging();
		std::cout << "Logging process has ended\n\n\n";
	}
	
	void logWorker() {
		while (true) {
			std::unique_lock<std::mutex> Lock(mtx);
			CV.wait(Lock, [this] { return !m_logQueue.empty() || !m_activeLogger; });

			if (m_logQueue.empty() && !m_activeLogger) { return; }

			auto logEntry = m_logQueue.front();
			m_logQueue.pop();
			Lock.unlock();

			Log(logEntry);
		}
	}

	void startLogging() {
		loggingThread = std::thread(&Logger::logWorker, this);
	}

	void stopLogging() {
		m_activeLogger = false;
		CV.notify_all();

		if (loggingThread.joinable()) {
			loggingThread.join();
		}
	}

	void Log(LogEntry entry) {

		if (entry.Level >= m_loggingLevel) { return; }

		std::lock_guard<std::mutex> Lock(mtx);

		if (m_logFileName) {
			size_t pos = entry.FileName.find_last_of('/\\');
			if (pos != std::string::npos) { entry.FileName = entry.FileName.substr(pos + 1); }
			Util::limitStringLength(entry.FileName, m_lengthFile);
		}
		Util::limitStringLength(entry.t_id, m_lengthThreadId, true, '0');
		Util::limitStringLength(entry.FuncName, m_lengthFunc);

		std::ostream& output = (m_fileLoggingEnabled) ? foutput : std::cout;
		output << Util::getDateTime(Constant::Clock::time) << "  |  ";
		output << Constant::LogLevel::unmapLoggingLevel(entry.Level) << "  |  ";
		output << entry.t_id << "  |  ";
		if (m_logFileName) {
			output << "[" << entry.FileName << ":" << entry.FuncName << ":" << entry.Line << "]" << "\t|  ";
		} else {
			output << "[" << entry.FuncName << ":" << entry.Line << "]" << "\t|  ";
		}
		output << entry.Msg << "\n";
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

		startLogging();

		ConfigParser::ParseCfgFile(configPath);
		m_fileLoggingEnabled = ConfigParser::Get<bool>("Logger.EnableFileLogging", m_fileLoggingEnabled);
		std::string lvl = ConfigParser::Get<std::string>("Logger.LoggingLevel", "info");
		m_logFileName = ConfigParser::Get<bool>("Logger.LogFileName", m_logFileName);
		m_lengthFile = ConfigParser::Get<int>("Logger.FileNameLength", m_lengthFile);
		m_lengthFunc = ConfigParser::Get<int>("Logger.FuncNameLength", m_lengthFunc);
		m_lengthThreadId = ConfigParser::Get<int>("Logger.ThreadIdLength", m_lengthThreadId);

		//transforms the logging level to lowercase
		std::transform(lvl.begin(), lvl.end(), lvl.begin(), [](unsigned char c) {
			return std::tolower(c);
		});

		m_loggingLevel = Constant::LogLevel::mapLoggingLevel(lvl);

		if (m_fileLoggingEnabled) {
			m_filePath = ConfigParser::Get<std::string>("Logger.FilePath", "");
			if (!Util::checkOrCreateFilePath(m_filePath)) {
				std::cerr << "Error: file path was not present and couldn't be created.\n";
				return;
			}

			foutput.open(m_filePath, std::ios::app);
			if (foutput.is_open()) {
				std::cout << "Opened file: " << m_filePath << " for logging." << std::endl;
				foutput << "--- New log started at " << Util::getDateTime(Constant::Clock::time) << " ---\n\n";
			}
			else {
				std::cerr << "Error: Couldn't open the file stream.\n";
			}
		} else {
			std::cout << "--- New log started at " << Util::getDateTime(Constant::Clock::time) << " ---\n\n";
			std::cout << "File logging is disabled, didn't register file path.\n";
		}
	}
	
	void AddLog(Constant::LogPriority level, std::string fileName, std::string funcName, int line, std::thread::id threadId, std::string msg) {
		{
			std::lock_guard<std::mutex> Lock(mtx);
			LogEntry log_msg(level, fileName, funcName, line, threadId, msg);
			m_logQueue.push(log_msg);
		}
		CV.notify_one();
	}

private:
	std::mutex mtx;
	std::condition_variable CV;
	std::thread loggingThread;
	std::queue<LogEntry> m_logQueue;
	bool m_activeLogger;

	bool m_logFileName;
	int m_lengthFile;
	int m_lengthFunc;
	int m_lengthThreadId;
	std::ofstream foutput;
	std::string m_filePath;
	bool m_fileLoggingEnabled;
	Constant::LogPriority m_loggingLevel;
};
