#pragma once

#include <QString>

class SettingsManager {
public:
    static QString load(const QString& key, const QString& def = QString());
    static void save(const QString& key, const QString& value);
};
