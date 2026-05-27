// RAMFlux - Telemetry Engine Implementation
// Implementação do engine de telemetria unificado

#include "TelemetryEngine.h"
#include "WindowsMetrics.h"
#include "MemoryMetrics.h"
#include "PressureMonitor.h"
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QtGlobal>

// Construtor
TelemetryEngine::TelemetryEngine(QObject *parent)
    : QObject(parent),
      samplingInterval_(1000),
      updateInterval_(5000)
{
    // Inicializar componentes
    systemMetrics_.reset();
    memoryMetrics_.reset();
    pressureMonitor_.reset();
    
    // Iniciar thread de coleta
    collectionThread_.moveToThread(&collectionThread_);
    collectionThread_.start();
    
    // Conectar sinais e slots
    connect(&collectionThread_, &QThread::finished, this, &TelemetryEngine::cleanup);
}

// Destruir
TelemetryEngine::~TelemetryEngine()
{
    stopCollection();
    if (collectionThread_.isRunning()) {
        collectionThread_.quit();
        collectionThread_.wait(5000);
    }
}

// Iniciar coleta
void TelemetryEngine::startCollection()
{
    collectionActive_ = true;
    samplingInterval_ = samplingInterval_.value;
    updateInterval_ = updateInterval_.value;
}

// Parar coleta
void TelemetryEngine::stopCollection()
{
    collectionActive_ = false;
}

// Atualizar intervalo de amostragem
void TelemetryEngine::setSamplingInterval(int interval)
{
    samplingInterval_ = interval;
}

// Atualizar intervalo de atualização
void TelemetryEngine::setUpdateInterval(int interval)
{
    updateInterval_ = interval;
}

// Obter métricas do sistema
QMap<QString, QVariant> TelemetryEngine::getSystemMetrics()
{
    // Converter para QVariantMap
    QMap<QString, QVariant> result;
    result["SystemMemoryTotal"] = systemMetrics_->getSystemMemoryTotal();
    result["SystemMemoryAvailable"] = systemMetrics_->getSystemMemoryAvailable();
    result["LogicalProcessors"] = systemMetrics_->getLogicalProcessors();
    result["ProcessorSpeed"] = systemMetrics_->getProcessorSpeed();
    result["ProcessorDescription"] = systemMetrics_->getProcessorDescription();
    result["WindowsVersion"] = systemMetrics_->getWindowsVersion();
    result["WindowsProductName"] = systemMetrics_->getWindowsProductName();
    result["UptimeSeconds"] = systemMetrics_->getUptimeSeconds();
    result["PhysicalDiskRead"] = systemMetrics_->getPhysicalDiskRead();
    result["PhysicalDiskWrite"] = systemMetrics_->getPhysicalDiskWrite();
    result["PhysicalDiskTime"] = systemMetrics_->getPhysicalDiskTime();
    result["NetworkBytesSent"] = systemMetrics_->getNetworkBytesSent();
    result["NetworkBytesReceived"] = systemMetrics_->getNetworkBytesReceived();
    
    return result;
}

// Obter métricas de memória
QMap<QString, QVariant> TelemetryEngine::getMemoryMetrics()
{
    QMap<QString, QVariant> result;
    result["TotalMemory"] = memoryMetrics_->getTotalMemory();
    result["UsedMemory"] = memoryMetrics_->getUsedMemory();
    result["FreeMemory"] = memoryMetrics_->getFreeMemory();
    result["AvailableMemory"] = memoryMetrics_->getAvailableMemory();
    result["CommittedMemory"] = memoryMetrics_->getCommittedMemory();
    result["PeakVirtualMemory"] = memoryMetrics_->getPeakVirtualMemory();
    result["WorkingSet"] = memoryMetrics_->getWorkingSet();
    result["PageFaults"] = memoryMetrics_->getPageFaults();
    result["CacheHitRate"] = memoryMetrics_->getCacheHitRate();
    result["ProcessCount"] = memoryMetrics_->getProcessCount();
    
    return result;
}

// Obter estado de pressão de memória
QMap<QString, QVariant> TelemetryEngine::getPressureStatus()
{
    QMap<QString, QVariant> result;
    result["PressureLevel"] = static_cast<int>(pressureMonitor_->getPressureLevel());
    result["MemoryPressure"] = pressureMonitor_->getMemoryPressure();
    result["AllocatedMemory"] = pressureMonitor_->getAllocatedMemory();
    result["UsedMemory"] = pressureMonitor_->getUsedMemory();
    result["FreeMemoryMB"] = pressureMonitor_->getFreeMemoryMB();
    result["LeakSeverity"] = static_cast<int>(pressureMonitor_->getLeakSeverity());
    result["LeakAnalysis"] = pressureMonitor_->getLeakAnalysis();
    result["TotalLeakedMemory"] = pressureMonitor_->getTotalLeakedMemory();
    result["AverageMemoryMB"] = pressureMonitor_->getAverageMemoryMB();
    result["StdDevMemoryMB"] = pressureMonitor_->getStdDevMemoryMB();
    result["IsMemoryStable"] = pressureMonitor_->isMemoryStable();
    result["UnstableRegions"] = pressureMonitor_->getUnstableRegions();
    
    return result;
}

