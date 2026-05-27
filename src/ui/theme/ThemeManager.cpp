#include "ThemeManager.h"

QString ThemeManager::accentColor()
{
    return "#4cc2ff";
}

void ThemeManager::applyDarkTheme(
    QApplication& app)
{
    app.setStyleSheet(
    R"(
        QWidget
        {
            background-color: #1e1e1e;
            color: #f0f0f0;
            font-size: 10pt;
        }

        QPushButton
        {
            background-color: #2d2d2d;
            border: 1px solid #444;
            border-radius: 8px;
            padding: 6px 12px;
        }

        QPushButton:hover
        {
            border: 1px solid #4cc2ff;
        }

        )"
    );
}
