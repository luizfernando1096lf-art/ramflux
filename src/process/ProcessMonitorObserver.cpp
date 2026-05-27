// RAMFlux - Process Monitor Observer Implementation
// Observador que monitora métricas de processos em tempo real
// TASK 4.2

#include "ProcessMonitorObserver.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

ProcessMonitorObserver::ProcessMonitorObserver(QObject *parent)
    : IProcessObserver(parent), m_timer(new QTimer(this))
{
    // Configurar polling automático para atualizações de 1 segundo
    m_timer->setInterval(m_pollingInterval);
    
    // Conectar timeout do timer
    QObject::connect(m_timer, &QTimer::timeout, this, [this]() {
        // Polling automático - pode ser expandido
        // em um cenário real para re-coletar métricas
    });
}

void ProcessMonitorObserver::observe(const ProcessSnapshot &snapshot)
{
    Q_UNUSED(snapshot);
    // Observar snapshot do processo
    // Implementação base que pode ser estendida
}