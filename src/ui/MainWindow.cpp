// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "MainWindow.h"
#include "core/FluxCore.h"
#include "core/EventBus.h"
#include "core/Logger.h"
#include "telemetry/FluxTelemetry.h"
#include "cleaner/FluxCleaner.h"
#include "optimizer/FluxOptimizer.h"
#include "profiles/ProfileManager.h"
#include "leakhunter/LeakHunter.h"
#include "scheduler/FluxScheduler.h"
#include "SettingsDialog.h"
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
#include <QTableWidget>
#include <QHeaderView>
#include "ntapi/FluxNTAPI.h"
#include <QMenuBar>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QSettings>
#include <QDialog>
#include <QTextEdit>
#include <QFile>
#include <thread>
#include <atomic>
#include <cmath>
namespace RAMFlux::UI {
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    resize(1100, 760);
    m_trayManager = new SystemTrayManager(this);
    m_trayManager->initialize();
    setupUI();
    setupStyleSheet();
    setupConnections();
    loadSavedSettings();
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
        onCleanStatsUpdate();
        QSettings s("RAMFlux", "RAMFlux");
        if(s.value("autoGameDetect", true).toBool()) {
            if(NtApi::isFullScreenAppActive()) {
                m_gameModeLabel->setText("GAME DETECTED");
                m_gameModeLabel->setStyleSheet("color: #f38ba8; padding: 2px 8px; font-weight: bold;");
            } else if(m_gameModeLabel->text() == "GAME DETECTED") {
                m_gameModeLabel->setText("");
            }
        }
    });
    m_statsTimer->start(5000);
    m_fileCacheTimer = new QTimer(this);
    connect(m_fileCacheTimer, &QTimer::timeout, this, &MainWindow::onFileCacheUpdated);
    m_fileCacheTimer->start(15000);
    m_memoryMapTimer = new QTimer(this);
    connect(m_memoryMapTimer, &QTimer::timeout, this, &MainWindow::onMemoryMapUpdated);
    m_memoryMapTimer->start(10000);
    QSettings s("RAMFlux", "RAMFlux");
    if(s.value("startupOptimize", true).toBool()) {
        int delaySec = s.value("startupDelaySec", 30).toInt();
        auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(
            Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));
        if(cleaner) {
            QTimer::singleShot(delaySec * 1000, this, [this, cleaner]() {
                cleaner->quickClean();
                m_statusLabel->setText("Startup clean completed");
                QTimer::singleShot(5000, this, [this]() {
                    m_statusLabel->setText("Ready");
                });
            });
        }
    }
}

MainWindow::~MainWindow() {
    Core::Logger::instance().setCallback(nullptr);
}

void MainWindow::setupUI() {
    auto* centralWidget = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
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
    auto* fileCacheTab = new QWidget();
    setupFileCacheTab(fileCacheTab);
    m_tabWidget->addTab(fileCacheTab, "File Summary");
    auto* mmTab = new QWidget();
    setupMemoryMapTab(mmTab);
    m_tabWidget->addTab(mmTab, "Memory Map");
    auto* infoTab = new QWidget();
    setupInfoTab(infoTab);
    m_tabWidget->addTab(infoTab, "System Info");
    auto* consoleTab = new QWidget();
    setupConsoleTab(consoleTab);
    m_tabWidget->addTab(consoleTab, "Console");
    mainLayout->addWidget(m_tabWidget);
    setCentralWidget(centralWidget);
    auto* fileMenu = menuBar()->addMenu("&File");
    auto* settingsAction = fileMenu->addAction("&Settings");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onOpenSettings);
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
    auto* aboutAction = helpMenu->addAction("&About RAMFlux");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About RAMFlux",
            "RAMFlux - Memory Flux Analyzer\nVersion 1.1.0\n\n"
            "Real-time memory monitoring and optimization tool.");
    });
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("color: #a6adc8; padding: 2px 8px;");
    statusBar()->addPermanentWidget(m_statusLabel);
    m_cleanStatsLabel = new QLabel("", this);
    m_cleanStatsLabel->setStyleSheet("color: #585b70; padding: 2px 8px;");
    statusBar()->addPermanentWidget(m_cleanStatsLabel);
    m_gameModeLabel = new QLabel("", this);
    m_gameModeLabel->setStyleSheet("color: #a6e3a1; padding: 2px 8px;");
    statusBar()->addPermanentWidget(m_gameModeLabel);
}

