#include "MessageLogger.h"


MessageLogger PrintLog;

void MessageLogger::Message(const std::string& message)
{
    printf(std::string(message + "\n").c_str());
}

void MessageLogger::Warning(const std::string& warning_message)
{
    printf(std::string("WARNING: " + warning_message + "\n").c_str());
}

void MessageLogger::Error(const std::string& error_message)
{
    printf(std::string("ERROR: " + error_message + "\n").c_str());
}
