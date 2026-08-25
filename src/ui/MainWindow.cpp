// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "MainWindow.h"
#include <QPointer>
#include <QTimer>
#include <QDateTime>
#include "core/FluxCore.h"
#include "core/EventBus.h"
#include "core/Logger.h"
#include "telemetry/FluxTelemetry.h"
#include "cleaner/FluxCleaner.h"
#include "optimizer/FluxOptimizer.h"
#include "profiles/ProfileManager.h"
#include "leakhunter/LeakHunter.h"
#include "gamemode/FluxGameMode.h"
#include "scheduler/FluxScheduler.h"
#include "ai/HeuristicEngine.h"
#include "ui/IoDashboardWidget.h"
#include "ui/PluginBrowserWidget.h"
#include "ui/BenchmarkWidget.h"
#include "ui/DedupWidget.h"
#include "ui/ClassifierWidget.h"
#include "ui/PrefetchWidget.h"
#include "ui/QosWidget.h"
#include "ui/SchedulerDashboardWidget.h"
#include "ui/ResponsivenessWidget.h"
#include <QtConcurrent>
#include <QFutureWatcher>
#include "ui/HibernateWidget.h"
#include "ui/PowerPlanWidget.h"
#include "ui/StandbyWidget.h"
#include "ui/ForecastWidget.h"
#include "SettingsDialog.h"
#include "shared/Constants.h"
#include "shared/StartupMarker.h"
#include "ui/ThemeManager.h"
#include "power/PowerManager.h"
#include "io/ConfigIO.h"
#include "io/UpdateChecker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QScrollArea>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>
#include <QStatusBar>
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <windows.h>

#include "ntapi/FluxNTAPI.h"
#include "helper/HelperClient.h"
#include <QMenuBar>
#include <QAction>
#include <QDesktopServices>
#include <QFileDialog>
#include <QUrl>
#include <QDateTime>
#include <QSettings>
#include <QDialog>
#include <QTextEdit>
#include <QPointer>
#include <QFile>
#include <QEvent>
#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>

#include <cmath>
namespace RAMFlux::UI {
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1100, 760);
    if(auto* screen = QApplication::primaryScreen()) {
        auto geo = screen->availableGeometry();
        resize(std::min(width(), geo.width()), std::min(height(), geo.height()));
        move(geo.topLeft());
    }
    markStartup("MainWindow: constructor start");
    m_trayManager = new SystemTrayManager(this);
    m_trayManager->initialize();
    markStartup("MainWindow: setupUI start");
    setupUI();
    markStartup("MainWindow: setupUI done");
    setupStyleSheet();
    setupConnections();
    loadSavedSettings();
    markStartup("MainWindow: connections done");
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    if(telemetry) {
        onMemoryUpdated();
    }
    m_uiTimer = new QTimer(this);
    connect(m_uiTimer, &QTimer::timeout, this, &MainWindow::onMemoryUpdated);
    m_uiTimer->start(2000);
    m_statsTimer = new QTimer(this);
    connect(m_statsTimer, &QTimer::timeout, this, [this]() {
        markStartup("statsTimer fired");
        onCleanStatsUpdate();
        QSettings s(QSettings::IniFormat, QSettings::UserScope, "RAMFlux", "RAMFlux");
        if(s.value("autoGameDetect", true).toBool()) {
            auto* watcher = new QFutureWatcher<bool>(this);
            connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
                bool isFs = watcher->result();
                watcher->deleteLater();
                if(isFs) {
                    m_gameModeLabel->setText("GAME DETECTED");
                    m_gameModeLabel->setStyleSheet("color: #f38ba8; padding: 2px 8px; font-weight: bold;");
                } else if(m_gameModeLabel->text() == "GAME DETECTED") {
                    m_gameModeLabel->setText("");
                }
            });
            watcher->setFuture(QtConcurrent::run([]() { return NtApi::isFullScreenAppActive(); }));
        }
    });
    m_statsTimer->start(5000);
    m_memoryMapTimer = new QTimer(this);
    connect(m_memoryMapTimer, &QTimer::timeout, this, &MainWindow::onMemoryMapUpdated);
    m_memoryMapTimer->start(10000);
    auto* powerMgr = dynamic_cast<Power::PowerManager*>(
        Core::FluxCore::instance().moduleManager().getModule("PowerManager"));
    if(powerMgr) {
        updateBatteryDisplay(powerMgr->currentState());
        m_powerStateSubId = powerMgr->onPowerStateChanged(
            [wp = QPointer<MainWindow>(this)](const Power::PowerState& state) {
                QMetaObject::invokeMethod(wp, [wp, state]() {
                    if(!wp) return;
                    wp->updateBatteryDisplay(state);
                    bool nowOnBattery = !state.onAC;
                    if(nowOnBattery != wp->m_lowPowerMode) {
                        wp->m_lowPowerMode = nowOnBattery;
                        wp->updatePollingIntervals(nowOnBattery);
                    }
                });
            });
    }
    if(!Helper::isHelperRunning()) {
        Core::Logger::instance().info("[MainWindow] Helper not running, deferring launch attempt");
        QTimer::singleShot(3000, this, [this]() {
            markStartup("helper launch timer fired");
            if(!Helper::isHelperRunning()) {
                Core::Logger::instance().info("[MainWindow] Attempting helper launch (may prompt UAC once)");
                markStartup("about to call launchHelper");
                Helper::launchHelper();
                markStartup("launchHelper returned");
                if(!Helper::isHelperRunning()) {
                    Core::Logger::instance().info("[MainWindow] Waiting for helper...");
                    auto* pollTimer = new QTimer(this);
                    pollTimer->setProperty("retries", 0);
                    connect(pollTimer, &QTimer::timeout, this, [this, pollTimer]() {
                        if(Helper::isHelperRunning()) {
                            Core::Logger::instance().info("[MainWindow] Helper is now running");
                            pollTimer->stop();
                            pollTimer->deleteLater();
                            return;
                        }
                        int retries = pollTimer->property("retries").toInt() + 1;
                        pollTimer->setProperty("retries", retries);
                        if(retries >= 20) {
                            Core::Logger::instance().error("[MainWindow] Helper failed to start within timeout");
                            pollTimer->stop();
                            pollTimer->deleteLater();
                        }
                    });
                    pollTimer->start(250);
                }
            }
        });
    } else {
        Core::Logger::instance().info("[MainWindow] Helper is running");
    }
    QSettings s(QSettings::IniFormat, QSettings::UserScope, "RAMFlux", "RAMFlux");
    if(s.value("startupOptimize", true).toBool()) {
        int delaySec = s.value("startupDelaySec", 30).toInt();
        delaySec = std::clamp(delaySec, 5, 600);
        QPointer<MainWindow> weakThis(this);
        QTimer::singleShot(delaySec * 1000, this, [weakThis]() {
            if(!weakThis) return;
            auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(
                Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));
            if(!cleaner) return;
            cleaner->quickClean();
            if(!weakThis) return;
            weakThis->m_statusLabel->setText("Startup clean completed");
            QPointer<MainWindow> weak2(weakThis);
            QTimer::singleShot(5000, weakThis, [weak2]() {
                if(!weak2) return;
                weak2->m_statusLabel->setText("Ready");
            });
        });
    }
}