void MainWindow::setupStyleSheet() {
    setStyleSheet(R"(
        QMainWindow { background-color: #11111b; }
        QTabWidget::pane { background-color: #1e1e2e; border: none; }
        QTabBar::tab {
            background-color: #181825; color: #585b70;
            padding: 8px 16px; border: none; border-right: 1px solid #313244;
            font-size: 12px;
        }
        QTabBar::tab:selected { background-color: #1e1e2e; color: #cdd6f4; }
        QTabBar::tab:hover { background-color: #313244; color: #cdd6f4; }
        QMenuBar { background-color: #181825; color: #cdd6f4; border-bottom: 1px solid #313244; }
        QMenuBar::item:selected { background-color: #313244; }
        QMenu { background-color: #1e1e2e; color: #cdd6f4; border: 1px solid #313244; }
        QMenu::item:selected { background-color: #313244; }
        QStatusBar { background-color: #181825; color: #a6adc8; border-top: 1px solid #313244; }
    )");
}

void MainWindow::setupConnections() {
    Core::EventBus::instance().subscribe(Constants::EventType::MemoryUpdated,
        [this]() { QMetaObject::invokeMethod(this, [this]() { onMemoryUpdated(); }, Qt::QueuedConnection); });
    Core::EventBus::instance().subscribe(Constants::EventType::CleaningFinished,
        [this]() { QMetaObject::invokeMethod(this, [this]() { onCleanStatsUpdate(); }, Qt::QueuedConnection); });
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
    autoCheck->setChecked(true);
    connect(autoCheck, &QCheckBox::toggled, this, &MainWindow::onToggleAutomation);
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
    m_profileCombo->addItem("Custom");
    connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onProfileChanged);
    actionBar->addWidget(m_profileCombo);
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
    auto* desc = new QLabel(
        "Leak Hunter detects processes with growing memory usage over time.\n"
        "Suspicious processes will be highlighted below.");
    desc->setStyleSheet("color: #a6adc8; font-size: 12px; padding: 4px;");
    desc->setWordWrap(true);
    layout->addWidget(desc);
    auto* scanBtn = new QPushButton("Scan for Leaks");
    scanBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #cba6f7; color: #1e1e2e;
            border: none; border-radius: 4px;
            padding: 6px 16px; font-weight: bold;
        }
        QPushButton:hover { background-color: #b4befe; }
    )");
    connect(scanBtn, &QPushButton::clicked, this, [this]() {
        auto* hunter = dynamic_cast<LeakHunter::LeakHunter*>(
            Core::FluxCore::instance().moduleManager().getModule("LeakHunter"));
        if(hunter) {
            auto leaks = hunter->getActiveLeaks();
            if(leaks.empty()) {
                QMessageBox::information(this, "Leak Scan",
                    "No memory leaks detected.");
            } else {
                QMessageBox::information(this, "Leak Scan",
                    QString("Found %1 potential memory leak(s).").arg(leaks.size()));
            }
        }
    });
    layout->addWidget(scanBtn);
    layout->addStretch();
}

void MainWindow::setupInfoTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);
    auto* sysGroup = new QGroupBox("System Information");
    sysGroup->setStyleSheet(R"(
        QGroupBox {
            color: #cdd6f4; border: 1px solid #313244;
            border-radius: 6px; margin-top: 12px; padding-top: 16px;
        }
        QGroupBox::title {
            subcontrol-origin: margin; padding: 0 8px;
        }
    )");
    auto* sysLayout = new QVBoxLayout(sysGroup);
    m_sysInfoLabel = new QLabel("Loading system info...");
    m_sysInfoLabel->setStyleSheet("color: #a6adc8; font-size: 12px;");
    sysLayout->addWidget(m_sysInfoLabel);
    layout->addWidget(sysGroup);
    auto* perfGroup = new QGroupBox("Performance Tips");
    perfGroup->setStyleSheet(R"(
        QGroupBox {
            color: #cdd6f4; border: 1px solid #313244;
            border-radius: 6px; margin-top: 12px; padding-top: 16px;
        }
        QGroupBox::title {
            subcontrol-origin: margin; padding: 0 8px;
        }
    )");
    auto* perfLayout = new QVBoxLayout(perfGroup);
    auto* perfTip = new QLabel(
        "- Close unused applications to free memory\n"
        "- Disable startup programs you don't need\n"
        "- Use 'Deep Clean' to clear standby memory\n"
        "- Enable 'Gaming' profile for optimal gaming performance");
    perfTip->setStyleSheet("color: #a6adc8; font-size: 12px;");
    perfLayout->addWidget(perfTip);
    layout->addWidget(perfGroup);
    auto* aboutGroup = new QGroupBox("About RAMFlux");
    aboutGroup->setStyleSheet(R"(
        QGroupBox {
            color: #cdd6f4; border: 1px solid #313244;
            border-radius: 6px; margin-top: 12px; padding-top: 16px;
        }
        QGroupBox::title {
            subcontrol-origin: margin; padding: 0 8px;
        }
    )");
    auto* aboutLayout = new QVBoxLayout(aboutGroup);
    auto* aboutLabel = new QLabel(
        "RAMFlux v2.0.0\n"
        "Memory Flux Analyzer & Optimizer\n"
        "Built with Qt 6 and MinGW");
    aboutLabel->setStyleSheet("color: #a6adc8; font-size: 12px;");
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
}

void MainWindow::setupConsoleTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(0, 0, 0, 0);
    m_consoleWidget = new ConsoleWidget(tab);
    layout->addWidget(m_consoleWidget);
    auto& logger = Core::Logger::instance();
    logger.setCallback([this](Core::LogLevel level, const std::string& msg) {
        QMetaObject::invokeMethod(this, [this, level, msg]() {
            if(m_consoleWidget) {
                m_consoleWidget->appendLog(level, msg);
            }
        }, Qt::QueuedConnection);
    });
}

void MainWindow::onMemoryUpdated() {
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    if(!telemetry) return;
    auto snap = telemetry->lastSnapshot();
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
    m_historyChart->addDataPoint(usedGB, freeGB, loadPct,
        static_cast<double>(pressure), commitGB, standbyGB);
    Q_UNUSED(totalGB);
    if(m_sysInfoLabel) {        QString infoText = QString(            "Total RAM: %1 GB\n"            "Kernel Memory: %2 GB (Paged: %3 GB | Nonpaged: %4 GB)\n"            "Virtual Memory: %5 / %6 GB\n"            "Page File: %7 / %8 GB\n"            "Compression Savings: %9 GB (%10%%)")            .arg(snap.totalRamGB(), 0, 'f', 2)            .arg(snap.kernelMemory / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.kernelPaged / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.kernelNonpaged / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.usedVirtual / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.totalVirtual / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.usedPageFile / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.totalPageFile / (1024.0 * 1024 * 1024), 0, 'f', 2)            .arg(snap.compressionSavingsGB(), 0, 'f', 2)            .arg(snap.compressionSavingsPercent(), 0, 'f', 1);        m_sysInfoLabel->setText(infoText);    }
}

void MainWindow::onSmartOptimize() {
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));
    if(telemetry && cleaner) {
        auto snap = telemetry->lastSnapshot();
        if(cleaner->deepClean()) {
            m_statusLabel->setText("Smart Optimize completed");
            onMemoryUpdated();
            QSettings s("RAMFlux", "RAMFlux");
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
        cleaner->deepClean();
        m_statusLabel->setText("Deep Clean completed");
        onMemoryUpdated();
        QSettings s("RAMFlux", "RAMFlux");
        if(s.value("notifyClean", true).toBool()) {
            auto st = cleaner->stats();
            double mb = st.lastRecoveredBytes / (1024.0 * 1024.0);
            m_trayManager->showNotification("RAMFlux", QString("Deep Clean: %1 MB recovered").arg(mb, 0, 'f', 1));
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

void MainWindow::onOpenSettings() {
    auto* dialog = new SettingsDialog(this);
    connect(dialog, &SettingsDialog::profileChanged, this, [this](int idx) {
        m_profileCombo->setCurrentIndex(idx);
        onProfileChanged(idx);
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
        QSettings s("RAMFlux", "RAMFlux");
        s.setValue("startupOptimize", delaySec > 0);
        s.setValue("startupDelaySec", delaySec > 0 ? delaySec : 30);
    });
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::onCleanStatsUpdate() {
    auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));
    if(cleaner) {
        auto stats = cleaner->stats();
        m_cleanStatsLabel->setText(QString("Recovered: %1 MB | Deep: %2 | Auto: %3")
            .arg(stats.bytesRecovered / (1024 * 1024))
            .arg(stats.deepCleanCount)
            .arg(stats.thresholdCleanCount));
    }
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

void MainWindow::loadSavedSettings() {
    auto* profileMgr = dynamic_cast<Profiles::ProfileManager*>(
        Core::FluxCore::instance().moduleManager().getModule("ProfileManager"));
    if(profileMgr) {
        auto profileType = profileMgr->activeProfile();
        m_profileCombo->setCurrentIndex(static_cast<int>(profileType));
    }
    QSettings s("RAMFlux", "RAMFlux");
    bool autoOpt = s.value("autoOptimize", true).toBool();
    // Find and sync the dashboard auto checkbox (last checkbox in action bar)
    auto* actionBar = m_tabWidget->widget(0)->layout()->itemAt(1)->widget();
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

void MainWindow::setupFileCacheTab(QWidget* tab) {    auto* layout = new QVBoxLayout(tab);
    layout->setContentsMargins(8, 8, 8, 8);
    auto* header = new QLabel("Top cached files by memory footprint (loaded modules across all processes)");
    header->setStyleSheet("color: #a6adc8; font-size: 12px; padding: 4px;");
    header->setWordWrap(true);
    layout->addWidget(header);
    m_fileCacheTable = new QTableWidget(0, 3, tab);
    m_fileCacheTable->setHorizontalHeaderLabels({"File Name", "Total Size", "Processes"});
    m_fileCacheTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_fileCacheTable->horizontalHeader()->resizeSection(1, 120);
    m_fileCacheTable->horizontalHeader()->resizeSection(2, 90);
    m_fileCacheTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_fileCacheTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_fileCacheTable->setAlternatingRowColors(true);
    m_fileCacheTable->setStyleSheet(R"(
        QTableWidget { background-color: #1e1e2e; color: #cdd6f4; border: 1px solid #313244; gridline-color: #313244; }
        QTableWidget::item { padding: 4px 8px; }
        QTableWidget::item:alternate { background-color: #181825; }
        QHeaderView::section { background-color: #11111b; color: #a6adc8; border: none; border-bottom: 1px solid #313244; padding: 6px 8px; font-weight: bold; }
    )");
    layout->addWidget(m_fileCacheTable); }

void MainWindow::onFileCacheUpdated() {
    if(m_fileCachePending.load(std::memory_order_relaxed)) return;
    m_fileCachePending.store(true, std::memory_order_relaxed);
    std::thread([this]() {
        auto files = NtApi::getTopFileCache(50);
        QMetaObject::invokeMethod(this, [this, files]() {
            m_fileCachePending.store(false, std::memory_order_relaxed);
            m_fileCacheTable->setRowCount(static_cast<int>(files.size()));
            for(int i = 0; i < static_cast<int>(files.size()); i++) {
                auto& f = files[i];
                m_fileCacheTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdWString(f.fileName)));
                m_fileCacheTable->setItem(i, 1, new QTableWidgetItem(formatBytes(f.totalSize)));
                m_fileCacheTable->setItem(i, 2, new QTableWidgetItem(QString::number(f.processCount)));
            }
        }, Qt::QueuedConnection);
    }).detach();
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
    auto b = NtApi::getPhysicalMemoryBreakdown();
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
}

} // namespace RAMFlux::UI

