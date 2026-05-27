// RAMFlux - Process Monitor Observer Implementation
// Observador que monitora métricas de processos em tempo real

#include "IProcessObserver.h"
#include <QtCharts>
#include <QTimer>

class ProcessMonitorObserver : public IProcessObserver
{
    Q_OBJECT

public:
    explicit ProcessMonitorObserver(QObject *parent = nullptr);
    
    // Observar mudanças na métrica do process monitor
    void observe(const ProcessSnapshot &snapshot);

protected:
    QTimer *m_timer;
    int m_pollingInterval = 1000;
};