MainWindow::~MainWindow() {
    if(m_uiTimer) m_uiTimer->stop();
    if(m_statsTimer) m_statsTimer->stop();
    if(m_leakTimer) m_leakTimer->stop();
    if(m_memoryMapTimer) m_memoryMapTimer->stop();
    if(m_diagnosticsEngine) m_diagnosticsEngine->stop();
    Core::Logger::instance().setCallback(nullptr);
    if(m_memoryUpdatedSubId) Core::EventBus::instance().unsubscribe(Constants::EventType::MemoryUpdated, m_memoryUpdatedSubId);
    if(m_cleaningFinishedSubId) Core::EventBus::instance().unsubscribe(Constants::EventType::CleaningFinished, m_cleaningFinishedSubId);
    if(m_workloadChangedSubId) Core::EventBus::instance().unsubscribe(Constants::EventType::WorkloadChanged, m_workloadChangedSubId);
    if(m_anomalyDetectedSubId) Core::EventBus::instance().unsubscribe(Constants::EventType::AnomalyDetected, m_anomalyDetectedSubId);
    if(m_pressurePredictedSubId) Core::EventBus::instance().unsubscribe(Constants::EventType::PressurePredicted, m_pressurePredictedSubId);
    if(m_gameDetectedSubId) Core::EventBus::instance().unsubscribe(Constants::EventType::GameDetected, m_gameDetectedSubId);
    if(m_miningDetectedSubId) Core::EventBus::instance().unsubscribe(Constants::EventType::MiningDetected, m_miningDetectedSubId);
    if(m_powerStateSubId) {
        auto* pm = dynamic_cast<Power::PowerManager*>(
            Core::FluxCore::instance().moduleManager().getModule("PowerManager"));
        if(pm) pm->unsubscribe(m_powerStateSubId);
    }
    if(m_profileChangedToken) {
        auto* profileMgr = dynamic_cast<Profiles::ProfileManager*>(
            Core::FluxCore::instance().moduleManager().getModule("ProfileManager"));
        if(profileMgr) profileMgr->unsubscribeProfileChanged(m_profileChangedToken);
    }
}

void MainWindow::setupUI() {
    auto* centralWidget = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("color: #a6adc8; padding: 2px 8px;");
    m_cleanStatsLabel = new QLabel("", this);
    m_cleanStatsLabel->setStyleSheet("color: #585b70; padding: 2px 8px;");
    m_gameModeLabel = new QLabel("", this);
    m_gameModeLabel->setStyleSheet("color: #a6e3a1; padding: 2px 8px;");
    m_aiWorkloadLabel = new QLabel("", this);
    m_aiWorkloadLabel->setStyleSheet("color: #cba6f7; padding: 2px 8px; font-weight: bold;");
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);
    auto* dashboardTab = new QWidget();
    setupDashboardTab(dashboardTab);
    m_tabWidget->addTab(dashboardTab, "Dashboard");
    auto* processesTab = new QWidget();
    setupProcessesTab(processesTab);
    m_tabWidget->addTab(processesTab, "Processes");
    auto* leakTab = new QWidget();
    setupLeakHunterTab(leakTab);
    m_tabWidget->addTab(leakTab, "Leak Hunter");
    auto* mmTab = new QWidget();
    setupMemoryMapTab(mmTab);
    m_tabWidget->addTab(mmTab, "Memory Map");
    auto* infoTab = new QWidget();
    setupInfoTab(infoTab);
    m_tabWidget->addTab(infoTab, "System Info");
    auto* consoleTab = new QWidget();
    setupConsoleTab(consoleTab);
    m_tabWidget->addTab(consoleTab, "Console");
    auto* scheduler = dynamic_cast<Scheduler::FluxScheduler*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxScheduler"));
    if(scheduler) {
        scheduler->setIoMonitorEnabled(true);
        auto* ioTab = new IoDashboardWidget(scheduler);
        m_tabWidget->addTab(ioTab, "I/O");
    }
    m_heatmap = new MemoryHeatmapWidget();
    m_tabWidget->addTab(m_heatmap, "Heatmap");
    m_forecastWidget = new ForecastWidget();
    m_tabWidget->addTab(m_forecastWidget, "Forecast");
    m_diagnosticsEngine = std::make_shared<Diagnostics::DiagnosticsEngine>();
    m_diagnosticsEngine->setEnabled(true);
    m_healthWidget = new HealthDashboardWidget();
    m_healthWidget->setEngine(m_diagnosticsEngine);
    m_tabWidget->addTab(m_healthWidget, "Health");
    m_diagnosticsEngine->start();
    auto* pluginTab = new PluginBrowserWidget();
    m_tabWidget->addTab(pluginTab, "Plugins");
    auto* benchmarkTab = new BenchmarkWidget();
    m_tabWidget->addTab(benchmarkTab, "Benchmark");
    auto* dedupTab = new DedupWidget();
    m_tabWidget->addTab(dedupTab, "Dedup");
    auto* classTab = new ClassifierWidget();
    m_tabWidget->addTab(classTab, "Classifier");
    auto* prefetchTab = new PrefetchWidget();
    m_tabWidget->addTab(prefetchTab, "Prefetch");
    auto* qosTab = new QosWidget();
    m_tabWidget->addTab(qosTab, "QoS");
    auto* schedTab = new SchedulerDashboardWidget();
    m_tabWidget->addTab(schedTab, "Scheduler");
    mainLayout->addWidget(m_tabWidget);
    setCentralWidget(centralWidget);
    auto* fileMenu = menuBar()->addMenu("&File");
    auto* settingsAction = fileMenu->addAction("&Settings");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onOpenSettings);
    fileMenu->addSeparator();
    auto* exportAction = fileMenu->addAction("&Export Configuration...");
    connect(exportAction, &QAction::triggered, this, [this]() {
        auto path = QFileDialog::getSaveFileName(this, "Export Configuration",
            "RAMFlux_Config.json", "JSON Files (*.json)");
        if(path.isEmpty()) return;
        auto data = IO::collectExportData();
        if(IO::exportToFile(data, path)) {
            m_statusLabel->setText("Configuration exported");
            Core::Logger::instance().info("Configuration exported to " + path.toStdString());
        }
    });
    auto* importAction = fileMenu->addAction("&Import Configuration...");
    connect(importAction, &QAction::triggered, this, [this]() {
        auto path = QFileDialog::getOpenFileName(this, "Import Configuration",
            QString(), "JSON Files (*.json)");
        if(path.isEmpty()) return;
        IO::ExportData data;
        if(!IO::importFromFile(path, data)) return;
        IO::applyImportData(data);
        m_statusLabel->setText("Configuration imported — restart recommended");
        Core::Logger::instance().info("Configuration imported from " + path.toStdString());
    });
    fileMenu->addSeparator();
    auto* quitAction = fileMenu->addAction("&Quit");
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
    auto* toolsMenu = menuBar()->addMenu("&Tools");
    auto* smartAction = toolsMenu->addAction("Smart &Optimize");
    connect(smartAction, &QAction::triggered, this, &MainWindow::onSmartOptimize);
    auto* deepAction = toolsMenu->addAction("&Deep Clean");
    connect(deepAction, &QAction::triggered, this, &MainWindow::onDeepClean);
    auto* helpMenu = menuBar()->addMenu("&Help");
    auto* manualPtAction = helpMenu->addAction("Manual (Português)");
    connect(manualPtAction, &QAction::triggered, this, [this]() { showManual(":/manual_pt.txt"); });
    auto* manualEnAction = helpMenu->addAction("Manual (English)");
    connect(manualEnAction, &QAction::triggered, this, [this]() { showManual(":/manual_en.txt"); });
    helpMenu->addSeparator();
    auto* checkUpdateAction = helpMenu->addAction("&Check for Updates");
    connect(checkUpdateAction, &QAction::triggered, this, [this]() {
        auto* checker = new IO::UpdateChecker(this);
        connect(checker, &IO::UpdateChecker::updateCheckComplete, this,
            [this, checker](const IO::UpdateInfo& info) {
                if(info.available) {
                    auto btn = QMessageBox::information(this, "Update Available",
                        QString("Version %1 is available!\n\n%2\n\nCurrent: v%3")
                            .arg(info.latestVersion.toString())
                            .arg(QString::fromStdString(info.releaseName))
                            .arg(IO::UpdateChecker::currentVersion().toString()),
                        QMessageBox::Open | QMessageBox::Cancel);
                    if(btn == QMessageBox::Open && !info.downloadUrl.empty())
                        QDesktopServices::openUrl(QUrl(QString::fromStdString(info.downloadUrl)));
                } else if(!info.error.empty()) {
                    QMessageBox::warning(this, "Update Check Failed",
                        QString("Could not check for updates.\n%1")
                            .arg(QString::fromStdString(info.error)));
                } else {
                    QMessageBox::information(this, "Up to Date",
                        QString("RAMFlux v%1 is the latest version.")
                            .arg(IO::UpdateChecker::currentVersion().toString()));
                }
                checker->deleteLater();
            });
        checker->checkForUpdates();
    });
    helpMenu->addSeparator();
    auto* aboutAction = helpMenu->addAction("&About RAMFlux");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About RAMFlux",
            QString("RAMFlux - Memory Flux Analyzer\nVersion %1\n\n"
            "Real-time memory monitoring and optimization tool.").arg(Constants::APP_VERSION));
    });
    m_batteryLabel = new QLabel("", this);
    m_batteryLabel->setStyleSheet("color: #a6adc8; padding: 2px 8px; font-size: 11px;");
    statusBar()->addPermanentWidget(m_statusLabel);
    statusBar()->addPermanentWidget(m_cleanStatsLabel);
    statusBar()->addPermanentWidget(m_gameModeLabel);
    statusBar()->addPermanentWidget(m_batteryLabel);
    statusBar()->addPermanentWidget(m_aiWorkloadLabel);
}

