
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include "Logger.h"
#include "ConfigParser.h"


void print()
{
    TRACE("Improved Performance");
    INFO("Reduced Latency");
    DEBUG("Thread Safety");
    FATL("Optimal");
    WARN("Blah blah");
    DEBUG("blah blah blah blah");
    DEBUG("blah blah blah blah");
}
void printabcd()
{
    TRACE("Improved Performance");
    INFO("Reduced Latency");
    DEBUG("Thread Safety");
    FATL("Optimal");
    WARN("Blah blah");
    DEBUG("blah blah blah blah");
    DEBUG("blah blah blah blah");
}
void printlengthbig()
{
    TRACE("Improved Performance");
    INFO("Reduced Latency");
    DEBUG("Thread Safety");
    FATL("Optimal");
    WARN("Blah blah");
    DEBUG("blah blah blah blah");
    DEBUG("blah blah blah blah");
}


void loggerTest() {

    Logger& logger = Logger::getInstance();
    logger.setLoggingLevel(Constant::LogPriority::trace);
    logger.RegisterLogger("src/AnyComponent.cfg");

    std::thread worker[20];
    for (int i = 0; i < 20; i++) { worker[i] = std::thread(print); }
    for (int i = 0; i < 20; i++) { worker[i].join(); }

    printabcd();
    printlengthbig();
}


int main()
{
  
    loggerTest();
   
    INFO("Main done.");

    return 0;
}