// Obter histórico de métricas
QVector<QVariantMap> TelemetryEngine::getMetricsHistory(int limit)
{
    QVector<QVariantMap> history;
    
    // Limitar resultados
    if (static_cast<int>(metricsHistory_.count()) > limit) {
        metricsHistory_.truncate(limit);
    }
    
    // Adicionar ao histórico
    history = metricsHistory_;
    return history;
}

// Obter processo por ID
QVariantMap TelemetryEngine::getProcessById(quint64 processId)
{
    auto process = memoryMetrics_->getProcessById(processId);
    
    QVariantMap result;
    result["Id"] = process.id;
    result["Name"] = process.name;
    result["MemoryMB"] = process.memoryMB;
    result["WorkingSetMB"] = process.workingSetMB;
    result["PrivateMemoryMB"] = process.privateMemoryMB;
    result["VirtualMemoryMB"] = process.virtualMemoryMB;
    result["ThreadCount"] = process.threadCount;
    result["IsApp"] = process.isApp;
    result["CommandLine"] = process.commandLine;
    
    return result;
}

// Obter top processos por memória
QList<QVariantMap> TelemetryEngine::getTopMemoryProcesses(int limit)
{
    QList<ProcessInfo> processes = memoryMetrics_->getTopMemoryProcesses(limit);
    
    QList<QVariantMap> result;
    for (const auto &proc : processes) {
        QVariantMap map;
        map["Id"] = proc.id;
        map["Name"] = proc.name;
        map["MemoryMB"] = proc.memoryMB;
        map["WorkingSetMB"] = proc.workingSetMB;
        map["PrivateMemoryMB"] = proc.privateMemoryMB;
        map["VirtualMemoryMB"] = proc.virtualMemoryMB;
        map["ThreadCount"] = proc.threadCount;
        map["IsApp"] = proc.isApp;
        map["CommandLine"] = proc.commandLine;
        result.append(map);
    }
    
    return result;
}

// Exportar métricas para CSV
QString TelemetryEngine::exportToCSV(const QString &filename)
{
    // Gerar conteúdo CSV
    QString csv;
    csv.append("Timestamp,MemoryTotal,MemoryUsed,MemoryFree,MemoryAvailable,"
               "CommittedMemory,PeakVirtualMemory,WorkingSet,PageFaults,"
               "ProcessCount,PressureLevel,LeakSeverity\n");
    
    for (const auto &entry : metricsHistory_) {
        csv.append(QString::fromVariant(entry[0], "Timestamp"))
                 .append(",")
                 .append(QString::number(entry[1].toUInt64()))
                 .append(",")
                 .append(QString::number(entry[2].toUInt64()))
                 .append(",")
                 .append(QString::number(entry[3].toUInt64()))
                 .append(",")
                 .append(QString::number(entry[4].toUInt64()))
                 .append(",")
                 .append(QString::number(entry[5].toUInt64()))
                 .append(",")
                 .append(QString::number(entry[6].toUInt64()))
                 .append(",")
                 .append(QString::number(entry[7].toUInt64()))
                 .append(",")
                 .append(QString::number(entry[8].toInt()))
                 .append(",")
                 .append(QString::number(entry[9].toInt()))
                 .append("\n");
    }
    
    return csv;
}

// Obter métricas de rede
QMap<QString, QVariant> TelemetryEngine::getNetworkMetrics()
{
    QMap<QString, QVariant> result;
    result["BytesSent"] = systemMetrics_->getNetworkBytesSent();
    result["BytesReceived"] = systemMetrics_->getNetworkBytesReceived();
    result["PacketsSent"] = systemMetrics_->getNetworkPacketsSent();
    result["PacketsReceived"] = systemMetrics_->getNetworkPacketsReceived();
    result["ErrorsSent"] = systemMetrics_->getNetworkErrorsSent();
    result["ErrorsReceived"] = systemMetrics_->getNetworkErrorsReceived();
    
    return result;
}

// Obter espaço de disco
QMap<QString, QVariant> TelemetryEngine::getDiskSpace()
{
    return systemMetrics_->getDiskSpace();
}

// Limpar
void TelemetryEngine::cleanup()
{
    systemMetrics_.reset();
    memoryMetrics_.reset();
    pressureMonitor_.reset();
    metricsHistory_.clear();
}