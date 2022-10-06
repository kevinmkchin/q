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
    void MessageFmt(const std::string& fmt, Args&&... args);

    template<class... Args>
    void WarningFmt(const std::string& fmt, Args&&... args);

    template<class... Args>
    void ErrorFmt(const std::string& fmt, Args&&... args);

    template<class... Args> 
    std::string Format(const std::string& fmt, Args&&... args);
};

extern MessageLogger PrintLog;

template<class ...Args>
inline void MessageLogger::MessageFmt(const std::string& fmt, Args && ...args)
{
    Message(Format(fmt, args...));
}

template<class ...Args>
inline void MessageLogger::WarningFmt(const std::string& fmt, Args && ...args)
{
    Warning(Format(fmt, args...));
}

template<class ...Args>
inline void MessageLogger::ErrorFmt(const std::string& fmt, Args && ...args)
{
    Error(Format(fmt, args...));
}

template<class ...Args>
inline std::string MessageLogger::Format(const std::string& fmt, Args && ...args)
{
#if _WIN32
    return std::vformat(fmt, std::make_format_args(args...));
#elif __APPLE__
    return "";
#endif
}
