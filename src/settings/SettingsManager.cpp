#include "SettingsManager.h"

QString SettingsManager::load(const QString& key, const QString& def)
{
    Q_UNUSED(key)
    return def;
}

void SettingsManager::save(const QString& key, const QString& value)
{
    Q_UNUSED(key)
    Q_UNUSED(value)
}
