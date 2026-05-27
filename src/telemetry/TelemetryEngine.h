// RAMFlux - Telemetry Engine Header
// Engine principal de telemetria e métricas de sistema

#ifndef TELEMETRYENGINE_H
#define TELEMETRYENGINE_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QDateTime>

// Forward declarations
class MemoryMetrics;
class WindowsMetrics;
class PressureMonitor;
class FluxPressureAnalyzer;
class FluxTelemetry;
class FluxCleaner;

/**
 * @brief Engine principal de telemetria para RAMFlux
 */
class TelemetryEngine : public QObject
{
    Q_OBJECT

public:
    explicit TelemetryEngine(QObject *parent = nullptr);
    ~TelemetryEngine() = default;

    // Métodos principais
    void start();
    void stop();
    bool isRunning() const { return running_; }

    // Métricas de memória
    void setTotalMemory(qint64 totalMB);
    qint64 getTotalMemory() const { return totalMemory_; }
    void setUsedMemory(qint64 usedMB);
    qint64 getUsedMemory() const { return usedMemory_; }
    void setFreeMemory(qint64 freeMB);
    qint64 getFreeMemory() const { return freeMemory_; }
    qint64 getAvailableMemory() const { return availableMemory_; }
    qint64 getCommittedMemory() const { return committedMemory_; }
    qreal getMemoryPressure() const { return memoryPressure_; }

    // Métricas de processador
    void setProcessors(int count);
    int getProcessors() const { return processors_; }
    void setCPUUsage(qreal usagePercent);
    qreal getCPUUsage() const { return cpuUsage_; }
    qreal getCPUPercent() const { return cpuUsage_; }

    // Métricas de disco
    void setDiskReads(qint64 reads);
    qint64 getDiskReads() const { return diskReads_; }
    void setDiskWrites(qint64 writes);
    qint64 getDiskWrites() const { return diskWrites_; }
    qint64 getTotalDiskIO() const { return totalDiskIO_; }

    // Métricas de rede
    void setNetworkBytesReceived(qint64 bytes);
    qint64 getNetworkBytesReceived() const { return networkBytesReceived_; }
    void setNetworkBytesSent(qint64 bytes);
    qint64 getNetworkBytesSent() const { return networkBytesSent_; }
    qint64 getNetworkBandwidth() const { return networkBandwidth_; }

    // Timestamp atual
    QDateTime getTimestamp() const { return timestamp_; }

    // Histórico de métricas
    int getMetricsCount() const { return metrics_.count(); }
    QVector<QVariantMap> getHistory(int limit = 100) const;

    // Atualizar todas as métricas
    void updateAllMetrics();

signals:
    // Sinal de atualização de métricas
    void metricsUpdated();
    
    // Sinal de alerta de pressão de memória
    void pressureAlert(qreal pressurePercent, const QString &message);
    
    // Sinal de alerta de uso de CPU elevado
    void cpuAlert(qreal cpuPercent, const QString &message);
    
    // Sinal de alerta de disco lento
    void diskAlert(const QString &message);

public slots:
    void refreshAll();
    void clearHistory();

private:
    // Estado interno
    bool running_ = false;
    qint64 totalMemory_ = 0;
    qint64 usedMemory_ = 0;
    qint64 freeMemory_ = 0;
    qint64 availableMemory_ = 0;
    qint64 committedMemory_ = 0;
    qreal memoryPressure_ = 0.0;
    int processors_ = 0;
    qreal cpuUsage_ = 0.0;
    qint64 diskReads_ = 0;
    qint64 diskWrites_ = 0;
    qint64 totalDiskIO_ = 0;
    qint64 networkBytesReceived_ = 0;
    qint64 networkBytesSent_ = 0;
    qint64 networkBandwidth_ = 0;
    QDateTime timestamp_;

    // Timer de atualização
    QTimer *updateTimer_ = nullptr;
    int updateInterval_ = 1000; // 1 segundo

    // Histórico de métricas
    QVector<QVariantMap> metrics_;
    int maxHistorySize_ = 100;

    // Sub-componentes
    MemoryMetrics *memoryMetrics_ = nullptr;
    WindowsMetrics *windowsMetrics_ = nullptr;
    PressureMonitor *pressureMonitor_ = nullptr;
    FluxPressureAnalyzer *fluxPressureAnalyzer_ = nullptr;
    FluxTelemetry *fluxTelemetry_ = nullptr;
    FluxCleaner *fluxCleaner_ = nullptr;
};

#endif // TELEMETRYENGINE_H