void MainWindow::setupStyleSheet() {
    setStyleSheet(ThemeManager::instance().generateStylesheet());
}

void MainWindow::setupConnections() {
    m_memoryUpdatedSubId = Core::EventBus::instance().subscribe(Constants::EventType::MemoryUpdated,
        [wp = QPointer<MainWindow>(this)]() {
            if(!wp) return;
            QMetaObject::invokeMethod(wp, [wp]() { if(wp) wp->onMemoryUpdated(); }, Qt::QueuedConnection);
        });
    m_workloadChangedSubId = Core::EventBus::instance().subscribe(Constants::EventType::WorkloadChanged,
        [wp = QPointer<MainWindow>(this)]() {
            if(!wp) return;
            QMetaObject::invokeMethod(wp, [wp]() { if(wp) wp->updateAIInfo(); }, Qt::QueuedConnection);
        });
    m_anomalyDetectedSubId = Core::EventBus::instance().subscribe(Constants::EventType::AnomalyDetected,
        [wp = QPointer<MainWindow>(this)]() {
            if(!wp) return;
            QMetaObject::invokeMethod(wp, [wp]() { if(wp) wp->updateAIInfo(); }, Qt::QueuedConnection);
        });
    m_pressurePredictedSubId = Core::EventBus::instance().subscribe(Constants::EventType::PressurePredicted,
        [wp = QPointer<MainWindow>(this)]() {
            if(!wp) return;
            QMetaObject::invokeMethod(wp, [wp]() { if(wp) wp->updateAIInfo(); }, Qt::QueuedConnection);
        });
    m_cleaningFinishedSubId = Core::EventBus::instance().subscribe(Constants::EventType::CleaningFinished,
        [wp = QPointer<MainWindow>(this)]() {
            if(!wp) return;
            QMetaObject::invokeMethod(wp, [wp]() { if(wp) wp->onCleanStatsUpdate(); }, Qt::QueuedConnection);
        });
    m_gameDetectedSubId = Core::EventBus::instance().subscribe(Constants::EventType::GameDetected,
        [wp = QPointer<MainWindow>(this)]() {
            if(!wp) return;
            QMetaObject::invokeMethod(wp, [wp]() { if(wp && wp->m_historyChart) wp->m_historyChart->addEventMarker("GAME", QDateTime::currentDateTime()); }, Qt::QueuedConnection);
        });
    m_miningDetectedSubId = Core::EventBus::instance().subscribe(Constants::EventType::MiningDetected,
        [wp = QPointer<MainWindow>(this)]() {
            if(!wp) return;
            QMetaObject::invokeMethod(wp, [wp]() { if(wp && wp->m_historyChart) wp->m_historyChart->addEventMarker("MINER", QDateTime::currentDateTime()); }, Qt::QueuedConnection);
        });
    connect(m_trayManager, &SystemTrayManager::showDashboardRequested, this, [this]() {
        showNormal(); activateWindow(); raise();
    });
    connect(m_trayManager, &SystemTrayManager::smartOptimizeRequested, this, &MainWindow::onSmartOptimize);
    connect(m_trayManager, &SystemTrayManager::deepCleanRequested, this, &MainWindow::onDeepClean);
    connect(m_trayManager, &SystemTrayManager::autoOptimizeToggled, this, [this](bool enabled) {
        onToggleAutomation(enabled);
    });
    connect(m_trayManager, &SystemTrayManager::gameModeToggled, this, [this](bool enabled) {
        if(enabled) m_gameModeLabel->setText("GAMING MODE");
        else m_gameModeLabel->setText("");
    });
    connect(m_trayManager, &SystemTrayManager::exitRequested, this, []() {
        QApplication::quit();
    });
}

void MainWindow::setupDashboardTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(4);
    auto* cardGrid = new QWidget();
    auto* grid = new QHBoxLayout(cardGrid);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(4);
    m_usedRamCard = new MemoryCard("Used RAM", "---", "GB");
    grid->addWidget(m_usedRamCard);
    m_freeRamCard = new MemoryCard("Free RAM", "---", "GB");
    grid->addWidget(m_freeRamCard);
    m_memoryLoadCard = new MemoryCard("Memory Load", "---", "%");
    grid->addWidget(m_memoryLoadCard);
    m_pressureCard = new MemoryCard("Pressure", "---", "");
    grid->addWidget(m_pressureCard);
    m_commitCard = new MemoryCard("Commit", "---", "GB");
    grid->addWidget(m_commitCard);
    layout->addWidget(cardGrid);
    auto* cardGrid2 = new QWidget();
    auto* grid2 = new QHBoxLayout(cardGrid2);
    grid2->setContentsMargins(0, 0, 0, 0);
    grid2->setSpacing(4);
    m_standbyCard = new MemoryCard("Standby", "---", "GB");
    grid2->addWidget(m_standbyCard);
    m_compressedCard = new MemoryCard("Compressed", "---", "GB");
    grid2->addWidget(m_compressedCard);
    m_modifiedCard = new MemoryCard("Modified", "---", "GB");
    grid2->addWidget(m_modifiedCard);
    m_cpuCard = new MemoryCard("CPU", "---", "%");
    grid2->addWidget(m_cpuCard);
    m_hardFaultsCard = new MemoryCard("Hard Faults", "---", "/sec");
    grid2->addWidget(m_hardFaultsCard);
    m_processCard = new MemoryCard("Processes", "---", "");
    grid2->addWidget(m_processCard);
    layout->addWidget(cardGrid2);
    m_fileCacheLabel = new QLabel("File Cache: ---");
    m_fileCacheLabel->setStyleSheet("color: #a6adc8; font-size: 11px; padding: 2px 6px;");
    layout->addWidget(m_fileCacheLabel);
    m_historyChart = new HistoryChart();
    layout->addWidget(m_historyChart, 1);
    m_explainLabel = new QLabel("—", this);
    m_explainLabel->setStyleSheet("color: #a6adc8; font-size: 11px; font-style: italic; padding: 2px 6px; background: #181825; border-radius: 4px;");
    m_explainLabel->setWordWrap(true);
    layout->addWidget(m_explainLabel);
    auto* actionBar = new QHBoxLayout();
    actionBar->setSpacing(8);
    auto* optimizeBtn = new QPushButton("Smart Optimize");
    optimizeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #89b4fa; color: #1e1e2e;
            border: none; border-radius: 6px;
            padding: 8px 20px; font-weight: bold; font-size: 13px;
        }
        QPushButton:hover { background-color: #74c7ec; }
    )");
    connect(optimizeBtn, &QPushButton::clicked, this, &MainWindow::onSmartOptimize);
    actionBar->addWidget(optimizeBtn);
    auto* deepCleanBtn = new QPushButton("Deep Clean");
    deepCleanBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #f38ba8; color: #1e1e2e;
            border: none; border-radius: 6px;
            padding: 8px 20px; font-weight: bold; font-size: 13px;
        }
        QPushButton:hover { background-color: #eba0ac; }
    )");
    connect(deepCleanBtn, &QPushButton::clicked, this, &MainWindow::onDeepClean);
    actionBar->addWidget(deepCleanBtn);
    actionBar->addStretch();
    auto* autoCheck = new QCheckBox("Auto-Optimize");
    autoCheck->setStyleSheet("color: #a6adc8; font-size: 12px;");
    connect(autoCheck, &QCheckBox::toggled, this, &MainWindow::onToggleAutomation);
    autoCheck->setChecked(true);
    actionBar->addWidget(autoCheck);
    m_profileLabel = new QLabel("Profile:");
    m_profileLabel->setStyleSheet("color: #a6adc8; font-size: 12px;");
    actionBar->addWidget(m_profileLabel);
    m_profileCombo = new QComboBox();
    m_profileCombo->setStyleSheet(R"(
        QComboBox {
            background-color: #313244; color: #cdd6f4;
            border: 1px solid #45475a; border-radius: 4px;
            padding: 4px 8px;
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView {
            background-color: #313244; color: #cdd6f4;
            selection-background-color: #45475a;
        }
    )");
    m_profileCombo->addItem("Economy");
    m_profileCombo->addItem("Balanced");
    m_profileCombo->addItem("Performance");
    m_profileCombo->addItem("Gaming");
    m_profileCombo->addItem("Mining");
    m_profileCombo->addItem("Custom");
    connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onProfileChanged);
    auto* profileMgr = dynamic_cast<Profiles::ProfileManager*>(
        Core::FluxCore::instance().moduleManager().getModule("ProfileManager"));
    if(profileMgr) {
        m_profileChangedToken = profileMgr->onProfileChanged(
            [wp = QPointer<MainWindow>(this)](Constants::ProfileType profile) {
                if(wp) wp->applyProfileConfig(profile);
            });
    }
    layout->addLayout(actionBar);
}

