#pragma once

#include <Windows.h>

class WorkingSetOptimizer {
public:
    static bool trimProcess(HANDLE process);
};
