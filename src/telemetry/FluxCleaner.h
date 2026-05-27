// RAMFlux - FluxCleaner Header
// Gerenciador de limpeza de memória para RAMFlux

#ifndef FLUXCLEANER_H
#define FLUXCLEANER_H

#include <QObject>
#include <QTimer>
#include <QVariantMap>

/**
 * @brief Gerenciador de limpeza de memória para RAMFlux
 */
class FluxCleaner : public QObject
{
    Q_OBJECT

public:
    explicit FluxCleaner(QObject *parent = nullptr);
    ~FluxCleaner();

    // Estado
    bool isRunning() const { return isRunning_; }
    int getCleanedObjects() const { return cleanedObjects_; }
    qint64 getReclaimedMemory() const { return reclaimedMemory_; }

    // Métodos de limpeza
    void startMonitoring();
    void stopMonitoring();
    void cleanupObjects();
    void cleanupProcess();
    
    // Gerenciamento de objetos
    void addObject(qint64 id);
    void removeObject(qint64 id);
    bool hasObject(qint64 id) const { return objectRegistry_.contains(id); }
    
    // Configurações
    void setAutoCleanupInterval(int ms);
    void setCleanThreshold(qreal percent);

signals:
    void cleanupStarted();
    void cleanupCompleted(qint64 objects, qint64 reclaimed);
    void objectAdded(qint64 id);
    void objectRemoved(qint64 id);
    void memoryPressureHigh(qreal percent);
};

#endif // FLUXCLEANER_H