void MainWindow::setupProcessesTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(8, 8, 8, 8);
    m_processList = new ProcessListWidget(tab);
    layout->addWidget(m_processList);
}

void MainWindow::setupLeakHunterTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);
    auto* headerLayout = new QHBoxLayout();
    auto* desc = new QLabel(
        "Leak Hunter monitors processes for abnormal memory growth.\n"
        "Highlights processes with accelerating private allocations.");
    desc->setStyleSheet("color: #a6adc8; font-size: 12px; padding: 4px;");
    desc->setWordWrap(true);
    headerLayout->addWidget(desc, 1);
    m_leakInfoLabel = new QLabel("No leaks detected");
    m_leakInfoLabel->setStyleSheet("color: #a6e3a1; font-size: 13px; font-weight: bold; padding: 4px;");
    headerLayout->addWidget(m_leakInfoLabel);
    layout->addLayout(headerLayout);
    auto* controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(8);
    auto* refreshBtn = new QPushButton("Refresh");
    refreshBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #313244; color: #cdd6f4;
            border: 1px solid #45475a; border-radius: 4px;
            padding: 4px 12px;
        }
        QPushButton:hover { background-color: #45475a; }
    )");
    connect(refreshBtn, &QPushButton::clicked, this, [this]() { onMemoryUpdated(); });
    controlsLayout->addWidget(refreshBtn);
    auto* toggleLeak = new QCheckBox("Leak Detection");
    toggleLeak->setStyleSheet("color: #a6adc8; font-size: 12px;");
    toggleLeak->setChecked(true);
    connect(toggleLeak, &QCheckBox::toggled, this, [](bool enabled) {
        auto* hunter = dynamic_cast<LeakHunter::LeakHunter*>(
            Core::FluxCore::instance().moduleManager().getModule("LeakHunter"));
        if(hunter) hunter->setEnabled(enabled);
    });
    controlsLayout->addWidget(toggleLeak);
    controlsLayout->addStretch();
    layout->addLayout(controlsLayout);
    m_leakTable = new QTableWidget(this);
    m_leakTable->setColumnCount(9);
    m_leakTable->setHorizontalHeaderLabels({
        "PID", "Process", "WS Growth", "Private Growth", "Accel",
        "Heap Regions", "Heap Committed", "WS Peak", "Status"
    });
    m_leakTable->horizontalHeader()->setStretchLastSection(true);
    m_leakTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_leakTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_leakTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_leakTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_leakTable->setAlternatingRowColors(true);
    m_leakTable->verticalHeader()->setVisible(false);
    m_leakTable->setStyleSheet(R"(
        QTableWidget {
            background-color: #181825; color: #cdd6f4;
            border: 1px solid #313244; border-radius: 6px;
            gridline-color: #313244;
        }
        QTableWidget::item { padding: 4px 8px; }
        QTableWidget::item:alternate { background-color: #181825; }
        QTableWidget::item:selected { background-color: #45475a; }
        QHeaderView::section {
            background-color: #1e1e2e; color: #a6adc8;
            border: none; border-bottom: 1px solid #313244;
            padding: 6px 8px; font-weight: bold;
        }
    )");
    layout->addWidget(m_leakTable, 1);
    m_leakTimer = new QTimer(this);
    connect(m_leakTimer, &QTimer::timeout, this, [this]() {
        auto* hunter = dynamic_cast<LeakHunter::LeakHunter*>(
            Core::FluxCore::instance().moduleManager().getModule("LeakHunter"));
        if(!hunter) return;
        auto leaks = hunter->currentReports();
        m_leakTable->setRowCount(static_cast<int>(leaks.size()));
        uint64_t totalWasted = 0;
        int activeCount = 0;
        for(int i = 0; i < static_cast<int>(leaks.size()); ++i) {
            const auto& l = leaks[i];
            auto* pidItem = new QTableWidgetItem(QString::number(l.pid));
            pidItem->setTextAlignment(Qt::AlignCenter);
            m_leakTable->setItem(i, 0, pidItem);
            m_leakTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdWString(l.processName)));
            auto* wsItem = new QTableWidgetItem(formatBytes(l.growthBytes));
            wsItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_leakTable->setItem(i, 2, wsItem);
            auto* privItem = new QTableWidgetItem(formatBytes(l.privateGrowthBytes));
            privItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_leakTable->setItem(i, 3, privItem);
            auto* accelItem = new QTableWidgetItem(QString::number(l.accelFactor, 'f', 2));
            accelItem->setTextAlignment(Qt::AlignCenter);
            m_leakTable->setItem(i, 4, accelItem);
            auto* hrItem = new QTableWidgetItem(l.heapRegionCount > 0 ? QString::number(l.heapRegionCount) : "-");
            hrItem->setTextAlignment(Qt::AlignCenter);
            m_leakTable->setItem(i, 5, hrItem);
            auto* hcItem = new QTableWidgetItem(l.heapCommittedBytes > 0 ? formatBytes(l.heapCommittedBytes) : "-");
            hcItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_leakTable->setItem(i, 6, hcItem);
            auto* pkItem = new QTableWidgetItem(formatBytes(l.peakWorkingSet));
            pkItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_leakTable->setItem(i, 7, pkItem);
            QString status;
            QString rowColor;
            if(l.privateGrowthBytes > l.growthBytes * 2 && l.privateGrowthBytes >= 50ULL * 1024 * 1024) {
                status = "⚠ PRIVATE LEAK";
                rowColor = "#f38ba8";
                ++activeCount;
            } else if(l.accelFactor > 1.5 && l.privateGrowthBytes >= 30ULL * 1024 * 1024) {
                status = "⚠ ACCELERATING";
                rowColor = "#f38ba8";
                ++activeCount;
            } else if(l.accelFactor > 1.2) {
                status = "⚠ WATCHING";
                rowColor = "#f9e2af";
            } else if(l.heapRegionCount >= 1000) {
                status = "⚠ FRAGMENTED";
                rowColor = "#f9e2af";
            } else {
                status = "stable";
                rowColor = "#a6adc8";
            }
            auto* stItem = new QTableWidgetItem(status);
            stItem->setForeground(QColor(rowColor));
            stItem->setTextAlignment(Qt::AlignCenter);
            m_leakTable->setItem(i, 8, stItem);
            totalWasted += (l.growthBytes > l.privateGrowthBytes) ? l.growthBytes : l.privateGrowthBytes;
        }
        if(activeCount > 0) {
            m_leakInfoLabel->setText(QString("⚠ %1 active leak(s) — %2 wasted")
                .arg(activeCount).arg(formatBytes(totalWasted)));
            m_leakInfoLabel->setStyleSheet("color: #f38ba8; font-size: 13px; font-weight: bold; padding: 4px;");
        } else if(!leaks.empty()) {
            m_leakInfoLabel->setText(QString("%1 processes monitored — no active leaks").arg(leaks.size()));
            m_leakInfoLabel->setStyleSheet("color: #f9e2af; font-size: 13px; font-weight: bold; padding: 4px;");
        } else {
            m_leakInfoLabel->setText("No leaks detected");
            m_leakInfoLabel->setStyleSheet("color: #a6e3a1; font-size: 13px; font-weight: bold; padding: 4px;");
        }
    });
    m_leakTimer->start(5000);
}

