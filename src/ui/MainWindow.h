// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QTabWidget>
#include <QFutureWatcher>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QVector>
#include <memory>
#include "shared/Constants.h"
#include "diagnostics/DiagnosticsEngine.h"
#include "MemoryCard.h"
#include "HistoryChart.h"
#include "ForecastWidget.h"
#include "ProcessListWidget.h"
#include "ConsoleWidget.h"
#include "MemoryHeatmapWidget.h"
#include "HealthDashboardWidget.h"
#include "SystemTrayManager.h"
#include "power/PowerManager.h"
#include "telemetry/MemorySnapshot.h"
namespace RAMFlux::Telemetry {
class FluxTelemetry; }
namespace RAMFlux::Cleaner {
class FluxCleaner; }
namespace RAMFlux::Optimizer {
class FluxOptimizer; }
namespace RAMFlux::Profiles {
class ProfileManager; }
namespace RAMFlux::LeakHunter {
class LeakHunter; }
namespace RAMFlux::AI {
class HeuristicEngine; }
namespace RAMFlux::UI {
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);    ~MainWindow() override;
    void showManual(const QString& resourcePath);
protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    private slots:    void onMemoryUpdated();
void onSmartOptimize();
void onDeepClean();
void onToggleAutomation(bool enabled);
void onProfileChanged(int index);
void onOpenSettings();
void onCleanStatsUpdate();
void onMemoryMapUpdated();
    private:
    void setupUI();
void setupStyleSheet();
void setupConnections();
void setupDashboardTab(QWidget* tab);
void setupProcessesTab(QWidget* tab);
void setupLeakHunterTab(QWidget* tab);
void setupMemoryMapTab(QWidget* tab);
void setupHealthTab(QWidget* tab);
void setupInfoTab(QWidget* tab);
void setupConsoleTab(QWidget* tab);
void loadSavedSettings();
    void applyProfileConfig(RAMFlux::Constants::ProfileType profile);
    static QString formatBytes(uint64_t bytes);    QTabWidget* m_tabWidget;    MemoryCard* m_usedRamCard;    MemoryCard* m_freeRamCard;    MemoryCard* m_memoryLoadCard;    MemoryCard* m_pressureCard;    MemoryCard* m_commitCard;    MemoryCard* m_standbyCard;    MemoryCard* m_cpuCard;    MemoryCard* m_hardFaultsCard;    MemoryCard* m_processCard;    MemoryCard* m_compressedCard;    MemoryCard* m_modifiedCard;        HistoryChart* m_historyChart;
    ForecastWidget* m_forecastWidget;
    ProcessListWidget* m_processList;    QLabel* m_fileCacheLabel;    SystemTrayManager* m_trayManager;    ConsoleWidget* m_consoleWidget;        MemoryHeatmapWidget* m_heatmap;
    HealthDashboardWidget* m_healthWidget;
    std::shared_ptr<Diagnostics::DiagnosticsEngine> m_diagnosticsEngine;
    QComboBox* m_profileCombo;    QLabel* m_statusLabel;    QLabel* m_cleanStatsLabel;    QLabel* m_gameModeLabel;    QLabel* m_profileLabel;    QTimer* m_uiTimer;    QTimer* m_statsTimer;    QTimer* m_memoryMapTimer;    QLabel* m_sysInfoLabel;    QVector<QLabel*> m_mmNameLabels;
QVector<QProgressBar*> m_mmBars;
QVector<QLabel*> m_mmValueLabels;    Telemetry::HistoryBuffer m_history;
    bool m_minimizedToTray{
false};
    bool m_lowPowerMode{
false};
    void updatePollingIntervals(bool lowPower);
    void updateAIInfo();
    void updateBatteryDisplay(const Power::PowerState& state);

    uint64_t m_memoryUpdatedSubId{0};
    uint64_t m_cleaningFinishedSubId{0};
    uint64_t m_workloadChangedSubId{0};
    uint64_t m_anomalyDetectedSubId{0};
    uint64_t m_pressurePredictedSubId{0};
    uint64_t m_powerStateSubId{0};
    QLabel* m_aiWorkloadLabel;
    QLabel* m_aiInfoLabel;
    QLabel* m_numaInfoLabel;
    QLabel* m_compressionStatusLabel;
    QLabel* m_batteryLabel;
    QTableWidget* m_leakTable;
    QLabel* m_leakInfoLabel;
    QTimer* m_leakTimer;
    uint64_t m_profileChangedToken{0};
};
} // namespace RAMFlux::UI


