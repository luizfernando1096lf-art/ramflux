#pragma once

#include <QApplication>
#include <QString>

class ThemeManager {
public:
    static QString accentColor();
    static void applyDarkTheme(QApplication& app);
};