void MainWindow::setupInfoTab(QWidget* tab) {
    auto* outerLayout = new QVBoxLayout(tab);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    auto* scrollArea = new QScrollArea(tab);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    auto* scrollContent = new QWidget();
    auto* layout = new QVBoxLayout(scrollContent);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);
    auto* sysGroup = new QGroupBox("System Information");
    auto* sysLayout = new QVBoxLayout(sysGroup);
    m_sysInfoLabel = new QLabel("Loading system info...");
    sysLayout->addWidget(m_sysInfoLabel);
    layout->addWidget(sysGroup);
    auto* perfGroup = new QGroupBox("Performance Tips");
    auto* perfLayout = new QVBoxLayout(perfGroup);
    auto* perfTip = new QLabel(
        "- Close unused applications to free memory\n"
        "- Disable startup programs you don't need\n"
        "- Use 'Deep Clean' to clear standby memory\n"
        "- Enable 'Gaming' profile for optimal gaming performance");
    perfLayout->addWidget(perfTip);
    layout->addWidget(perfGroup);
    auto* aiGroup = new QGroupBox("AI Heuristics");
    auto* aiLayout = new QVBoxLayout(aiGroup);
    m_aiInfoLabel = new QLabel("Waiting for AI data...");
    aiLayout->addWidget(m_aiInfoLabel);
    layout->addWidget(aiGroup);
    auto* numaGroup = new QGroupBox("NUMA Nodes");
    auto* numaLayout = new QVBoxLayout(numaGroup);
    m_numaInfoLabel = new QLabel("Collecting NUMA data...");
    numaLayout->addWidget(m_numaInfoLabel);
    layout->addWidget(numaGroup);
    auto* compGroup = new QGroupBox("Memory Compression");
    auto* compLayout = new QVBoxLayout(compGroup);
    m_compressionStatusLabel = new QLabel("Analyzing compression...");
    compLayout->addWidget(m_compressionStatusLabel);
    layout->addWidget(compGroup);
    layout->addWidget(new ResponsivenessWidget());
    layout->addWidget(new HibernateWidget());
    layout->addWidget(new PowerPlanWidget());
    layout->addWidget(new StandbyWidget());
    auto* aboutGroup = new QGroupBox("About RAMFlux");
    auto* aboutLayout = new QVBoxLayout(aboutGroup);
    auto* aboutLabel = new QLabel(
        QString("RAMFlux v%1\n"
        "Memory Flux Analyzer & Optimizer\n"
        "Built with Qt 6 and MinGW").arg(Constants::APP_VERSION));
    aboutLayout->addWidget(aboutLabel);
    layout->addWidget(aboutGroup);
    layout->addStretch();
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    if(telemetry) {
        auto snap = telemetry->lastSnapshot();
        QString infoText = QString(
            "Total RAM: %1 GB\n"
            "Kernel Memory: %2 GB (Paged: %3 GB | Nonpaged: %4 GB)\n"
            "Virtual Memory: %5 / %6 GB\n"
            "Page File: %7 / %8 GB\n"
            "Compression Savings: %9 GB (%10%%)")
            .arg(snap.totalRamGB(), 0, 'f', 2)
            .arg(snap.kernelMemory / (1024.0 * 1024 * 1024), 0, 'f', 2)
            .arg(snap.kernelPaged / (1024.0 * 1024 * 1024), 0, 'f', 2)
            .arg(snap.kernelNonpaged / (1024.0 * 1024 * 1024), 0, 'f', 2)
            .arg(snap.usedVirtual / (1024.0 * 1024 * 1024), 0, 'f', 2)
            .arg(snap.totalVirtual / (1024.0 * 1024 * 1024), 0, 'f', 2)
            .arg(snap.usedPageFile / (1024.0 * 1024 * 1024), 0, 'f', 2)
            .arg(snap.totalPageFile / (1024.0 * 1024 * 1024), 0, 'f', 2)
            .arg(snap.compressionSavingsGB(), 0, 'f', 2)
            .arg(snap.compressionSavingsPercent(), 0, 'f', 1);
        m_sysInfoLabel->setText(infoText);
    }
    scrollArea->setWidget(scrollContent);
    outerLayout->addWidget(scrollArea);
}

void MainWindow::setupConsoleTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 0, 0, 0);
    m_consoleWidget = new ConsoleWidget(tab);
    layout->addWidget(m_consoleWidget);
    auto& logger = Core::Logger::instance();
    logger.setCallback([wp = QPointer<MainWindow>(this)](Core::LogLevel level, const std::string& msg) {
        QMetaObject::invokeMethod(qApp, [wp, level, msg]() {
            if(wp && wp->m_consoleWidget) {
                wp->m_consoleWidget->appendLog(level, msg);
            }
        }, Qt::QueuedConnection);
    });
}

