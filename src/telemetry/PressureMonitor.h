// RAMFlux - Pressure Monitor Header
// Monitor de pressão de RAM e vazamentos de memória

#ifndef PRESSUREMONITOR_H
#define PRESSUREMONITOR_H

#include <QObject>
#include <QVector>
#include <QUuid>

class PressureMonitor : public QObject
{
    Q_OBJECT

public:
    explicit PressureMonitor(QObject *parent = nullptr);
    ~PressureMonitor() = default;

    // Pressão de RAM
    enum class PressureLevel {
        LOW = 0,
        MODERATE = 25,
        HIGH = 50,
        CRITICAL = 75,
        EMERGENCY = 100
    };

    enum class LeakSeverity {
        NONE = 0,
        LOW = 1,
        MODERATE = 2,
        HIGH = 3,
        CRITICAL = 4
    };

    // Métricas atuais
    PressureLevel getPressureLevel() const { return pressureLevel_; }
    quint64 getMemoryPressure() const { return memoryPressure_; }
    quint64 getAllocatedMemory() const { return allocatedMemory_; }
    quint64 getUsedMemory() const { return usedMemory_; }
    int getFreeMemoryMB() const { return freeMemoryMB_; }
    int getLowestMemoryMB() const { return lowestMemoryMB_; }

    // Análise de vazamentos
    LeakSeverity getLeakSeverity() const { return leakSeverity_; }
    QString getLeakAnalysis() const { return leakAnalysis_; }
    quint64 getTotalLeakedMemory() const { return totalLeakedMemory_; }
    QVector<MemoryLeakInfo> getDetectedLeaks() const { return detectedLeaks_; }

    // Estatísticas
    int getAverageMemoryMB() const { return averageMemoryMB_; }
    int getStdDevMemoryMB() const { return stdDevMemoryMB_; }
    bool isMemoryStable() const { return isMemoryStable_; }
    int getUnstableRegions() const { return unstableRegions_; }

    // Histórico
    QVector<PressurePoint> getPressureHistory(int limit = 100) const;

    // Reset
    void reset();

signals:
    void pressureChanged(PressureLevel level);
    void leakDetected(MemoryLeakInfo leak);

private:
    PressureLevel pressureLevel_ = PressureLevel::LOW;
    quint64 memoryPressure_ = 0;
    quint64 allocatedMemory_ = 0;
    quint64 usedMemory_ = 0;
    int freeMemoryMB_ = 0;
    int lowestMemoryMB_ = 0;
    LeakSeverity leakSeverity_ = LeakSeverity::NONE;
    QString leakAnalysis_;
    quint64 totalLeakedMemory_ = 0;
    QVector<MemoryLeakInfo> detectedLeaks_;
    int averageMemoryMB_ = 0;
    int stdDevMemoryMB_ = 0;
    bool isMemoryStable_ = true;
    int unstableRegions_ = 0;

    QVector<PressurePoint> pressureHistory_;

    struct PressurePoint {
        QUuid id;
        quint64 timestamp;
        int memoryMB;
        PressureLevel level;
    };

    struct MemoryLeakInfo {
        QUuid id;
        QString process;
        int memoryMB;
        int threadId;
        QString description;
        quint64 firstDetected;
        quint64 lastSeen;
        quint64 samplesCount;
    };
};

#endif // PRESSUREMONITOR_H