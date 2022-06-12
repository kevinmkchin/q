#pragma once

#include <string>
#include <format>

class MessageLogger
{
public:
    void Message(const std::string& message);
    void Warning(const std::string& warning_message);
    void Error(const std::string& error_message);

    template<class... Args> 
    std::string Format(const std::string& fmt, Args&&... args);
};

extern MessageLogger PrintLog;

template<class ...Args>
inline std::string MessageLogger::Format(const std::string& fmt, Args && ...args)
{
    return std::vformat(fmt, std::make_format_args(args...));
}