void MainWindow::onMemoryUpdated() {
    markStartup("onMemoryUpdated begin");
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    if(!telemetry) return;
    markStartup("onMemoryUpdated got telemetry");
    auto snap = telemetry->lastSnapshot();
    markStartup("onMemoryUpdated got snapshot");
    auto usedGB = snap.usedRamGB();
    auto freeGB = snap.freeRamGB();
    auto loadPct = snap.usedPercent();
    auto pressure = snap.pressureScore;
    auto commitGB = snap.committedMemory / (1024.0 * 1024 * 1024);
    auto standbyGB = snap.standbyRamGB();
    auto compressedGB = snap.compressedRamGB();
    auto modifiedGB = snap.modifiedMemory / (1024.0 * 1024 * 1024);
    auto totalGB = snap.totalRamGB();
    auto commitLimitGB = snap.commitLimit / (1024.0 * 1024 * 1024);
    m_usedRamCard->setValue(QString::number(usedGB, 'f', 2));
    m_freeRamCard->setValue(QString::number(freeGB, 'f', 2));
    m_memoryLoadCard->setValue(QString::number(loadPct, 'f', 1));
    m_pressureCard->setValue(QString::number(pressure));
    if(pressure > 70) {
        m_pressureCard->setColor(QColor("#f38ba8"));
    } else if(pressure > 40) {
        m_pressureCard->setColor(QColor("#f9e2af"));
    } else {
        m_pressureCard->setColor(QColor("#a6e3a1"));
    }
    m_commitCard->setValue(QString::number(commitGB, 'f', 2)
        + " / " + QString::number(commitLimitGB, 'f', 2));
    m_standbyCard->setValue(QString::number(standbyGB, 'f', 2));
    m_compressedCard->setValue(QString::number(compressedGB, 'f', 3));
    m_modifiedCard->setValue(QString::number(modifiedGB, 'f', 2));
    m_cpuCard->setValue(QString::number(snap.cpuUsage, 'f', 1));
    m_hardFaultsCard->setValue(QString::number(snap.hardFaultsPerSec));
    if(snap.hardFaultsPerSec > 10) {        m_hardFaultsCard->setColor(QColor("#f38ba8"));
    } else if(snap.hardFaultsPerSec > 3) {        m_hardFaultsCard->setColor(QColor("#f9e2af"));
    } else {        m_hardFaultsCard->setColor(QColor("#a6e3a1"));    }
    m_processCard->setValue(QString::number(snap.processCount));
    auto cacheGB = static_cast<double>(snap.cachedMemory) / (1024.0 * 1024 * 1024);
    m_fileCacheLabel->setText(QString("File Cache: %1 GB").arg(cacheGB, 0, 'f', 2));
    if(m_historyChart) m_historyChart->addDataPoint(usedGB, freeGB, loadPct,
        static_cast<double>(pressure), commitGB, standbyGB);
    if(m_explainLabel) {
        QString explain;
        if(pressure > 85) {
            QString topName = snap.topProcesses.empty() ? QStringLiteral("desconhecido") : QString::fromStdWString(snap.topProcesses[0].name);
            explain = QStringLiteral("Pressão %1% CRÍTICA → %2 faults/s + Standby %3GB + %4").arg(pressure).arg(snap.hardFaultsPerSec).arg(standbyGB,0,'f',1).arg(topName);
        } else if(pressure > 70) {
            explain = QStringLiteral("Pressão %1% alta → Standby %2GB + faults %3/s").arg(pressure).arg(standbyGB,0,'f',1).arg(snap.hardFaultsPerSec);
        } else if(pressure > 50) {
            explain = QStringLiteral("Pressão %1% moderada → Uso %2%").arg(pressure).arg(loadPct,0,'f',0);
        } else {
            explain = QStringLiteral("Pressão %1% normal — sistema saudável").arg(pressure);
        }
        // Append 60s prediction if confident
        if(auto* heuristic = dynamic_cast<AI::HeuristicEngine*>(Core::FluxCore::instance().moduleManager().getModule("HeuristicEngine"))) {
            auto rep = heuristic->currentReport();
            if(rep.predictionConfidence >= 0.5 && rep.predictedPressure60s >= 70.0) {
                explain += QStringLiteral(" | Previsto %1% em 60s (conf %2%)")
                    .arg(static_cast<int>(rep.predictedPressure60s))
                    .arg(static_cast<int>(rep.predictionConfidence*100));
                if(rep.predictedPressure60s >= 85.0) explain += QStringLiteral(" ⚠️");
            }
        }
        m_explainLabel->setText(explain);
    }
    Q_UNUSED(totalGB);
    //if(m_sysInfoLabel) {        QString infoText = QString(            "Total RAM: %1 GB\n"            "Kernel Memory: %2 GB (Paged: %3 GB | Nonpaged: %4 GB)\n"            "Virtual Memory: %5 / %6 GB\n"            "Page File: %7 / %8 GB\n"            "Compression Savings: %9 GB (%10%%)")            .arg(snap.totalRamGB(), 0, 'f', 2)            .arg(snap.kernelMemory / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.kernelPaged / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.kernelNonpaged / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.usedVirtual / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.totalVirtual / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.usedPageFile / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.totalPageFile / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.compressionSavingsGB(), 0, 'f', 2)            .arg(snap.compressionSavingsPercent(), 0, 'f', 1);        m_sysInfoLabel->setText(infoText);    }
    //m_processList->refreshProcessList();
    markStartup("onMemoryUpdated before heatmap");
    //if(m_heatmap) m_heatmap->updateProcesses(snap, snap.totalRam);
    markStartup("onMemoryUpdated before updateAIInfo");
    //updateAIInfo();
    markStartup("onMemoryUpdated after updateAIInfo");
    markStartup("onMemoryUpdated before numaBlock");
    //if(m_numaInfoLabel && snap.numaNodeCount > 0) {
    //    QString numaText;
    //    for(uint32_t i = 0; i < snap.numaNodeCount && i < static_cast<uint32_t>(snap.numaAvailableMemory.size()); ++i) {
    //        double gb = static_cast<double>(snap.numaAvailableMemory[i]) / (1024.0 * 1024 * 1024);
    //        numaText += QString("Node %1: %2 GB available\n").arg(i).arg(gb, 0, 'f', 1);
    //    }
    //    if(numaText.isEmpty()) numaText = "NUMA not available on this system";
    //    m_numaInfoLabel->setText(numaText.trimmed());
    //}
    markStartup("onMemoryUpdated before compressionBlock");
    //if(m_compressionStatusLabel) {
    //    double ratio = snap.compressionRatio;
    //    if(ratio > 0.0) {
    //        bool harmful = (ratio < Constants::COMPRESSION_RATIO_HARMFUL && snap.hardFaultsPerSec > 50);
    //        QString status = harmful
    //            ? QString("Ratio: %1x — ⚠ Low benefit with high faults, consider disabling")
    //                .arg(ratio, 0, 'f', 2)
    //            : QString("Ratio: %1x — OK").arg(ratio, 0, 'f', 2);
    //        m_compressionStatusLabel->setStyleSheet(harmful
    //            ? "color: #f38ba8; font-size: 12px;"
    //            : "color: #a6adc8; font-size: 12px;");
    //        m_compressionStatusLabel->setText(status);
    //    } else {
    //        m_compressionStatusLabel->setText("Memory compression not active");
    //    }
    //}
    markStartup("onMemoryUpdated function end");
}

void MainWindow::onSmartOptimize() {
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));
    if(telemetry && cleaner) {
        auto snap = telemetry->lastSnapshot();
        if(cleaner->quickClean()) {
            m_statusLabel->setText("Smart Optimize completed");
            onMemoryUpdated();
            QSettings s(QSettings::IniFormat, QSettings::UserScope, "RAMFlux", "RAMFlux");
            if(s.value("notifyClean", true).toBool()) {
                auto st = cleaner->stats();
                double mb = st.lastRecoveredBytes / (1024.0 * 1024.0);
                m_trayManager->showNotification("RAMFlux", QString("Smart Optimize: %1 MB recovered").arg(mb, 0, 'f', 1));
            }
        } else {
            m_statusLabel->setText("Smart Optimize: nothing to clean");
        }
    } else {
        m_statusLabel->setText("Smart Optimize: modules not available");
    }
}

void MainWindow::onDeepClean() {
    auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));
    if(cleaner) {
        bool ok = cleaner->deepClean();
        if(ok) {
            m_statusLabel->setText("Deep Clean completed");
            onMemoryUpdated();
            QSettings s(QSettings::IniFormat, QSettings::UserScope, "RAMFlux", "RAMFlux");
            if(s.value("notifyClean", true).toBool()) {
                auto st = cleaner->stats();
                double mb = st.lastRecoveredBytes / (1024.0 * 1024.0);
                m_trayManager->showNotification("RAMFlux", QString("Deep Clean: %1 MB recovered").arg(mb, 0, 'f', 1));
            }
        } else {
            m_statusLabel->setText("Deep Clean failed");
        }
    } else {
        m_statusLabel->setText("Deep Clean: module not available");
    }
}

void MainWindow::onToggleAutomation(bool enabled) {
    auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));
    auto* scheduler = dynamic_cast<Scheduler::FluxScheduler*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxScheduler"));
    if(cleaner && scheduler) {
        scheduler->setAutomationEnabled(enabled);
        scheduler->setPredictiveCleanEnabled(enabled);
        if(enabled) {
            cleaner->setCooldownMs(10000);
            scheduler->setIntervalMs(5000);
            m_statusLabel->setText("Auto-Optimize enabled");
        } else {
            cleaner->setCooldownMs(30000);
            m_statusLabel->setText("Auto-Optimize disabled");
        }
    }
}

