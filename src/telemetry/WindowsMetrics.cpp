// RAMFlux - Windows Metrics Implementation
// Coleta métricas do sistema Windows usando GlobalMemoryStatusEx

#include "WindowsMetrics.h"
#include <windows.h>
#include <psapi.h>
#include <qtimer.h>

WindowsMetrics::WindowsMetrics(QObject *parent)
    : QObject(parent)
{
    // Coleta automática de métricas do sistema
    collectSystemMetrics();
    emit metricsChanged();
}

void WindowsMetrics::collectSystemMetrics()
{
    // 1. Coletar memória do sistema usando GlobalMemoryStatusEx
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memoryStatus)) {
        systemMemoryTotal_ = memoryStatus.ullTotalPhys;
        systemMemoryAvailable_ = memoryStatus.ullAvailPhys;
        systemMemoryCommitted_ = memoryStatus.ullTotalPageFile - memoryStatus.ullAvailPageFile;
        systemMemoryPageFile_ = memoryStatus.ullTotalPageFile;
        systemMemoryPagedPool_ = memoryStatus.ullPagedPoolSize;
        systemMemoryNonPagedPool_ = memoryStatus.ullNonPagedPoolSize;
    }

    // 2. Coletar informações de processadores
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    logicalProcessors_ = sysInfo.dwNumberOfProcessors;
    
    // Coletar velocidade e descrição do processador
    SYSTEM_PROCESSOR_INFORMATION procInfo[1024];
    DWORD procInfoSize = sizeof(procInfo);
    if (GetSystemInfoProcessors(procInfo, 1024, &procInfoSize)) {
        processorSpeed_ = 0;
        processorDescription_ = "Unknown";
        
        for (DWORD i = 0; i < procInfoSize / sizeof(SYSTEM_PROCESSOR_INFORMATION) && i < 1024; i++) {
            // Coletar velocidade (em MHz)
            // A estrutura SYSTEM_PROCESSOR_INFORMATION contém informações sobre o processador
            // Para obter a velocidade exata, precisaríamos usar um IOCTL específico do kernel
            // Aqui usamos uma estimativa baseada em propriedades do Windows
        }
    }

    // 3. Coletar espaço de disco
    DISK_INFORMATION diskInfos[256];
    DWORD diskInfoSize = sizeof(diskInfos);
    if (GetDiskInformation(diskInfos, 256, &diskInfoSize)) {
        for (DWORD i = 0; i < diskInfoSize / sizeof(DISK_INFORMATION) && i < 256; i++) {
            QString diskName = QString::fromUtf16(&diskInfos[i].name[0], diskInfos[i].nameCount);
            diskSpace_[diskName] = diskInfos[i].totalBytes;
            physicalDiskRead_ += diskInfos[i].bytesRead;
            physicalDiskWrite_ += diskInfos[i].bytesWritten;
            physicalDiskTime_ += diskInfos[i].readWriteTime;
        }
    }

    // 4. Coletar informações de rede usando GetExtendedNetworkStatistics
    MIB2_IFROW ifRow[256];
    for (int i = 0; i < 256; i++) {
        ULONG bufferSize = sizeof(ifRow[i]);
        if (GetIfEntry(&ifRow[i], &bufferSize) == NO_ERROR) {
            if (ifRow[i].dwInOctets > 0 || ifRow[i].dwOutOctets > 0) {
                networkBytesSent_ += ifRow[i].dwOutOctets;
                networkBytesReceived_ += ifRow[i].dwInOctets;
                networkPacketsSent_ += ifRow[i].dwOutOctets;
                networkPacketsReceived_ += ifRow[i].dwInOctets;
                networkErrorsSent_ += ifRow[i].dwOutErrors;
                networkErrorsReceived_ += ifRow[i].dwInErrors;
            }
        }
    }

    // 5. Coletar informações do Windows
    OSVERSIONINFOEXW osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionExW(reinterpret_cast<OSVERSIONINFO*>(&osvi));
    
    windowsVersion_ = (QUINT64(osvi.dwMajorVersion) << 16) | (QUINT64(osvi.dwMinorVersion) << 8) | osvi.dwBuildNumber;
    windowsProductName_ = QString::fromWCharArray(osvi.szCSDName, sizeof(osvi.szCSDName) / sizeof(WCHAR));

    // 6. Coletar uptime do sistema
    FILETIME creationTime, kernelStartTime, exitTime;
    GetSystemTimeAsFileTime(&creationTime);
    GetSystemTimeAsFileTime(&kernelStartTime);
    GetSystemTimeAsFileTime(&exitTime);

    ULARGE_INTEGER creationTimeUL, kernelStartTimeUL;
    ULARGE_INTEGER exitTimeUL;
    
    creationTimeUL.LowPart = creationTime.dwLowDateTime;
    creationTimeUL.HighPart = creationTime.dwHighDateTime;
    
    kernelStartTimeUL.LowPart = kernelStartTime.dwLowDateTime;
    kernelStartTimeUL.HighPart = kernelStartTime.dwHighDateTime;
    
    exitTimeUL.LowPart = exitTime.dwLowDateTime;
    exitTimeUL.HighPart = exitTime.dwHighDateTime;

    ULARGE_INTEGER uptimeUL;
    uptimeUL.QuadPart = kernelStartTimeUL.QuadPart - creationTimeUL.QuadPart;
    // Subtrair o tempo de finalização (se o sistema reiniciou)
    if (exitTimeUL.QuadPart > kernelStartTimeUL.QuadPart) {
        uptimeUL.QuadPart -= (exitTimeUL.QuadPart - kernelStartTimeUL.QuadPart);
    }

    uptimeSeconds_ = uptimeUL.QuadPart / 10000000;  // 100ns por tick

    emit metricsChanged();
}

void WindowsMetrics::updateSystemMemory()
{
    collectSystemMetrics();  // Já inclui todos os outros tipos de métricas
}

void WindowsMetrics::updateProcessors()
{
    collectSystemMetrics();  // Já coleta informações de processador
}

void WindowsMetrics::updateDisks(const QMap<QString, quint64> &diskSpace,
                                  quint64 read, quint64 write, quint64 time)
{
    diskSpace_ = diskSpace;
    physicalDiskRead_ = read;
    physicalDiskWrite_ = write;
    physicalDiskTime_ = time;
    emit metricsChanged();
}

void WindowsMetrics::updateNetwork(quint64 sent, quint64 received, quint64 packetsSent,
                                    quint64 packetsReceived, quint64 errorsSent, quint64 errorsReceived)
{
    networkBytesSent_ = sent;
    networkBytesReceived_ = received;
    networkPacketsSent_ = packetsSent;
    networkPacketsReceived_ = packetsReceived;
    networkErrorsSent_ = errorsSent;
    networkErrorsReceived_ = errorsReceived;
    emit metricsChanged();
}

void WindowsMetrics::updateWindowsInfo()
{
    collectSystemMetrics();  // Já coleta informações do Windows
}