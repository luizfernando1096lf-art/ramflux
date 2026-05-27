#include <QApplication>
#include "src/ui/theme/ThemeManager.h"
#include "src/stability/CrashHandler.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CrashHandler::install();

    ThemeManager::applyDarkTheme(app);

    return app.exec();
}
