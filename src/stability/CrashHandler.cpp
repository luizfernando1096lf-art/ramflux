#include "CrashHandler.h"
#include "../logging/FluxLogger.h"
#include <Windows.h>

LONG WINAPI
RAMFluxExceptionHandler(
    EXCEPTION_POINTERS*)
{
    FluxLogger::error(
        "Unhandled exception detected"
    );
    return EXCEPTION_EXECUTE_HANDLER;
}

void CrashHandler::install()
{
    SetUnhandledExceptionFilter(
        RAMFluxExceptionHandler
    );
}
