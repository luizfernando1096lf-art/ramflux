// RAMFlux - FluxCleaner Implementation
// Gerenciador de limpeza de memória para RAMFlux

#include "FluxCleaner.h"

FluxCleaner::FluxCleaner(QObject *parent)
    : QObject(parent),
      isRunning_(false),
      autoCleanupInterval_(5000),
      cleanThreshold_(80.0),
      cleanedObjects_(0),
      reclaimedMemory_(0)
{
}

FluxCleaner::~FluxCleaner()
{
    stopMonitoring();
}

void FluxCleaner::startMonitoring()
{
    if (isRunning_) return;
    
    isRunning_ = true;
    emit cleanupStarted();
    
    updateTimer_ = new QTimer(this);
    updateTimer_->start(autoCleanupInterval_);
    
    QObject::connect(updateTimer_, &QTimer::timeout, this, [this]() {
        if (memoryPressure_ >= cleanThreshold_) {
            cleanupObjects();
        }
    });
}

void FluxCleaner::stopMonitoring()
{
    if (updateTimer_) {
        updateTimer_->stop();
        delete updateTimer_;
        updateTimer_ = nullptr;
    }
    isRunning_ = false;
}

void FluxCleaner::cleanupObjects()
{
    emit cleanupStarted();
    
    // Simular limpeza de objetos
    for (auto it = objectRegistry_.begin(); it != objectRegistry_.end(); ++it) {
        reclaimedMemory_ += 64; // 64KB por objeto
        cleanedObjects_++;
        emit objectRemoved(it.key());
    }
    
    objectRegistry_.clear();
    emit cleanupCompleted(cleanedObjects_, reclaimedMemory_);
    cleanedObjects_ = 0;
    reclaimedMemory_ = 0;
}

void FluxCleaner::cleanupProcess()
{
    // Forçar GC do process
    emit cleanupStarted();
    reclaimedMemory_ = 1024 * 1024; // 1MB simulado
    cleanedObjects_ = 100;
    
    emit cleanupCompleted(cleanedObjects_, reclaimedMemory_);
}

void FluxCleaner::setAutoCleanupInterval(int ms)
{
    if (updateTimer_) {
        updateTimer_->setInterval(ms);
        autoCleanupInterval_ = ms;
    }
}

void FluxCleaner::setCleanThreshold(qreal percent)
{
    cleanThreshold_ = percent;
}

void FluxCleaner::setMemoryPressure(qreal pressure)
{
    memoryPressure_ = pressure;
    
    if (pressure >= cleanThreshold_) {
        emit memoryPressureHigh(pressure);
    }
}

void FluxCleaner::addObject(qint64 id)
{
    objectRegistry_.insert(id, true);
    emit objectAdded(id);
}

void FluxCleaner::removeObject(qint64 id)
{
    if (objectRegistry_.contains(id)) {
        objectRegistry_.remove(id);
        emit objectRemoved(id);
    }
}