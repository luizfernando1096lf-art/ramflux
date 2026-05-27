#include "WorkingSetOptimizer.h"
#include <psapi.h>

bool WorkingSetOptimizer::trimProcess(
    HANDLE process)
{
    return EmptyWorkingSet(process) != 0;
}
