// RAMFlux - Windows Metrics Header
// Métricas específicas do sistema Windows usando GlobalMemoryStatusEx

#ifndef WINDOWSMETRICS_H
#define WINDOWSMETRICS_H

#include <QObject>
#include <QMap>
#include <QVariantMap>

class WindowsMetrics : public QObject
{
    Q_OBJECT

public:
    explicit WindowsMetrics(QObject *parent = nullptr);
    ~WindowsMetrics() = default;

    // Métricas de sistema
    quint64 getSystemMemoryTotal() const { return systemMemoryTotal_; }
    quint64 getSystemMemoryAvailable() const { return systemMemoryAvailable_; }
    quint64 getSystemMemoryCommitted() const { return systemMemoryCommitted_; }
    quint64 getSystemMemoryPageFile() const { return systemMemoryPageFile_; }
    quint64 getSystemMemoryPagedPool() const { return systemMemoryPagedPool_; }
    quint64 getSystemMemoryNonPagedPool() const { return systemMemoryNonPagedPool_; }

    // Métricas de processador
    int getLogicalProcessors() const { return logicalProcessors_; }
    quint64 getProcessorSpeed() const { return processorSpeed_; }
    QString getProcessorDescription() const { return processorDescription_; }

    // Métricas de disco
    QMap<QString, quint64> getDiskSpace() const { return diskSpace_; }
    quint64 getPhysicalDiskRead() const { return physicalDiskRead_; }
    quint64 getPhysicalDiskWrite() const { return physicalDiskWrite_; }
    quint64 getPhysicalDiskTime() const { return physicalDiskTime_; }

    // Métricas de rede
    quint64 getNetworkBytesSent() const { return networkBytesSent_; }
    quint64 getNetworkBytesReceived() const { return networkBytesReceived_; }
    quint64 getNetworkPacketsSent() const { return networkPacketsSent_; }
    quint64 getNetworkPacketsReceived() const { return networkPacketsReceived_; }
    quint64 getNetworkErrorsSent() const { return networkErrorsSent_; }
    quint64 getNetworkErrorsReceived() const { return networkErrorsReceived_; }

    // Windows Information
    quint64 getWindowsVersion() const { return windowsVersion_; }
    QString getWindowsProductName() const { return windowsProductName_; }
    quint64 getUptimeSeconds() const { return uptimeSeconds_; }

    // Atualizar métricas (automático com GlobalMemoryStatusEx)
    void updateSystemMemory();  // Coleta memória real do sistema
    void updateProcessors();  // Coleta informações de processadores
    void updateDisks(const QMap<QString, quint64> &diskSpace,
                     quint64 read, quint64 write, quint64 time);  // Discos ainda precisa de parâmetros
    void updateNetwork(quint64 sent, quint64 received, quint64 packetsSent,
                       quint64 packetsReceived, quint64 errorsSent, quint64 errorsReceived);
    void updateWindowsInfo();  // Coleta versão e uptime do Windows

signals:
    void metricsChanged();

private:
    quint64 systemMemoryTotal_ = 0;
    quint64 systemMemoryAvailable_ = 0;
    quint64 systemMemoryCommitted_ = 0;
    quint64 systemMemoryPageFile_ = 0;
    quint64 systemMemoryPagedPool_ = 0;
    quint64 systemMemoryNonPagedPool_ = 0;

    int logicalProcessors_ = 0;
    quint64 processorSpeed_ = 0;
    QString processorDescription_;

    QMap<QString, quint64> diskSpace_;
    quint64 physicalDiskRead_ = 0;
    quint64 physicalDiskWrite_ = 0;
    quint64 physicalDiskTime_ = 0;

    quint64 networkBytesSent_ = 0;
    quint64 networkBytesReceived_ = 0;
    quint64 networkPacketsSent_ = 0;
    quint64 networkPacketsReceived_ = 0;
    quint64 networkErrorsSent_ = 0;
    quint64 networkErrorsReceived_ = 0;

    quint64 windowsVersion_ = 0;
    QString windowsProductName_;
    quint64 uptimeSeconds_ = 0;
};

#endif // WINDOWSMETRICS_H