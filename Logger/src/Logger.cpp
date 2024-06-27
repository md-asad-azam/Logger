
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
    logger.RegisterLogger("E:\\MyProject_cpp\\Logger\\Logger\\src\\AnyComponent.cfg");

    std::thread worker[10];
    for (int i = 0; i < 10; i++) { worker[i] = std::thread(print); }
    for (int i = 0; i < 10; i++) { worker[i].join(); }

    printabcd();
    printlengthbig();
}


int main()
{
    auto start = std::chrono::high_resolution_clock::now();


    loggerTest();
    

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time = end - start;
    std::cout << "\n\n\n---- Total execution time = " << time.count() << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}
