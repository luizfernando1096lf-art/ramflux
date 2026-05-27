// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
namespace RAMFlux::Core {
Logger& Logger::instance() {    static Logger inst;
    return inst;
}
void Logger::setLogFile(const std::string& path) {    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_file.is_open()) { m_file.close(); }    m_file.open(path, std::ios::app);
}
void Logger::setMinLevel(LogLevel level) {    m_minLevel = level;
}
void Logger::setCallback(LogCallback cb) {    std::lock_guard<std::mutex> lock(m_mutex);    m_callback = std::move(cb);
}
void Logger::log(LogLevel level, const std::string& message) {
if(level < m_minLevel) return;
std::string line = "[" + currentTimestamp() + "] [" + levelToString(level) + "] " + message;
std::lock_guard<std::mutex> lock(m_mutex);
    if(m_consoleOutput) {
if(level == LogLevel::Error)            std::cerr << line << std::endl;        else            std::cout << line << std::endl;    }    if (m_file.is_open()) {        m_file << line << std::endl;        m_file.flush();    }    if (m_callback) {        m_callback(level, line);    }}
void Logger::debug(const std::string& message) { log(LogLevel::Debug, message); }
void Logger::info(const std::string& message) { log(LogLevel::Info, message); }
void Logger::warn(const std::string& message) { log(LogLevel::Warning, message); }
void Logger::error(const std::string& message) { log(LogLevel::Error, message); }
Logger::Logger() = default;
    Logger::~Logger() {
if(m_file.is_open()) m_file.close();
}
std::string Logger::levelToString(LogLevel level) const {
    switch(level) {
    case LogLevel::Debug:   return "DEBUG";
    case LogLevel::Info:    return "INFO";
    case LogLevel::Warning: return "WARN";
    case LogLevel::Error:   return "ERROR";
    }
return "UNKNOWN";
}
std::string Logger::currentTimestamp() const {    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(        now.time_since_epoch()) % 1000;
std::tm tm;    localtime_s(&tm, &t);
std::ostringstream oss;    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}} // namespace RAMFlux::Core


