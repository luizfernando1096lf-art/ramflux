#include "DashboardWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QStatusBar>

DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(600, 400);
}

void DashboardWidget::loadSnapshotData(ISnapshotProcessor& processor)
{
    // Implementação básica - subclasses devem sobrescrever
}

void DashboardWidget::updateView()
{
    // Implementação básica - subclasses devem sobrescrever
}