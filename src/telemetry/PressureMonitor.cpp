// RAMFlux - Pressure Monitor Implementation
// Implementação do monitor de pressão de memória

#include "PressureMonitor.h"
#include <QDateTime>

// Construtor
PressureMonitor::PressureMonitor(QObject *parent)
    : QObject(parent)
{
    // Inicializar
}

// Resetar
void PressureMonitor::reset()
{
    pressureLevel_ = PressureLevel::LOW;
    memoryPressure_ = 0;
    allocatedMemory_ = 0;
    usedMemory_ = 0;
    freeMemoryMB_ = 0;
    lowestMemoryMB_ = 0;
    leakSeverity_ = LeakSeverity::NONE;
    leakAnalysis_.clear();
    totalLeakedMemory_ = 0;
    detectedLeaks_.clear();
    averageMemoryMB_ = 0;
    stdDevMemoryMB_ = 0;
    isMemoryStable_ = true;
    unstableRegions_ = 0;
    pressureHistory_.clear();
    
    emit pressureChanged(PressureLevel::LOW);
}

// Obter histórico de pressão
QVector<PressurePoint> PressureMonitor::getPressureHistory(int limit)
{
    // Ordenar por timestamp
    pressureHistory_.sort();
    
    // Limite
    if (static_cast<int>(pressureHistory_.count()) > limit) {
        pressureHistory_.truncate(limit);
    }
    
    return pressureHistory_;
}