void MainWindow::onProfileChanged(int index) {
    if(index < 0 || index > 5) index = 0;
    QSettings s(QSettings::IniFormat, QSettings::UserScope, "RAMFlux", "RAMFlux");
    s.setValue("profileIndex", index);
    auto* profileMgr = dynamic_cast<Profiles::ProfileManager*>(
        Core::FluxCore::instance().moduleManager().getModule("ProfileManager"));
    if(profileMgr) {
        auto profileType = static_cast<Constants::ProfileType>(index);
        profileMgr->setProfile(profileType);
        bool isGaming = (profileType == Constants::ProfileType::Gaming);
        m_gameModeLabel->setText(isGaming ? "GAMING PROFILE ACTIVE" : "");
        m_statusLabel->setText(QString("Profile changed to: %1")
            .arg(m_profileCombo->currentText()));
    }
}

void MainWindow::applyProfileConfig(Constants::ProfileType profile) {
    auto* profileMgr = dynamic_cast<Profiles::ProfileManager*>(
        Core::FluxCore::instance().moduleManager().getModule("ProfileManager"));
    if(!profileMgr) return;
    auto cfg = profileMgr->configForProfile(profile);
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    if(telemetry) telemetry->setPollingInterval(cfg.pollingIntervalMs);
    auto* scheduler = dynamic_cast<Scheduler::FluxScheduler*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxScheduler"));
    if(scheduler) scheduler->setIntervalMs(cfg.cooldownMs);
    auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));
    if(cleaner) {
        cleaner->setCooldownMs(cfg.cooldownMs);
        cleaner->setStandbyThresholdMB(cfg.standbyThresholdMB);
        cleaner->setFreeMemThresholdMB(cfg.freeMemThresholdMB);
    }
    auto* hunter = dynamic_cast<LeakHunter::LeakHunter*>(
        Core::FluxCore::instance().moduleManager().getModule("LeakHunter"));
    if(hunter) hunter->setEnabled(cfg.leakDetection);
    auto* gameMode = dynamic_cast<GameMode::FluxGameMode*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxGameMode"));
    if(gameMode) gameMode->setGameModeEnabled(cfg.gameMode);
    auto* optimizer = dynamic_cast<Optimizer::FluxOptimizer*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxOptimizer"));
    Q_UNUSED(optimizer);
}

void MainWindow::onOpenSettings() {
    auto* dialog = new SettingsDialog(this);
    connect(dialog, &SettingsDialog::profileChanged, this, [this](int idx) {
        // Update MainWindow's profile combo to match dialog (triggers currentIndexChanged → onProfileChanged)
        m_profileCombo->setCurrentIndex(idx);
    });
    connect(dialog, &SettingsDialog::automationToggled, this, [this](bool enabled) {
        onToggleAutomation(enabled);
    });
    connect(dialog, &SettingsDialog::gameModeToggled, this, [this](bool enabled) {
        if(enabled) m_gameModeLabel->setText("GAMING MODE");
        else m_gameModeLabel->setText("");
    });
    connect(dialog, &SettingsDialog::pollingIntervalChanged, this, [this](int ms) {
        m_uiTimer->setInterval(ms);
    });
    connect(dialog, &SettingsDialog::startupOptimizationChanged, this, [this](int delaySec) {
        // saveSettings() already writes these to QSettings - no redundant write needed
        Q_UNUSED(delaySec);
    });
    connect(dialog, &SettingsDialog::themeChanged, this, [this](int themeIndex) {
        auto& tmgr = ThemeManager::instance();
        tmgr.setTheme(themeIndex);
        tmgr.applyTo(this);
    });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::onCleanStatsUpdate() {
    markStartup("onCleanStatsUpdate begin");
    auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));
    if(cleaner) {
        auto stats = cleaner->stats();
        m_cleanStatsLabel->setText(QString("Recovered: %1 MB | Deep: %2 | Auto: %3")
            .arg(stats.bytesRecovered / (1024 * 1024))
            .arg(stats.deepCleanCount)
            .arg(stats.thresholdCleanCount));
    }
    markStartup("onCleanStatsUpdate end");
}

