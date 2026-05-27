# RAMFlux - Sistema de Análise de Fluxo de Memória

## 📋 Visão Geral

RAMFlux é uma aplicação desktop de análise de fluxo de memória, desenvolvida com **Qt6** e **C++20**. A aplicação analisa processos, monitora uso de memória e gera telemetria em tempo real.

## 🏗️ Arquitetura

```
RAMFlux/
├── CMakeLists.txt          # Configuração do CMake
├── build.bat               # Build script Windows
├── build.ps1               # Build script PowerShell
├── release/                # Arquivos de distribuição
├── assets/                 # Imagens e recursos
├── src/
│   ├── core/
│   │   ├── Version.h
│   │   └── Version.cpp
│   ├── telemetry/
│   │   ├── TelemetryEngine.h/.cpp
│   │   ├── FluxTelemetry.h/.cpp
│   │   ├── FluxCleaner.h/.cpp
│   │   ├── FluxPressureAnalyzer.h/.cpp
│   │   ├── PressureMonitor.h/.cpp
│   │   ├── MemoryMetrics.h/.cpp
│   │   └── WindowsMetrics.h/.cpp
│   ├── process/
│   │   ├── ProcessMonitor.h/.cpp
│   │   ├── ProcessAnalytics.h/.cpp
│   │   ├── ProcessSnapshot.h/.cpp
│   │   ├── IProcessObserver.h/.cpp
│   │   ├── ProcessMonitorObserver.h/.cpp
│   │   └── ProcessAnalyzer.h/.cpp
│   ├── ui/
│   │   ├── MainWindow.h/.cpp
│   │   └── dashboard/
│   │       ├── DashboardController.h/.cpp
│   │       ├── DashboardView.h/.cpp
│   │       ├── DashboardWidget.h/.cpp
│   │       ├── MemoryChartWidget.h/.cpp
│   │       └── SystemInfoCard.h/.cpp
│   ├── MemoryCollector.h/.cpp
│   ├── MemorySnapshot.h/.cpp
│   └── main.cpp
├── CMakeCache.txt          # Cache do CMake (gerado)
└── build/                  # Pasta de build (gerada)
```

## 📦 Requisitos

- **Windows 10/11** (64-bit)
- **Visual Studio 2022** (Community/Professional/Enterprise)
- **Qt 6.5+** (com MSVC 2019/2022)
- **CMake 3.20+**
- **Windows SDK** 10.0.19041+

## 🚀 Instalação

### 1. Instalar Dependências

Baixe e instale:
- [CMake](https://cmake.org/download/)
- [Qt6](https://www.qt.io/download) (versão com MSVC)
- [Visual Studio 2022](https://visualstudio.microsoft.com/)

### 2. Build do Projeto

**Opção A: Using PowerShell (Recomendado)**
```powershell
.\build.ps1
```

**Opção B: Using Batch File**
```cmd
build.bat
```

**Opção C: Usar CMake diretamente**
```cmd
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## 🎮 Uso

Após o build, os arquivos estarão em:

```
release/
├── RAMFlux.exe
├── Qt6Plugins/
└── *.dll
```

Execute:
```powershell
.\release\RAMFlux.exe
```

## 📖 Estrutura do Projeto

### Módulo de Dashboard
- **DashboardView** - Layout principal
- **DashboardController** - Lógica de controle
- **DashboardWidget** - Container do dashboard
- **MemoryChartWidget** - Gráficos de memória
- **SystemInfoCard** - Informações do sistema

### Módulo de Telemetria
- **TelemetryEngine** - Engine principal
- **FluxTelemetry** - Telemetria RAMFlux
- **FluxCleaner** - Limpeza de telemetria
- **FluxPressureAnalyzer** - Análise de pressão
- **PressureMonitor** - Monitor de pressão
- **MemoryMetrics** - Métricas de memória
- **WindowsMetrics** - Métricas do Windows

### Módulo de Processos
- **ProcessMonitor** - Monitor de processos
- **ProcessAnalyzer** - Análise de processos
- **ProcessSnapshot** - Captura de snapshot
- **ProcessAnalytics** - Análise avançada
- **IProcessObserver** - Interface de observer
- **ProcessMonitorObserver** - Observer do monitor

## 🔧 Customização

### Adicionar novos gráficos

1. Crie um arquivo `.cpp` no diretório `src/ui/dashboard/`
2. Adicione no `CMakeLists.txt`
3. Compile com `build.ps1`

### Configurar telemetria

Edite `src/telemetry/TelemetryEngine.cpp` para ajustar:
- Limite de métricas (`METRICS_LIMIT`)
- Taxa de atualização (`UPDATERATE`)

## 📝 Licença

Este projeto está disponível para uso pessoal e comercial.

## 📞 Suporte

Para dúvidas e suporte, entre em contato com o desenvolvedor.

---

**Versão Atual:** 1.0.0  
**Última Atualização:** 13/05/2026