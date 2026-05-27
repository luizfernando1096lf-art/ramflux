// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <QMainWindow>
#include <QTimer>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QVector>
#include <atomic>
#include <memory>
#include "MemoryCard.h"
#include "HistoryChart.h"
#include "ProcessListWidget.h"
#include "ConsoleWidget.h"
#include "SystemTrayManager.h"
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
namespace RAMFlux::UI {
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);    ~MainWindow() override;
    void showManual(const QString& resourcePath);
protected:
    void closeEvent(QCloseEvent* event) override;
    private slots:    void onMemoryUpdated();
void onSmartOptimize();
void onDeepClean();
void onToggleAutomation(bool enabled);
void onProfileChanged(int index);
void onOpenSettings();
void onCleanStatsUpdate();
void onFileCacheUpdated();
void onMemoryMapUpdated();
    private:
    void setupUI();
void setupStyleSheet();
void setupConnections();
void setupDashboardTab(QWidget* tab);
void setupProcessesTab(QWidget* tab);
void setupLeakHunterTab(QWidget* tab);
void setupFileCacheTab(QWidget* tab);
void setupMemoryMapTab(QWidget* tab);
void setupInfoTab(QWidget* tab);
void setupConsoleTab(QWidget* tab);
void loadSavedSettings();
static QString formatBytes(uint64_t bytes);    QTabWidget* m_tabWidget;    MemoryCard* m_usedRamCard;    MemoryCard* m_freeRamCard;    MemoryCard* m_memoryLoadCard;    MemoryCard* m_pressureCard;    MemoryCard* m_commitCard;    MemoryCard* m_standbyCard;    MemoryCard* m_cpuCard;    MemoryCard* m_hardFaultsCard;    MemoryCard* m_processCard;    MemoryCard* m_compressedCard;    MemoryCard* m_modifiedCard;    HistoryChart* m_historyChart;    ProcessListWidget* m_processList;    QLabel* m_fileCacheLabel;    SystemTrayManager* m_trayManager;    ConsoleWidget* m_consoleWidget;    QComboBox* m_profileCombo;    QLabel* m_statusLabel;    QLabel* m_cleanStatsLabel;    QLabel* m_gameModeLabel;    QLabel* m_profileLabel;    QTimer* m_uiTimer;    QTimer* m_statsTimer;    QTimer* m_fileCacheTimer;    QTimer* m_memoryMapTimer;    QTableWidget* m_fileCacheTable;    QLabel* m_sysInfoLabel;    QVector<QLabel*> m_mmNameLabels;
QVector<QProgressBar*> m_mmBars;
QVector<QLabel*> m_mmValueLabels;    Telemetry::HistoryBuffer m_history;
    bool m_minimizedToTray{
false};
    std::atomic<bool> m_fileCachePending{false};
};
} // namespace RAMFlux::UI


