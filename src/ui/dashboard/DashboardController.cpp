// RAMFlux - Dashboard Controller Implementation

#include "DashboardController.h"
#include <QtDebug>

DashboardController::DashboardController(QObject *parent)
    : QObject(parent)
    , updateTimer_(new QTimer(this))
    , activeProcessCount_(0)
    , systemLoad_(0.0)
{
    // Iniciar timer de atualização automática
    updateTimer_->setInterval(1000);
    connect(updateTimer_, &QTimer::timeout, this, &DashboardController::onTimerTick);
}

DashboardController::~DashboardController()
{
    stopAutoUpdate();
}

void DashboardController::updateMemoryMetrics(double usedMB, double totalMB, double freeMB)
{
    memoryMetrics_.usedMB = usedMB;
    memoryMetrics_.totalMB = totalMB;
    memoryMetrics_.freeMB = freeMB;
    memoryMetrics_.utilizationPercent = (totalMB > 0) ? (usedMB / totalMB * 100) : 0;

    emit memoryUpdated(memoryMetrics_);
}

void DashboardController::updateActiveProcesses(int count)
{
    activeProcessCount_ = count;
}

void DashboardController::updateSystemLoad(double cpuLoad, double memoryPressure)
{
    systemLoad_ = cpuLoad * 0.6 + memoryPressure * 0.4;  // Média ponderada
}

void DashboardController::addChartData(const QMap<int, double> &points)
{
    chartData_.clear();
    chartData_ = points;

    // Manter histórico
    int now = QDateTime::currentMSecsSinceEpoch() / 1000;
    for (int i = now - MAX_HISTORY_POINTS + 1; i <= now; ++i) {
        chartHistory_[i] = points.value(i, 0);
    }

    emit dataPointsUpdated();
}

DashboardController::MemoryMetrics DashboardController::getMemoryMetrics() const
{
    return memoryMetrics_;
}

int DashboardController::getActiveProcessCount() const
{
    return activeProcessCount_;
}

double DashboardController::getSystemLoad() const
{
    return systemLoad_;
}

QList<double> DashboardController::getChartData() const
{
    QList<double> data;
    for (const auto &point : chartData_) {
        data.append(point.second);
    }
    return data;
}

void DashboardController::startAutoUpdate(int intervalMs)
{
    updateTimer_->setInterval(intervalMs);
    updateTimer_->start();
}

void DashboardController::stopAutoUpdate()
{
    updateTimer_->stop();
    updateTimer_->deleteLater();
}

void DashboardController::onTimerTick()
{
    // Simular atualização periódica
    double currentLoad = systemLoad_ + (qrand() % 10 - 5) / 100.0;
    systemLoad_ = qBound(0.0, currentLoad, 100.0);
}