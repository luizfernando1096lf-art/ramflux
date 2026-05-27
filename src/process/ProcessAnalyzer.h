/**
 * @file ProcessAnalyzer.h
 * @brief Análise avançada de processos e métricas de sistema
 */

#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <map>
#include "ProcessMonitor.h"

namespace RAMFlux
{
namespace Process
{

/**
 * @brief Representa informações detalhadas de um processo
 */
struct ProcessInfo
{
    HANDLE hProcess;                ///< Handle do processo
    DWORD processId;                ///< ID do processo
    std::wstring processName;       ///< Nome do processo
    SIZE_T workingSetSize;          ///< Tamanho da Working Set (RAM alocada)
    SIZE_T privateSize;             ///< Tamanho privado da memória
    DWORD exitCode;                 ///< Código de saída
    bool exists;                    ///< Se o processo ainda existe
};

/**
 * @brief Enumera todos os processos em execução
 */
class ProcessEnumerator
{
public:
    /**
     * @brief Enumera todos os processos
     * @return Vetor de handles de processos
     */
    std::vector<HANDLE> enumProcesses() const;

    /**
     * @brief Enumera todos os processos com suas informações
     * @return Vetor de ProcessInfo
     */
    std::vector<ProcessInfo> enumProcessesWithInfo() const;

    /**
     * @brief Filtra processos por nome parcial
     * @param name Nome parcial para filtrar
     * @return Vetor de ProcessInfo filtrados
     */
    std::vector<ProcessInfo> filterByName(const std::wstring& name) const;

    /**
     * @brief Verifica se um processo existe
     * @param processId ID do processo
     * @return true se existe, false caso contrário
     */
    bool processExists(DWORD processId) const;

private:
    void getProcessInfo(HANDLE hProcess, DWORD pid, ProcessInfo& info) const;
    SIZE_T getProcessWorkingSet(HANDLE hProcess, SIZE_T& workingSet) const;
    SIZE_T getProcessPrivateSize(HANDLE hProcess, SIZE_T& privateSize) const;
};

/**
 * @brief Análise avançada de métricas de processo
 */
class ProcessAnalyzer
{
public:
    ProcessAnalyzer() = default;
    ~ProcessAnalyzer() = default;

    /**
     * @brief Analisa todos os processos em execução
     * @return Mapa de PID -> ProcessInfo
     */
    std::map<DWORD, ProcessInfo> analyzeAllProcesses() const;

    /**
     * @brief Analisa processos específicos
     * @param pids Vetor de IDs de processos
     * @return Mapa de PID -> ProcessInfo
     */
    std::map<DWORD, ProcessInfo> analyzeProcesses(const std::vector<DWORD>& pids) const;

    /**
     * @brief Encontra o processo com maior consumo de memória
     * @return ProcessInfo do processo com maior uso de RAM
     */
    ProcessInfo findHighestMemoryProcess() const;

    /**
     * @brief Encontra processos com maior consumo de memória
     * @param top Número de processos top a retornar
     * @return Vetor dos top processos por consumo de memória
     */
    std::vector<ProcessInfo> findTopProcessesByMemory(int top) const;

    /**
     * @brief Calcula métricas de agregação
     * @return Struct com métricas agregadas
     */
    struct AggregateMetrics
    {
        SIZE_T totalProcessMemory;           ///< Total RAM de todos os processos
        SIZE_T totalPrivateMemory;           ///< Total RAM privada
        SIZE_T totalThreads;                  ///< Total de threads
        int processCount;                     ///< Número de processos
        int threadCount;                      ///< Número de threads
    } getAggregateMetrics() const;

private:
    bool isValidProcess(HANDLE hProcess) const;
    bool getProcessMemoryInfo(HANDLE hProcess, PROCESS_MEMORY_COUNTERS_EX& memInfo) const;
};

} // namespace Process
} // namespace RAMFlux

#endif // RAMFLUX_PROCESS_PROCESSANALYZER_H