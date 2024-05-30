#pragma once
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
		static std::string unmapLoggingLevel(LogPriority priority) {
			switch (priority) {
			case LogPriority::trace:	return "trce";
			case LogPriority::debug:	return "dbug";
			case LogPriority::info:		return "info";
			case LogPriority::warn:		return "warn";
			case LogPriority::fatl:		return "fatl";
			default: return "Unknown";
			}
		}
		static LogPriority mapLoggingLevel(std::string level) {
			if (level == "trce") { return LogPriority::trace; }
			else if (level == "dbug") { return LogPriority::debug; }
			else if (level == "info") { return LogPriority::info; }
			else if (level == "warn") { return LogPriority::warn; }
			else if (level == "fatl") { return LogPriority::fatl; }
			else { return LogPriority::info; }
		}
	};
}

std::string getDateTime(Constant::Clock clock) {
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

//bool checkOrCreateFilePath(std::string& m_filePath) {
//	size_t dotPos = m_filePath.find_last_of('.');
//	size_t slashPos = m_filePath.find_last_of('/');
//	if (dotPos == std::string::npos || slashPos == std::string::npos) {
//		std::cerr << "Error: Incorrect file path.";
//		return false;
//	}
//	// modifying file path to add /dd-mm-yyyy/filename-hh.log
//	m_filePath = m_filePath.substr(0, slashPos) + "/" + getDateTime(Constant::Clock::date) + m_filePath.substr(slashPos, dotPos - slashPos) + "-" + getDateTime(Constant::Clock::hours) + m_filePath.substr(dotPos);
//
//	struct stat buffer;
//	size_t lastSlashPos = m_filePath.find_last_of('/');
//	std::string directoryPath = m_filePath.substr(0, lastSlashPos);
//	if (stat(directoryPath.c_str(), &buffer) == 0) {
//		std::cout << "File path: " << directoryPath << " exists." << std::endl;
//		return true;
//	}
//
//	if (_mkdir(directoryPath.c_str()) != 0) {
//		std::cerr << "Error: Failed to create directory: " << directoryPath << std::endl;
//		return false;
//	}
//
//	return true;
//}


#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

bool createDirectory(const std::string& path) {
	size_t pos = 0;
	do {
		pos = path.find_first_of("/\\", pos + 1);
		std::string subdir = path.substr(0, pos);
		if (!subdir.empty() && MKDIR(subdir.c_str()) != 0 && errno != EEXIST) {
			std::cerr << "Error: Failed to create directory: " << subdir << " (" << errno << ")" << std::endl;
			return false;
		}
	} while (pos != std::string::npos);
	return true;
}

bool checkOrCreateFilePath(std::string& m_filePath) {
	size_t dotPos = m_filePath.find_last_of('.');
	size_t slashPos = m_filePath.find_last_of('/');
	if (dotPos == std::string::npos || slashPos == std::string::npos) {
		std::cerr << "Error: Incorrect file path." << std::endl;
		return false;
	}

	// Modifying file path to add /dd-mm-yyyy/filename-hh.log
	m_filePath = m_filePath.substr(0, slashPos) + "/" + getDateTime(Constant::Clock::date) +
		m_filePath.substr(slashPos, dotPos - slashPos) + "-" + getDateTime(Constant::Clock::hours) +
		m_filePath.substr(dotPos);

	size_t lastSlashPos = m_filePath.find_last_of('/');
	std::string directoryPath = m_filePath.substr(0, lastSlashPos);

	struct stat buffer;
	if (stat(directoryPath.c_str(), &buffer) == 0) {
		std::cout << "File path: " << directoryPath << " exists." << std::endl;
		return true;
	}

	if (createDirectory(directoryPath)) {
		std::cout << "Created directory path: " << directoryPath << std::endl;
		return true;
	}
	else {
		std::cerr << "Error: Failed to create directory path: " << directoryPath << std::endl;
		return false;
	}
}