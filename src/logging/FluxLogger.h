#pragma once

#include <QString>

class FluxLogger {
public:
    static void info(const QString& message);
    static void warning(const QString& message);
    static void error(const QString& message);
private:
    static void write(const QString& level, const QString& message);
};
