// RAMFlux - Process Observer Interface
// Interface para observadores de processos

#ifndef IPROCESSOBSERVER_H
#define IPROCESSOBSERVER_H

#include <QObject>
#include <functional>
#include "ProcessMonitor.h"

// Forward declaration
class ProcessSnapshot;

class IProcessObserver : public QObject
{
    Q_OBJECT

public:
    explicit IProcessObserver(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IProcessObserver() = default;

    // Obter observador de processos (polimorfismo)
    static IProcessObserver* createInstance() { return new ProcessObserver; }

protected:
    // Implementado pelas subclasses
    virtual void observe(const ProcessSnapshot &snapshot) = 0;

signals:
    // Sinal quando um novo processo é detectado
    void processDetected(const ProcessInfo &process);
    
    // Sinal quando um processo é removido
    void processRemoved(const ProcessInfo &process);
    
    // Sinal quando o uso de memória de um processo muda significativamente
    void memoryUsageChanged(const ProcessInfo &process, quint64 previousUsage);
    
    // Sinal de alerta para processos com vazamento de memória detectado
    void memoryLeakDetected(const ProcessInfo &process);
};

// Implementação padrão de ProcessObserver
class ProcessObserver : public IProcessObserver
{
    Q_OBJECT

public:
    explicit ProcessObserver(QObject *parent = nullptr) : IProcessObserver(parent) {}
    
protected:
    void observe(const ProcessSnapshot &snapshot) override
    {
        // Implementação padrão - subclasses podem sobrescrever
    }

signals:
    // Sinais redeclarados
    void processDetected(const ProcessInfo &process) override;
    void processRemoved(const ProcessInfo &process) override;
    void memoryUsageChanged(const ProcessInfo &process, quint64 previousUsage) override;
    void memoryLeakDetected(const ProcessInfo &process) override;

    // Sinais de alerta de memória
    void memoryWarningThreshold(const ProcessInfo &process);
    void memoryCriticalThreshold(const ProcessInfo &process);
    void cpuWarningThreshold(const ProcessInfo &process);
    void cpuCriticalThreshold(const ProcessInfo &process);

};

#endif // IPROCESSOBSERVER_H