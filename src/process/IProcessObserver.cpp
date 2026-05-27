// RAMFlux - Process Observer Implementation

#include "IProcessObserver.h"

IProcessObserver* IProcessObserver::createInstance()
{
    return new ProcessObserver;
}

void ProcessObserver::observe(const ProcessSnapshot &snapshot)
{
    // Implementação padrão
}