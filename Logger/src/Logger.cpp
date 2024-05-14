
#include <iostream>
#include <fstream>
#include <thread>
#include "Logger.h"
#include "ConfigParser.h"



void loggerTest(int n) {

    Logger& logger = Logger::getInstance();
    logger.setLoggingLevel(Constant::LogPriority::trace);
          
    //Logger::RegisterFilePath("C:/Users/mdasa/Downloads/my_logger_output/output.log");
    logger.TRACEsc("Traceing" + std::to_string(n));
    logger.DEBUGsc("DEBUGsc" + std::to_string(n));
    logger.INFOsc("INFOsc" + std::to_string(n));
    logger.WARNsc("WARNsc" + std::to_string(n));
    logger.FATLsc("FATLsc" + std::to_string(n));

}

int main()
{
    loggerTest(1);

    /*std::thread worker[10];

    for (int i = 0; i < 10; i++) {
        worker[i] = std::thread(loggerTest, i);
    }

    for (int i = 0; i < 10; i++) {
        worker[i].join();
    }*/


    ConfigParser::parseCfgFile("./AnyComponent.cfg");
    std::cout << ConfigParser::getSetting("Logger.FilePath");


    return 0;
}
