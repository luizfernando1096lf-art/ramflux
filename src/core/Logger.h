// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <functional>
namespace RAMFlux::Core {
enum class LogLevel {    Debug,    Info,    Warning,    Error};
class Logger {
public:
    using LogCallback = std::function<void(LogLevel, const std::string&)>;
static Logger& instance();
void setLogFile(const std::string& path);
void setMinLevel(LogLevel level);
void setCallback(LogCallback cb);
void log(LogLevel level, const std::string& message);
void debug(const std::string& message);
void info(const std::string& message);
void warn(const std::string& message);
void error(const std::string& message);
    private:
    Logger();    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
std::string levelToString(LogLevel level) const;
std::string currentTimestamp() const;
std::ofstream m_file;
std::mutex m_mutex;    LogLevel m_minLevel{
LogLevel::Info};
bool m_consoleOutput{
false};
LogCallback m_callback;
};
} // namespace RAMFlux::Core


