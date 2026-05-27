#include "FluxLogger.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>

void FluxLogger::info(const QString& message)
{
    write("INFO", message);
}
void FluxLogger::warning(const QString& message)
{
    write("WARNING", message);
}
void FluxLogger::error(const QString& message)
{
    write("ERROR", message);
}
void FluxLogger::write(
    const QString& level,
    const QString& message)
{
    QFile file("ramflux.log");
    if (!file.open(
        QIODevice::Append |
        QIODevice::Text))
    {
        return;
    }

    QTextStream stream(&file);

    stream
        << "["
        << QDateTime::currentDateTime()
        .toString("yyyy-MM-dd hh:mm:ss")
        << "] "
        << "["
        << level
        << "] "
        << message
        << "\n";
}
