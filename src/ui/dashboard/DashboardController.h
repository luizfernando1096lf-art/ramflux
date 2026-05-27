// RAMFlux - Dashboard Controller
// Gerencia a lógica do dashboard premium com charts e métricas

#ifndef DASHBOARD_CONTROLLER_H
#define DASHBOARD_CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QList>
#include <QMap>

// Forward declarations
class MemorySnapshot;

class DashboardController : public QObject
{
    Q_OBJECT

public:
    explicit DashboardController(QObject *parent = nullptr);
    ~DashboardController();

    // Interface pública
    void updateMemoryMetrics(double usedMB, double totalMB, double freeMB);
    void updateActiveProcesses(int count);
    void updateSystemLoad(double cpuLoad, double memoryPressure);
    void addChartData(const QMap<int, double> &points);

    // Getters
    struct MemoryMetrics {
        double usedMB = 0;
        double totalMB = 0;
        double freeMB = 0;
        double utilizationPercent = 0;
    };

    [[nodiscard]] MemoryMetrics getMemoryMetrics() const;
    [[nodiscard]] int getActiveProcessCount() const;
    [[nodiscard]] double getSystemLoad() const;
    [[nodiscard]] QList<double> getChartData() const;

private:
    QTimer *updateTimer_;
    MemoryMetrics memoryMetrics_;
    int activeProcessCount_;
    double systemLoad_;
    QMap<int, double> chartData_;
    QMap<int, double> chartHistory_;
    static constexpr int MAX_HISTORY_POINTS = 100;

signals:
    void memoryUpdated(const MemoryMetrics &metrics);
    void dataPointsUpdated();

public slots:
    void startAutoUpdate(int intervalMs = 1000);
    void stopAutoUpdate();

private slots:
    void onTimerTick();
};

#endif // DASHBOARD_CONTROLLER_H