void MainWindow::showManual(const QString& resourcePath) {
    QFile file(resourcePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("RAMFlux Manual");
    dlg->resize(700, 550);
    auto* layout = new QVBoxLayout(dlg);
    auto* textEdit = new QTextEdit(dlg);
    textEdit->setPlainText(content);
    textEdit->setReadOnly(true);
    QFont monoFont("Consolas", 10);
    monoFont.setStyleHint(QFont::Monospace);
    textEdit->setFont(monoFont);
    textEdit->setLineWrapMode(QTextEdit::NoWrap);
    layout->addWidget(textEdit);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->show();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if(QSystemTrayIcon::isSystemTrayAvailable()) {
        hide();
        m_minimizedToTray = true;
        if(m_trayManager) {
            m_trayManager->showNotification("RAMFlux",
                "Application minimized to system tray.");
        }
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::changeEvent(QEvent* event) {
    if(event->type() == QEvent::WindowStateChange) {
        bool minimized = isMinimized();
        updatePollingIntervals(minimized || m_lowPowerMode);
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::updatePollingIntervals(bool lowPower) {
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    auto* scheduler = dynamic_cast<Scheduler::FluxScheduler*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxScheduler"));
    if(lowPower) {
        int tp = Constants::LOW_POWER_POLLING_INTERVAL_MS;
        if(telemetry) telemetry->setPollingInterval(tp);
        if(scheduler) scheduler->setIntervalMs(tp * 2);
        m_uiTimer->setInterval(5000);
        m_memoryMapTimer->setInterval(30000);
    } else {
        if(telemetry) telemetry->setPollingInterval(Constants::POLLING_INTERVAL_MS);
        if(scheduler) scheduler->setIntervalMs(Constants::OPTIMIZER_INTERVAL_MS);
        m_uiTimer->setInterval(2000);
        m_memoryMapTimer->setInterval(10000);
    }
}

void MainWindow::updateBatteryDisplay(const Power::PowerState& ps) {
    QString text;
    QString color;
    if(ps.onAC) {
        text = "AC";
        color = "#a6e3a1";
    } else {
        if(ps.charging) {
            text = QString("BAT %1% (chrg)").arg(ps.batteryPercent);
            color = "#f9e2af";
        } else if(ps.batteryPercent <= Constants::BATTERY_CRITICAL_THRESHOLD) {
            text = QString("BAT %1% CRIT").arg(ps.batteryPercent);
            color = "#f38ba8";
            Core::EventBus::instance().post(Constants::EventType::BatteryLow);
        } else if(ps.batteryPercent <= Constants::BATTERY_LOW_THRESHOLD) {
            text = QString("BAT %1% LOW").arg(ps.batteryPercent);
            color = "#fab387";
        } else {
            text = QString("BAT %1%").arg(ps.batteryPercent);
            color = "#a6adc8";
        }
    }
    m_batteryLabel->setText(text);
    m_batteryLabel->setStyleSheet(QString("color: %1; padding: 2px 8px; font-size: 11px;").arg(color));
}
void MainWindow::updateAIInfo() {
    auto* heuristic = dynamic_cast<AI::HeuristicEngine*>(
        Core::FluxCore::instance().moduleManager().getModule("HeuristicEngine"));
    if(!heuristic) return;
    auto report = heuristic->currentReport();
    double currentPressure = 0.0;
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    if(telemetry) currentPressure = telemetry->lastSnapshot().pressureScore;
    const char* names[] = { "Unknown", "Gaming", "Development", "Media", "Heavy", "Office", "Idle" };
    int wlIdx = static_cast<int>(report.workload);
    if(wlIdx < 0 || wlIdx > 6) wlIdx = 0;
    QString wlName = names[wlIdx];
    double confPct = report.workloadConfidence * 100.0;
    m_aiWorkloadLabel->setText(QString("AI: %1 (%2%)").arg(wlName).arg(confPct, 0, 'f', 0));
    QString pred30 = (report.predictionConfidence >= Constants::AI_PREDICTION_CONFIDENCE_MIN)
        ? QString::number(report.predictedPressure30s, 'f', 0) : "?";
    QString pred60 = (report.predictionConfidence >= Constants::AI_PREDICTION_CONFIDENCE_MIN)
        ? QString::number(report.predictedPressure60s, 'f', 0) : "?";
    QString pred120 = (report.predictionConfidence >= Constants::AI_PREDICTION_CONFIDENCE_MIN)
        ? QString::number(report.predictedPressure120s, 'f', 0) : "?";
    QString trend = (report.trendSlope > 0.1) ? "↑ rising" :
                    (report.trendSlope < -0.1) ? "↓ falling" : "→ stable";
    QString anomaly = report.anomalyDetected
        ? QString("⚠ ANOMALY DETECTED (σ=%1)").arg(report.anomalyDeviation, 0, 'f', 1)
        : "Normal pattern";
    QString infoText = QString(
        "Current Workload: %1 (conf: %2%) — %3\n"
        "Predicted Pressure: 30s=%4 | 60s=%5 | 120s=%6\n"
        "Trend: %7 (slope: %8/s)\n"
        "Memory Pattern: %9\n"
        "Samples: %10 | Conf: %11%")
        .arg(wlName).arg(confPct, 0, 'f', 0).arg(report.workloadTrigger.c_str())
        .arg(pred30).arg(pred60).arg(pred120)
        .arg(trend).arg(report.trendSlope, 0, 'f', 3)
        .arg(anomaly)
        .arg(report.totalSamples)
        .arg(report.predictionConfidence * 100.0, 0, 'f', 0);
    m_aiInfoLabel->setText(infoText);
    m_forecastWidget->updateForecast(
        currentPressure,
        report.predictedPressure30s,
        report.predictedPressure60s,
        report.predictedPressure120s,
        report.predictionConfidence,
        report.trendSlope,
        report.effectiveness.accuracy,
        report.mlScore,
        report.mlSampleCount,
        report.effectiveness.totalPredictions,
        report.effectiveness.correctPredictions,
        report.effectiveness.falsePositives,
        report.effectiveness.falseNegatives);
}
void MainWindow::loadSavedSettings() {
    auto& tmgr = ThemeManager::instance();
    QSettings s(QSettings::IniFormat, QSettings::UserScope, "RAMFlux", "RAMFlux");
    tmgr.setTheme(s.value("themeIndex", 0).toInt());
    tmgr.applyTo(this);
    auto* profileMgr = dynamic_cast<Profiles::ProfileManager*>(
        Core::FluxCore::instance().moduleManager().getModule("ProfileManager"));
    if(profileMgr) {
        int saved = s.value("profileIndex", static_cast<int>(profileMgr->activeProfile())).toInt();
        if(saved < 0 || saved > 5) saved = 1;
        auto profileType = static_cast<Constants::ProfileType>(saved);
        if(profileMgr->activeProfile() != profileType) profileMgr->setProfile(profileType);
        m_profileCombo->setCurrentIndex(saved);
    }
    bool autoOpt = s.value("autoOptimize", true).toBool();
    auto* dashboardPage = m_tabWidget->widget(0);
    auto* layout = dashboardPage ? dashboardPage->layout() : nullptr;
    auto* item = (layout && layout->count() > 1) ? layout->itemAt(1) : nullptr;
    auto* actionBar = item ? item->widget() : nullptr;
    if(actionBar) {
        auto checks = actionBar->findChildren<QCheckBox*>();
        for(auto* cb : checks) {
            if(cb->text() == "Auto-Optimize") {
                cb->setChecked(autoOpt);
                break;
            }
        }
    }
}

QString MainWindow::formatBytes(uint64_t bytes) {
    if(bytes >= 1099511627776ULL) return QString::number(bytes / 1099511627776.0, 'f', 2) + " TB";
    if(bytes >= 1073741824) return QString::number(bytes / 1073741824.0, 'f', 2) + " GB";
    if(bytes >= 1048576) return QString::number(bytes / 1048576.0, 'f', 1) + " MB";
    if(bytes >= 1024) return QString::number(bytes / 1024.0, 'f', 0) + " KB";
    return QString::number(bytes) + " B"; }

void MainWindow::setupMemoryMapTab(QWidget* tab) {    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(6);
    auto* header = new QLabel("Physical Memory Distribution\nPages grouped by current usage state");
    header->setStyleSheet("color: #a6adc8; font-size: 12px; padding: 4px;");
    layout->addWidget(header);
    struct BarDef { const char* name; const char* color; };
    const BarDef defs[] = {
        {"Active (in use)", "#89b4fa"}, {"Standby (cached)", "#a6e3a1"},
        {"Modified", "#f9e2af"}, {"Modified No-Write", "#f38ba8"},
        {"Transition", "#cba6f7"}, {"Zeroed", "#94e2d5"},
        {"Free", "#585b70"}, {"Bad", "#11111b"} };
    for(auto& d : defs) {        auto* row = new QWidget();
    auto* hl = new QHBoxLayout(row);        hl->setContentsMargins(0, 0, 0, 0);
    hl->setSpacing(8);
    auto* nl = new QLabel(d.name);        nl->setFixedWidth(140);
    nl->setStyleSheet("color: #cdd6f4; font-size: 11px;");        m_mmNameLabels.append(nl);
    hl->addWidget(nl);
    auto* bar = new QProgressBar();        bar->setRange(0, 100);
    bar->setTextVisible(false);        bar->setFixedHeight(18);
    bar->setStyleSheet(QString(R"(
        QProgressBar { background-color: #181825; border: 1px solid #313244; border-radius: 3px; }
        QProgressBar::chunk { background-color: %1; border-radius: 2px; }
    )").arg(d.color));        m_mmBars.append(bar);
    hl->addWidget(bar, 1);
    auto* vl = new QLabel("---");        vl->setFixedWidth(160);
    vl->setStyleSheet("color: #a6adc8; font-size: 11px;");        m_mmValueLabels.append(vl);
    hl->addWidget(vl);        layout->addWidget(row);    }    layout->addStretch(); }

void MainWindow::onMemoryMapUpdated() {
    markStartup("onMemoryMapUpdated begin");
    auto* watcher = new QFutureWatcher<decltype(NtApi::getPhysicalMemoryBreakdown())>(this);
    connect(watcher, &QFutureWatcher<decltype(NtApi::getPhysicalMemoryBreakdown())>::finished, this, [this, watcher]() {
        auto b = watcher->result();
        watcher->deleteLater();
        uint64_t total = b.totalPages;
        if(total == 0) {
            for(int i = 0; i < m_mmBars.size(); i++) { m_mmValueLabels[i]->setText("N/A"); m_mmBars[i]->setValue(0); }
            return;
        }
        uint64_t vals[] = { b.activePages, b.standbyPages, b.modifiedPages, b.modifiedNoWritePages, b.transitionPages, b.zeroPages, b.freePages, b.badPages };
        for(int i = 0; i < 8 && i < m_mmBars.size(); i++) {
            double pct = static_cast<double>(vals[i]) * 100.0 / total;
            m_mmBars[i]->setValue(static_cast<int>(pct));
            auto bytes = vals[i] * b.pageSize;
            m_mmValueLabels[i]->setText(QString("%1 (%2%)").arg(formatBytes(bytes)).arg(pct, 0, 'f', 1));
        }
        markStartup("onMemoryMapUpdated end");
    });
    watcher->setFuture(QtConcurrent::run([]() { return NtApi::getPhysicalMemoryBreakdown(); }));
}

} // namespace RAMFlux::UI

