// RAMFlux - Flux Pressure Analyzer Header
// Analisador de pressão de fluxo de memória

#ifndef FLUXPRESSUREANALYZER_H
#define FLUXPRESSUREANALYZER_H

#include <QObject>
#include <QVector>
#include <QUuid>

class FluxPressureAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit FluxPressureAnalyzer(QObject *parent = nullptr);
    ~FluxPressureAnalyzer() = default;

    // Analisar ponto de pressão
    PressureAnalysis analyzePressurePoint(int memoryMB, int timestamp);

    // Obter análise de ponto
    PressureAnalysis getPressureAnalysis() const { return pressureAnalysis_; }

    // Obter nível de pressão
    PressureLevel getPressureLevel() const { return pressureLevel_; }

    // Obter fluxo de memória
    int getMemoryFluxMBPerSec() const { return memoryFluxMBPerSec_; }

    // Obter tendência
    MemoryTrend getMemoryTrend() const { return memoryTrend_; }

    // Obter volume de memória
    int getMemoryVolume() const { return memoryVolume_; }

    // Obter latência
    int getLatencyMS() const { return latencyMS_; }

    // Obter throughput
    int getThroughputMBPS() const { return throughputMBPS_; }

    // Resetar
    void reset();

signals:
    void pressureLevelChanged(PressureLevel level);
    void memoryFluxChanged(int flux);

private:
    struct PressureAnalysis {
        int memoryMB;
        PressureLevel level;
        int fluxMBPS;
        MemoryTrend trend;
        int latencyMS;
        int errorRate;
        QUuid timestamp;
    };

    PressureAnalysis pressureAnalysis_;
    PressureLevel pressureLevel_ = PressureLevel::LOW;
    int memoryFluxMBPerSec_ = 0;
    MemoryTrend memoryTrend_ = MemoryTrend::STABLE;
    int memoryVolume_ = 0;
    int latencyMS_ = 0;
    int throughputMBPS_ = 0;
};

#endif // FLUXPRESSUREANALYZER_H