// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "SettingsDialog.h"
#include "core/FluxCore.h"
#include "profiles/ProfileManager.h"
#include "cleaner/FluxCleaner.h"
#include "leakhunter/LeakHunter.h"
#include "mining/FluxMiningMode.h"
#include "scheduler/FluxScheduler.h"
#include "shared/Constants.h"
#include "ui/ThemeManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QFormLayout>
#include <QScrollArea>
#include <QApplication>
#include <QStyle>
#include <QSettings>
#include <QDir>
#include <QMessageBox>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QInputDialog>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <windows.h>
#include "ntapi/FluxNTAPI.h"
#include "rules/ProcessRulesEngine.h"
#include "ui/CpuAffinityDialog.h"
namespace {
bool isAutostartEnabled() {    HKEY hKey;
    if(RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",                      0, KEY_READ, &hKey) != ERROR_SUCCESS)
return false;    wchar_t value[MAX_PATH];
DWORD size = sizeof(value);
LONG ret = RegQueryValueExW(hKey, L"RAMFlux", nullptr, nullptr, (LPBYTE)value, &size);    RegCloseKey(hKey);
    return ret == ERROR_SUCCESS;
}
void setAutostart(bool enable) {    HKEY hKey;
    if(RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",                      0, KEY_WRITE, &hKey) != ERROR_SUCCESS) return;
    if(enable) {        wchar_t exePath[MAX_PATH]{};        DWORD len = GetModuleFileNameW(nullptr, exePath, MAX_PATH);        if(len == 0 || len >= MAX_PATH) { RegCloseKey(hKey); return; }        RegSetValueExW(hKey, L"RAMFlux", 0, REG_SZ, (LPBYTE)exePath,                       (wcslen(exePath) + 1) * sizeof(wchar_t));    } else {        RegDeleteValueW(hKey, L"RAMFlux");    }    RegCloseKey(hKey);
}}
namespace RAMFlux::UI {
SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {    setupUI();    loadSettings();
}
void SettingsDialog::setupUI() {    setWindowTitle("RAMFlux Settings");    setMinimumSize(520, 400);    resize(740, 600);    setStyleSheet(R"(        QDialog {            background-color: #1e1e2e;            color: #ffffff;            font-size: 13px;        }        QGroupBox {            border: 2px solid #585b70;            border-radius: 10px;            margin-top: 22px;            color: #ffffff;            font-weight: bold;            font-size: 14px;            background: #181825;        }        QGroupBox::title {            subcontrol-origin: margin;            left: 14px;            padding: 0 10px;        }        QLabel {            color: #cdd6f4;            font-size: 13px;        }        QComboBox {            background-color: #313244;            color: #ffffff;            border: 1px solid #585b70;            border-radius: 6px;            padding: 8px 12px;            min-width: 200px;            font-size: 13px;        }        QComboBox::drop-down {            border: none;            width: 30px;        }        QComboBox QAbstractItemView {            background-color: #313244;            color: #ffffff;            selection-background-color: #585b70;            font-size: 14px;            padding: 6px;        }        QCheckBox {            color: #ffffff;            spacing: 10px;            font-size: 13px;            padding: 3px 0;        }        QCheckBox::indicator {            width: 26px;            height: 26px;            border-radius: 5px;            border: 2px solid #6c7086;        }        QCheckBox::indicator:checked {            background-color: #89b4fa;            border-color: #89b4fa;        }        QCheckBox::indicator:hover {            border-color: #b4d0fb;        }        QSpinBox {            background-color: #313244;            color: #ffffff;            border: 1px solid #585b70;            border-radius: 6px;            padding: 6px 10px;            font-size: 13px;            min-height: 24px;        }        QSlider::groove:horizontal {            height: 8px;            background: #313244;            border-radius: 4px;        }        QSlider::handle:horizontal {            background: #89b4fa;            width: 24px;            height: 24px;            margin: -8px 0;            border-radius: 12px;        }        QSlider::handle:horizontal:hover {            background: #b4d0fb;        }        QSlider::sub-page:horizontal {            background: #89b4fa;            border-radius: 4px;        }        QPushButton {            background-color: #89b4fa;            color: #1e1e2e;            border: none;            border-radius: 8px;            padding: 8px 24px;            font-weight: bold;            font-size: 14px;        }        QPushButton:hover { background-color: #b4d0fb; }        QPushButton:pressed { background-color: #7f849c; }    )");
    auto* mainLayout = new QVBoxLayout(this);    mainLayout->setContentsMargins(0, 0, 0, 0);
    auto* scrollArea = new QScrollArea(this);    scrollArea->setWidgetResizable(true);    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; } QScrollBar:vertical { background: #181825; width: 10px; } QScrollBar::handle:vertical { background: #45475a; border-radius: 5px; min-height: 30px; } QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");
    auto* scrollContent = new QWidget();    scrollContent->setStyleSheet("background: transparent;");
    auto* contentLayout = new QVBoxLayout(scrollContent);    contentLayout->setSpacing(16);    contentLayout->setContentsMargins(24, 20, 24, 20);
    auto* profileGroup = new QGroupBox("Profile", scrollContent);
    profileGroup->setStyleSheet("QGroupBox { border: 2px solid #585b70; border-radius: 10px; margin-top: 22px; color: #ffffff; font-weight: bold; font-size: 16px; background: #181825; } QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 10px; }");
    auto* profileLayout = new QFormLayout(profileGroup);    profileLayout->setSpacing(12);    profileLayout->setContentsMargins(16, 20, 16, 20);    m_profileCombo = new QComboBox(this);        m_profileCombo->addItem("Economy - Maximum power saving");
    m_profileCombo->addItem("Balanced - Default optimized");
    m_profileCombo->addItem("Performance - Maximum cleanup");
    m_profileCombo->addItem("Gaming - Anti-stutter mode");
    m_profileCombo->addItem("Mining - CPU miner optimization");
    m_profileCombo->addItem("Custom - Manual settings");        connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        if(m_isApplyingProfile) return;
        if(idx >= 0 && idx <= 5) {
            auto* pm = dynamic_cast<Profiles::ProfileManager*>(
                Core::FluxCore::instance().moduleManager().getModule("ProfileManager"));
            if(pm) applyProfileToUI(pm->configForProfile(static_cast<Constants::ProfileType>(idx)), idx == 5);
        }
    });    profileLayout->addRow("Active Profile:", m_profileCombo);    contentLayout->addWidget(profileGroup);
    auto* behaviorGroup = new QGroupBox("Behavior", this);
    auto* behaviorLayout = new QVBoxLayout(behaviorGroup);    behaviorLayout->setSpacing(6);    behaviorLayout->setContentsMargins(16, 20, 16, 20);    m_autoOptimizeCheck = new QCheckBox("Enable Auto-optimization", this);    m_gameModeCheck = new QCheckBox("Enable Game Mode Detection", this);    m_autoGameDetectCheck = new QCheckBox("Auto-detect full-screen games", this);    m_startMinimizedCheck = new QCheckBox("Start Minimized to Tray", this);    m_autostartCheck = new QCheckBox("Start with Windows", this);        m_notifyCleanCheck = new QCheckBox("Show notification after cleaning", this);
    m_aiHeuristicsCheck = new QCheckBox("Enable AI Heuristics (workload detection + pressure prediction)", this);
    m_miningModeCheck = new QCheckBox("Enable Mining Mode (CPU miner optimization)", this);
    m_aiHeuristicsCheck->setChecked(true);
    m_miningModeCheck->setChecked(true);
    m_notifyCleanCheck->setChecked(true);
    behaviorLayout->addWidget(m_autoOptimizeCheck);
    behaviorLayout->addWidget(m_gameModeCheck);
    behaviorLayout->addWidget(m_autoGameDetectCheck);
    behaviorLayout->addWidget(m_miningModeCheck);
    behaviorLayout->addWidget(m_startMinimizedCheck);
    behaviorLayout->addWidget(m_autostartCheck);
    behaviorLayout->addWidget(m_notifyCleanCheck);
    behaviorLayout->addWidget(m_aiHeuristicsCheck);
    auto* diagBtn = new QPushButton("Run System Diagnostics", this);
    diagBtn->setStyleSheet(R"(        QPushButton {            background-color: #cba6f7; color: #1e1e2e;            border: none; border-radius: 6px;            padding: 8px 16px; font-weight: bold; font-size: 13px;        }        QPushButton:hover { background-color: #b4befe; }    )");
    connect(diagBtn, &QPushButton::clicked, this, [this, diagBtn]() {
        diagBtn->setEnabled(false);
        diagBtn->setText("Running Diagnostics...");
        auto* watcher = new QFutureWatcher<QStringList>(this);
        connect(watcher, &QFutureWatcher<QStringList>::finished, this, [this, watcher, diagBtn]() {
            QStringList results = watcher->result();
            watcher->deleteLater();
            diagBtn->setEnabled(true);
            diagBtn->setText("Run System Diagnostics");
            QMessageBox::information(this, "Diagnostics", results.join("\n"));
        });
        watcher->setFuture(QtConcurrent::run([]() -> QStringList {
            using namespace NtApi;
            QStringList results;
            results << "RAMFlux System Diagnostics";
            results << "========================";
            results << "";
            results << QString("Standby Memory: %1 MB").arg(getStandbyMemorySize() / (1024*1024));
            results << QString("Modified Memory: %1 MB").arg(getTotalModifiedMemory() / (1024*1024));
            results << QString("Compressed Memory: %1 MB").arg(getCompressedMemorySize() / (1024*1024));
            results << "";
            results << "Capability Tests:";
            auto pm = getPhysicalMemoryBreakdown();
            results << QString("  Query Memory Lists: %1").arg(pm.totalPages > 0 ? "PASS" : "FAIL");
            results << QString("  Clear Standby List: %1").arg(NtApi::clearStandbyList() ? "PASS" : "FAIL (may need admin)");
            results << QString("  Clear Modified Pages: %1").arg(NtApi::clearModifiedPageList() ? "PASS" : "FAIL (may need admin)");
            results << QString("  Clear Working Set: %1").arg(NtApi::clearWorkingSet() ? "PASS" : "FAIL (may need admin)");
            results << QString("  NTDLL Query: %1").arg(getStandbyMemorySize() > 0 ? "PASS" : "FAIL");
            results << "";
            results << "Process Priorities:";
            auto myPrio = getProcessPriority(GetCurrentProcessId());
            results << QString("  RAMFlux Priority Class: %1").arg(myPrio);
            return results;
        }));
    });
    behaviorLayout->addWidget(diagBtn);
    auto* resetBtn = new QPushButton("Reset Cleaner Statistics", this);
    resetBtn->setStyleSheet(R"(        QPushButton {            background-color: #f38ba8; color: #1e1e2e;            border: none; border-radius: 6px;            padding: 8px 16px; font-weight: bold; font-size: 13px;        }        QPushButton:hover { background-color: #eba0ac; }    )");
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        if(QMessageBox::question(this, "Reset Statistics",
            "Reset all cleaner statistics? This cannot be undone.",
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
        auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));
        if(cleaner) { cleaner->resetStats(); QMessageBox::information(this, "Stats Reset", "Cleaner statistics have been reset."); }
    });
    behaviorLayout->addWidget(resetBtn);
    contentLayout->addWidget(behaviorGroup);
    auto* themeGroup = new QGroupBox("Appearance", this);
    auto* themeLayout = new QFormLayout(themeGroup);
    themeLayout->setSpacing(12);
    themeLayout->setContentsMargins(16, 20, 16, 20);
    m_themeCombo = new QComboBox(this);
    auto& tmgr = ThemeManager::instance();
    for (int i = 0; i < tmgr.themeCount(); ++i) {
        m_themeCombo->addItem(tmgr.themeName(i));
    }
    themeLayout->addRow("Theme:", m_themeCombo);
    contentLayout->addWidget(themeGroup);

    auto* rulesGroup = new QGroupBox("Process Rules", this);
    auto* rulesLayout = new QVBoxLayout(rulesGroup);
    rulesLayout->setContentsMargins(16, 20, 16, 16);
    rulesLayout->setSpacing(6);
    m_rulesEnabledCheck = new QCheckBox("Enable persistent process rules", this);
    m_rulesEnabledCheck->setChecked(true);
    rulesLayout->addWidget(m_rulesEnabledCheck);
    m_rulesTable = new QTableWidget(0, 4, this);
    m_rulesTable->setHorizontalHeaderLabels({"Process", "Type", "Value", "Status"});
    m_rulesTable->horizontalHeader()->setStretchLastSection(true);
    m_rulesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_rulesTable->setMinimumHeight(120);
    rulesLayout->addWidget(m_rulesTable);
    auto* rulesBtnRow = new QHBoxLayout();
    auto* addRuleBtn = new QPushButton("Add Rule", this);
    auto* editRuleBtn = new QPushButton("Edit Rule", this);
    auto* delRuleBtn = new QPushButton("Delete Rule", this);
    rulesBtnRow->addWidget(addRuleBtn);
    rulesBtnRow->addWidget(editRuleBtn);
    rulesBtnRow->addWidget(delRuleBtn);
    rulesBtnRow->addStretch();
    rulesLayout->addLayout(rulesBtnRow);
    connect(addRuleBtn, &QPushButton::clicked, this, &SettingsDialog::addPersistentRuleDialog);
    connect(editRuleBtn, &QPushButton::clicked, this, [this]() {
        int row = m_rulesTable->currentRow();
        if(row >= 0) editRuleDialog(row, false);
    });
    connect(delRuleBtn, &QPushButton::clicked, this, [this]() {
        int row = m_rulesTable->currentRow();
        if(row < 0) return;
        auto* engine = dynamic_cast<Rules::ProcessRulesEngine*>(
            Core::FluxCore::instance().moduleManager().getModule("ProcessRulesEngine"));
        if(!engine) return;
        auto rules = engine->persistentRules();
        if(row < static_cast<int>(rules.size())) {
            engine->removeRule(rules[row].id);
            refreshRulesTable();
        }
    });
    contentLayout->addWidget(rulesGroup);

    auto* watchGroup = new QGroupBox("Watchdog Rules", this);
    auto* watchLayout = new QVBoxLayout(watchGroup);
    watchLayout->setContentsMargins(16, 20, 16, 16);
    watchLayout->setSpacing(6);
    m_watchdogEnabledCheck = new QCheckBox("Enable process watchdog", this);
    m_watchdogEnabledCheck->setChecked(true);
    watchLayout->addWidget(m_watchdogEnabledCheck);
    m_watchdogTable = new QTableWidget(0, 6, this);
    m_watchdogTable->setHorizontalHeaderLabels({"Process", "Trigger", "Threshold", "Duration", "Action", "Status"});
    m_watchdogTable->horizontalHeader()->setStretchLastSection(true);
    m_watchdogTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_watchdogTable->setMinimumHeight(120);
    watchLayout->addWidget(m_watchdogTable);
    auto* watchBtnRow = new QHBoxLayout();
    auto* addWatchBtn = new QPushButton("Add Watchdog", this);
    auto* editWatchBtn = new QPushButton("Edit Watchdog", this);
    auto* delWatchBtn = new QPushButton("Delete Watchdog", this);
    watchBtnRow->addWidget(addWatchBtn);
    watchBtnRow->addWidget(editWatchBtn);
    watchBtnRow->addWidget(delWatchBtn);
    watchBtnRow->addStretch();
    watchLayout->addLayout(watchBtnRow);
    connect(addWatchBtn, &QPushButton::clicked, this, &SettingsDialog::addWatchdogRuleDialog);
    connect(editWatchBtn, &QPushButton::clicked, this, [this]() {
        int row = m_watchdogTable->currentRow();
        if(row >= 0) editRuleDialog(row, true);
    });
    connect(delWatchBtn, &QPushButton::clicked, this, [this]() {
        int row = m_watchdogTable->currentRow();
        if(row < 0) return;
        auto* engine = dynamic_cast<Rules::ProcessRulesEngine*>(
            Core::FluxCore::instance().moduleManager().getModule("ProcessRulesEngine"));
        if(!engine) return;
        auto rules = engine->watchdogRules();
        if(row < static_cast<int>(rules.size())) {
            engine->removeRule(rules[row].id);
            refreshWatchdogTable();
        }
    });
    contentLayout->addWidget(watchGroup);

    auto* pollingGroup = new QGroupBox("Polling", this);
    auto* pollingLayout = new QVBoxLayout(pollingGroup);    pollingLayout->setSpacing(10);    pollingLayout->setContentsMargins(16, 20, 16, 20);    m_pollingLabel = new QLabel("Update Interval: 1000 ms", this);    m_pollingSlider = new QSlider(Qt::Horizontal, this);    m_pollingSlider->setRange(200, 5000);    m_pollingSlider->setValue(1000);    m_pollingSlider->setTickPosition(QSlider::TicksBelow);    m_pollingSlider->setTickInterval(500);    m_pollingSlider->setMinimumHeight(40);    connect(m_pollingSlider, &QSlider::valueChanged, this, [this](int val) {        m_pollingLabel->setText(QString("Update Interval: %1 ms").arg(val));    }
);    pollingLayout->addWidget(m_pollingLabel);    pollingLayout->addWidget(m_pollingSlider);    contentLayout->addWidget(pollingGroup);
    auto* leakGroup = new QGroupBox("Leak Detection", this);
    auto* leakLayout = new QFormLayout(leakGroup);    leakLayout->setSpacing(12);    leakLayout->setContentsMargins(16, 20, 16, 20);    m_thresholdSpin = new QSpinBox(this);    m_thresholdSpin->setRange(50, 2000);    m_thresholdSpin->setValue(100);    m_thresholdSpin->setSuffix(" MB");    m_thresholdSpin->setMinimumHeight(34);    leakLayout->addRow("Alert Threshold:", m_thresholdSpin);    contentLayout->addWidget(leakGroup);
    auto* thresholdGroup = new QGroupBox("Auto-Clean Threshold", this);
    auto* thresholdLayout = new QVBoxLayout(thresholdGroup);    thresholdLayout->setContentsMargins(16, 20, 16, 16);    thresholdLayout->setSpacing(10);    m_freeMemLabel = new QLabel("Auto-clean when free RAM < 2.0 GB", this);    m_freeMemSlider = new QSlider(Qt::Horizontal, this);    m_freeMemSlider->setRange(1, 16);    m_freeMemSlider->setValue(2);    m_freeMemSlider->setTickPosition(QSlider::TicksBelow);    m_freeMemSlider->setTickInterval(1);    m_freeMemSlider->setMinimumHeight(40);    connect(m_freeMemSlider, &QSlider::valueChanged, this, [this](int val) {        m_freeMemLabel->setText(QString("Auto-clean when free RAM < %1 GB").arg(val));    });    auto* freeMemNote = new QLabel("When free RAM drops below this value, adaptive cleaning triggers automatically.", this);    freeMemNote->setStyleSheet("color: #585b70; font-size: 12px;");    freeMemNote->setWordWrap(true);    thresholdLayout->addWidget(m_freeMemLabel);    thresholdLayout->addWidget(m_freeMemSlider);    thresholdLayout->addWidget(freeMemNote);    contentLayout->addWidget(thresholdGroup);
    auto* areaGroup = new QGroupBox("Clean Areas", this);
    auto* areaLayout = new QVBoxLayout(areaGroup);    areaLayout->setContentsMargins(16, 20, 16, 16);    areaLayout->setSpacing(4);    m_cleanStandbyCheck = new QCheckBox("Standby List (cached memory)", this);    m_cleanModifiedCheck = new QCheckBox("Modified Page List", this);    m_cleanWorkingSetCheck = new QCheckBox("Process Working Sets", this);    m_cleanFileCacheCheck = new QCheckBox("System File Cache", this);    m_cleanCombinedCheck = new QCheckBox("Combined Page List", this);    m_defragCheck = new QCheckBox("Defragment RAM (combine pages + compact working sets)", this);    m_cleanStandbyCheck->setChecked(true);    m_cleanModifiedCheck->setChecked(true);    m_cleanWorkingSetCheck->setChecked(true);    m_cleanFileCacheCheck->setChecked(false);    m_cleanCombinedCheck->setChecked(false);    m_defragCheck->setChecked(true);    auto* areaNote = new QLabel("Uncheck areas you want to preserve during cleaning.", this);    areaNote->setStyleSheet("color: #585b70; font-size: 12px;");    areaLayout->addWidget(m_cleanStandbyCheck);    areaLayout->addWidget(m_cleanModifiedCheck);    areaLayout->addWidget(m_cleanWorkingSetCheck);    areaLayout->addWidget(m_cleanFileCacheCheck);    areaLayout->addWidget(m_cleanCombinedCheck);    areaLayout->addWidget(m_defragCheck);    areaLayout->addWidget(areaNote);    contentLayout->addWidget(areaGroup);
    auto* proGroup = new QGroupBox("ProBalance Process Manager", this);
    auto* proLayout = new QVBoxLayout(proGroup);    proLayout->setContentsMargins(16, 20, 16, 16);    proLayout->setSpacing(10);    m_proBalanceCheck = new QCheckBox("Auto-lower priority of memory-heavy processes", this);    m_proBalanceLabel = new QLabel("Process memory threshold: 512 MB", this);    m_proBalanceSlider = new QSlider(Qt::Horizontal, this);    m_proBalanceSlider->setRange(128, 4096);    m_proBalanceSlider->setValue(512);    m_proBalanceSlider->setTickPosition(QSlider::TicksBelow);    m_proBalanceSlider->setTickInterval(256);    m_proBalanceSlider->setMinimumHeight(40);    m_proBalanceSlider->setEnabled(false);    connect(m_proBalanceSlider, &QSlider::valueChanged, this, [this](int val) {        m_proBalanceLabel->setText(QString("Process memory threshold: %1 MB").arg(val));    });    connect(m_proBalanceCheck, &QCheckBox::toggled, m_proBalanceSlider, &QSlider::setEnabled);    auto* proNote = new QLabel("Processes exceeding this threshold get BELOW_NORMAL priority until they calm down.", this);    proNote->setStyleSheet("color: #585b70; font-size: 12px;");    proNote->setWordWrap(true);    proLayout->addWidget(m_proBalanceCheck);    proLayout->addWidget(m_proBalanceLabel);    proLayout->addWidget(m_proBalanceSlider);    proLayout->addWidget(proNote);    contentLayout->addWidget(proGroup);
    auto* startupGroup = new QGroupBox("Startup Optimization", this);
    auto* startupLayout = new QVBoxLayout(startupGroup);    startupLayout->setContentsMargins(16, 20, 16, 16);    startupLayout->setSpacing(10);    m_startupOptimizeCheck = new QCheckBox("Perform initial clean on system startup", this);    m_startupOptimizeCheck->setChecked(true);    m_startupDelayLabel = new QLabel("Delay before clean: 30 seconds", this);    m_startupDelaySpin = new QSpinBox(this);    m_startupDelaySpin->setRange(10, 300);    m_startupDelaySpin->setValue(30);    m_startupDelaySpin->setSuffix(" seconds");    m_startupDelaySpin->setMinimumHeight(34);    connect(m_startupDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int val) {        m_startupDelayLabel->setText(QString("Delay before clean: %1 seconds").arg(val));    });    auto* startupNote = new QLabel("RAMFlux will automatically clean memory shortly after startup to reclaim memory before other apps finish loading.", this);    startupNote->setStyleSheet("color: #585b70; font-size: 12px;");    startupNote->setWordWrap(true);    startupLayout->addWidget(m_startupOptimizeCheck);    auto* delayRow = new QHBoxLayout();    delayRow->addWidget(m_startupDelayLabel);    delayRow->addWidget(m_startupDelaySpin);    delayRow->addStretch();    startupLayout->addLayout(delayRow);    startupLayout->addWidget(startupNote);    contentLayout->addWidget(startupGroup);
    auto* schedGroup = new QGroupBox("Scheduled Cleaning", this);
    auto* schedLayout = new QVBoxLayout(schedGroup);    schedLayout->setContentsMargins(16, 20, 16, 16);    schedLayout->setSpacing(10);    m_scheduleCombo = new QComboBox(this);    m_scheduleCombo->addItem("Disabled");    m_scheduleCombo->addItem("Every 30 minutes");    m_scheduleCombo->addItem("Every 1 hour");    m_scheduleCombo->addItem("Every 2 hours");    m_scheduleCombo->addItem("Every 4 hours");    m_scheduleCombo->addItem("Every 6 hours");    m_scheduleCombo->addItem("Every 12 hours");    m_scheduleCombo->addItem("Every 24 hours");    m_scheduleCombo->setMinimumHeight(34);    auto* schedNote = new QLabel("Schedule automatic deep cleaning at regular intervals regardless of memory pressure.", this);    schedNote->setStyleSheet("color: #585b70; font-size: 12px;");    schedNote->setWordWrap(true);    schedLayout->addWidget(new QLabel("Cleaning Schedule:", this));    schedLayout->addWidget(m_scheduleCombo);    schedLayout->addWidget(schedNote);    contentLayout->addWidget(schedGroup);
    contentLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);
    auto* btnLayout = new QHBoxLayout();    btnLayout->setContentsMargins(24, 8, 24, 12);
    auto* saveBtn = new QPushButton("Save && Close", this);    connect(saveBtn, &QPushButton::clicked, this, [this]() {        saveSettings();        accept();    }
);    btnLayout->addStretch();    btnLayout->addWidget(saveBtn);    mainLayout->addLayout(btnLayout);
    connectUIChangesToCustom();
}
void SettingsDialog::loadSettings() {    QSettings s(QSettings::IniFormat, QSettings::UserScope, "RAMFlux", "RAMFlux");
    auto* pm = dynamic_cast<Profiles::ProfileManager*>(        Core::FluxCore::instance().moduleManager().getModule("ProfileManager"));
int profileIdx = s.value("profile", 1).toInt();
    if(pm && profileIdx >= 0 && profileIdx <= 5) {        pm->setProfile(static_cast<Constants::ProfileType>(profileIdx));        m_isApplyingProfile = true;        m_profileCombo->setCurrentIndex(profileIdx);        m_isApplyingProfile = false;    } else if(pm) {        m_isApplyingProfile = true;        m_profileCombo->setCurrentIndex(static_cast<int>(pm->activeProfile()));        m_isApplyingProfile = false;    }
    m_autoOptimizeCheck->setChecked(s.value("autoOptimize", true).toBool());
    m_gameModeCheck->setChecked(s.value("gameMode", false).toBool());
    m_autoGameDetectCheck->setChecked(s.value("autoGameDetect", true).toBool());
    m_miningModeCheck->setChecked(s.value("miningMode", true).toBool());
    m_startMinimizedCheck->setChecked(s.value("startMinimized", false).toBool());
    m_autostartCheck->setChecked(isAutostartEnabled());
    m_notifyCleanCheck->setChecked(s.value("notifyClean", true).toBool());
    m_aiHeuristicsCheck->setChecked(s.value("aiHeuristics", true).toBool());
    m_rulesEnabledCheck->setChecked(s.value("rulesEnabled", true).toBool());
    m_watchdogEnabledCheck->setChecked(s.value("watchdogEnabled", true).toBool());
    m_themeCombo->setCurrentIndex(s.value("themeIndex", 0).toInt());
    m_scheduleCombo->setCurrentIndex(s.value("scheduleIndex", 0).toInt());        int pollVal = s.value("pollingInterval", 1000).toInt();        m_pollingSlider->setValue(pollVal);        m_pollingLabel->setText(QString("Update Interval: %1 ms").arg(pollVal));        m_thresholdSpin->setValue(s.value("leakThreshold", 100).toInt());
        auto* hunter = dynamic_cast<LeakHunter::LeakHunter*>(
            Core::FluxCore::instance().moduleManager().getModule("LeakHunter"));
        if(hunter) hunter->setThresholdMB(static_cast<uint64_t>(m_thresholdSpin->value()));
        int freeMemVal = s.value("freeMemThresholdGB", 2).toInt();        m_freeMemSlider->setValue(freeMemVal);        m_freeMemLabel->setText(QString("Auto-clean when free RAM < %1 GB").arg(freeMemVal));        m_cleanStandbyCheck->setChecked(s.value("areaStandby", true).toBool());        m_cleanModifiedCheck->setChecked(s.value("areaModified", true).toBool());        m_cleanWorkingSetCheck->setChecked(s.value("areaWorkingSet", true).toBool());        m_cleanFileCacheCheck->setChecked(s.value("areaFileCache", false).toBool());        m_cleanCombinedCheck->setChecked(s.value("areaCombined", false).toBool());        m_defragCheck->setChecked(s.value("areaDefrag", true).toBool());        bool pbEnabled = s.value("proBalance", false).toBool();        m_proBalanceCheck->setChecked(pbEnabled);        m_proBalanceSlider->setEnabled(pbEnabled);        int pbMem = s.value("proBalanceMemMB", 512).toInt();        m_proBalanceSlider->setValue(pbMem);        m_proBalanceLabel->setText(QString("Process memory threshold: %1 MB").arg(pbMem));        m_startupOptimizeCheck->setChecked(s.value("startupOptimize", true).toBool());        int startupDelay = s.value("startupDelaySec", 30).toInt();        m_startupDelaySpin->setValue(startupDelay);        m_startupDelayLabel->setText(QString("Delay before clean: %1 seconds").arg(startupDelay));
}
void SettingsDialog::saveSettings() {    QSettings s(QSettings::IniFormat, QSettings::UserScope, "RAMFlux", "RAMFlux");    s.setValue("profile", m_profileCombo->currentIndex());    s.setValue("autoOptimize", m_autoOptimizeCheck->isChecked());        s.setValue("gameMode", m_gameModeCheck->isChecked());
    s.setValue("miningMode", m_miningModeCheck->isChecked());
    s.setValue("aiHeuristics", m_aiHeuristicsCheck->isChecked());
    s.setValue("autoGameDetect", m_autoGameDetectCheck->isChecked());    s.setValue("startMinimized", m_startMinimizedCheck->isChecked());    s.setValue("notifyClean", m_notifyCleanCheck->isChecked());    s.setValue("pollingInterval", m_pollingSlider->value());    s.setValue("leakThreshold", m_thresholdSpin->value());    s.setValue("freeMemThresholdGB", m_freeMemSlider->value());    s.setValue("areaStandby", m_cleanStandbyCheck->isChecked());    s.setValue("areaModified", m_cleanModifiedCheck->isChecked());    s.setValue("areaWorkingSet", m_cleanWorkingSetCheck->isChecked());    s.setValue("areaFileCache", m_cleanFileCacheCheck->isChecked());    s.setValue("areaCombined", m_cleanCombinedCheck->isChecked());    s.setValue("areaDefrag", m_defragCheck->isChecked());    s.setValue("proBalance", m_proBalanceCheck->isChecked());    s.setValue("proBalanceMemMB", m_proBalanceSlider->value());    s.setValue("startupOptimize", m_startupOptimizeCheck->isChecked());    s.setValue("startupDelaySec", m_startupDelaySpin->value());    s.setValue("scheduleIndex", m_scheduleCombo->currentIndex());
    s.setValue("rulesEnabled", m_rulesEnabledCheck->isChecked());
    s.setValue("watchdogEnabled", m_watchdogEnabledCheck->isChecked());
    s.setValue("themeIndex", m_themeCombo->currentIndex());
    s.sync();
    auto* engine = dynamic_cast<Rules::ProcessRulesEngine*>(
        Core::FluxCore::instance().moduleManager().getModule("ProcessRulesEngine"));
    if(engine) {
        engine->setEnabled(m_rulesEnabledCheck->isChecked());
        engine->setWatchdogEnabled(m_watchdogEnabledCheck->isChecked());
    }
    emit themeChanged(m_themeCombo->currentIndex());
    auto* hunter = dynamic_cast<LeakHunter::LeakHunter*>(
        Core::FluxCore::instance().moduleManager().getModule("LeakHunter"));
    if(hunter) hunter->setThresholdMB(static_cast<uint64_t>(m_thresholdSpin->value()));
    auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(        Core::FluxCore::instance().moduleManager().getModule("FluxCleaner"));    if(cleaner) {        cleaner->setFreeMemThresholdMB(static_cast<uint64_t>(m_freeMemSlider->value()) * 1024);        uint64_t areas = 0;        if(m_cleanStandbyCheck->isChecked()) areas |= Cleaner::FluxCleaner::Area::Standby;        if(m_cleanModifiedCheck->isChecked()) areas |= Cleaner::FluxCleaner::Area::Modified;        if(m_cleanWorkingSetCheck->isChecked()) areas |= Cleaner::FluxCleaner::Area::WorkingSet;        if(m_cleanFileCacheCheck->isChecked()) areas |= Cleaner::FluxCleaner::Area::FileCache;        if(m_cleanCombinedCheck->isChecked()) areas |= Cleaner::FluxCleaner::Area::Combined;        if(m_defragCheck->isChecked()) areas |= Cleaner::FluxCleaner::Area::Defrag;        cleaner->setEnabledAreas(areas);    }
    auto* pm = dynamic_cast<Profiles::ProfileManager*>(        Core::FluxCore::instance().moduleManager().getModule("ProfileManager"));
    if(pm) {        int idx = m_profileCombo->currentIndex();        if(idx < 0 || idx > 5) idx = 0;        auto profile = static_cast<Constants::ProfileType>(idx);        pm->setProfile(profile);        if(idx == 5) {            Profiles::ProfileConfig customCfg;            customCfg.autoCleanEnabled = m_autoOptimizeCheck->isChecked();            customCfg.gameMode = m_gameModeCheck->isChecked();            customCfg.pollingIntervalMs = m_pollingSlider->value();            customCfg.freeMemThresholdMB = static_cast<uint64_t>(m_freeMemSlider->value()) * 1024;            customCfg.aggressiveTrim = m_defragCheck->isChecked();            customCfg.cooldownMs = 30000;            customCfg.leakDetection = true;            customCfg.pressureThreshold = 50;            customCfg.standbyThresholdMB = 1024;            pm->setCustomConfig(customCfg);        }        emit profileChanged(idx);    }
    auto* miningMode = dynamic_cast<Mining::FluxMiningMode*>(
        Core::FluxCore::instance().moduleManager().getModule("FluxMiningMode"));
    if(miningMode) {
        miningMode->setMiningModeEnabled(m_miningModeCheck->isChecked());
    }    auto* scheduler = dynamic_cast<Scheduler::FluxScheduler*>(        Core::FluxCore::instance().moduleManager().getModule("FluxScheduler"));    if(scheduler) {        scheduler->setProBalanceEnabled(m_proBalanceCheck->isChecked());        scheduler->setProBalanceMemoryMB(static_cast<uint64_t>(m_proBalanceSlider->value()));        int idx = m_scheduleCombo->currentIndex();        static const int scheduleMs[] = {0, 1800000, 3600000, 7200000, 14400000, 21600000, 43200000, 86400000};        if(idx > 0 && idx < 8) {            scheduler->setScheduleIntervalMs(scheduleMs[idx]);            scheduler->setScheduleEnabled(true);        } else {            scheduler->setScheduleEnabled(false);        }    }    emit automationToggled(m_autoOptimizeCheck->isChecked());    emit gameModeToggled(m_gameModeCheck->isChecked());    emit pollingIntervalChanged(m_pollingSlider->value());    emit startupOptimizationChanged(m_startupOptimizeCheck->isChecked() ? m_startupDelaySpin->value() : 0);    setAutostart(m_autostartCheck->isChecked());
}
void SettingsDialog::applyProfileToUI(Profiles::ProfileConfig cfg, bool isCustom) {    m_isApplyingProfile = true;    m_autoOptimizeCheck->setChecked(cfg.autoCleanEnabled);    m_gameModeCheck->setChecked(cfg.gameMode);    m_pollingSlider->setValue(cfg.pollingIntervalMs);    m_pollingLabel->setText(QString("Update Interval: %1 ms").arg(cfg.pollingIntervalMs));    int freeMemGB = static_cast<int>(cfg.freeMemThresholdMB / 1024);    if(freeMemGB < 1) freeMemGB = 2;    if(freeMemGB > 16) freeMemGB = 16;    m_freeMemSlider->setValue(freeMemGB);    m_freeMemLabel->setText(QString("Auto-clean when free RAM < %1 GB").arg(freeMemGB));    m_defragCheck->setChecked(cfg.aggressiveTrim);    m_isApplyingProfile = false;
}
void SettingsDialog::refreshRulesTable() {
    auto* engine = dynamic_cast<Rules::ProcessRulesEngine*>(
        Core::FluxCore::instance().moduleManager().getModule("ProcessRulesEngine"));
    if(!engine) return;
    auto rules = engine->persistentRules();
    m_rulesTable->setRowCount(static_cast<int>(rules.size()));
    for(int i = 0; i < static_cast<int>(rules.size()); ++i) {
        const auto& r = rules[i];
        m_rulesTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(r.processPattern)));
        const char* typeNames[] = {"CPU Priority", "I/O Priority", "Page Priority", "CPU Affinity", "Memory Priority"};
        int ti = static_cast<int>(r.type);
        m_rulesTable->setItem(i, 1, new QTableWidgetItem(ti < 5 ? typeNames[ti] : "Unknown"));
        QString val;
        if(r.type == Rules::RuleType::CpuPriority) {
            uint32_t prio = static_cast<uint32_t>(r.value);
            if(prio == Rules::PRIORITY_IDLE) val = "Idle";
            else if(prio == Rules::PRIORITY_BELOW_NORMAL) val = "Below Normal";
            else if(prio == Rules::PRIORITY_NORMAL) val = "Normal";
            else if(prio == Rules::PRIORITY_ABOVE_NORMAL) val = "Above Normal";
            else if(prio == Rules::PRIORITY_HIGH) val = "High";
            else if(prio == Rules::PRIORITY_REALTIME) val = "Realtime";
            else val = QString::number(prio);
        } else if(r.type == Rules::RuleType::IoPriority) {
            uint32_t prio = static_cast<uint32_t>(r.value);
            if(prio == Rules::IO_PRIORITY_VERY_LOW) val = "Very Low";
            else if(prio == Rules::IO_PRIORITY_NORMAL) val = "Normal";
            else if(prio == Rules::IO_PRIORITY_HIGH) val = "High";
            else val = QString::number(prio);
        } else if(r.type == Rules::RuleType::CpuAffinity) {
            val = QString("0x%1").arg(static_cast<uint64_t>(r.value), 0, 16);
        } else {
            val = QString::number(static_cast<uint64_t>(r.value));
        }
        m_rulesTable->setItem(i, 2, new QTableWidgetItem(val));
        m_rulesTable->setItem(i, 3, new QTableWidgetItem(r.enabled ? "Active" : "Disabled"));
    }
    m_rulesTable->resizeColumnsToContents();
}
void SettingsDialog::refreshWatchdogTable() {
    auto* engine = dynamic_cast<Rules::ProcessRulesEngine*>(
        Core::FluxCore::instance().moduleManager().getModule("ProcessRulesEngine"));
    if(!engine) return;
    auto rules = engine->watchdogRules();
    m_watchdogTable->setRowCount(static_cast<int>(rules.size()));
    for(int i = 0; i < static_cast<int>(rules.size()); ++i) {
        const auto& r = rules[i];
        m_watchdogTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(r.processPattern)));
        m_watchdogTable->setItem(i, 1, new QTableWidgetItem(r.type == Rules::RuleType::WatchdogMemory ? "Memory (MB)" : "Trigger"));
        m_watchdogTable->setItem(i, 2, new QTableWidgetItem(QString::number(r.triggerThreshold)));
        m_watchdogTable->setItem(i, 3, new QTableWidgetItem(QString::number(r.triggerDurationMs) + " ms"));
        const char* actionNames[] = {"None", "Set CPU Priority", "Set I/O Priority", "Set Page Priority", "Set CPU Affinity", "Set Memory Priority", "Terminate", "Restart", "Log", "Execute"};
        int ai = static_cast<int>(r.watchdogAction);
        m_watchdogTable->setItem(i, 4, new QTableWidgetItem(ai < 10 ? actionNames[ai] : "Unknown"));
        m_watchdogTable->setItem(i, 5, new QTableWidgetItem(r.enabled ? "Active" : "Disabled"));
    }
    m_watchdogTable->resizeColumnsToContents();
}
void SettingsDialog::addPersistentRuleDialog() {
    auto* engine = dynamic_cast<Rules::ProcessRulesEngine*>(
        Core::FluxCore::instance().moduleManager().getModule("ProcessRulesEngine"));
    if(!engine) return;
    bool ok = false;
    QString pattern = QInputDialog::getText(this, "New Rule", "Process name (supports * wildcard):", QLineEdit::Normal, "", &ok);
    if(!ok || pattern.isEmpty()) return;
    QStringList types;
    types << "CPU Priority" << "I/O Priority" << "Page Priority" << "CPU Affinity";
    QString typeStr = QInputDialog::getItem(this, "New Rule", "Rule type:", types, 0, false, &ok);
    if(!ok) return;
    Rules::ProcessRule rule;
    rule.processPattern = pattern.toStdString();
    rule.enabled = true;
    int typeIdx = types.indexOf(typeStr);
    if(typeIdx == 0) {
        rule.type = Rules::RuleType::CpuPriority;
        QStringList priorities;
        priorities << "Idle" << "Below Normal" << "Normal" << "Above Normal" << "High";
        QString prioStr = QInputDialog::getItem(this, "New Rule", "CPU priority:", priorities, 2, false, &ok);
        if(!ok) return;
        uint32_t vals[] = {Rules::PRIORITY_IDLE, Rules::PRIORITY_BELOW_NORMAL, Rules::PRIORITY_NORMAL, Rules::PRIORITY_ABOVE_NORMAL, Rules::PRIORITY_HIGH};
        rule.value = vals[priorities.indexOf(prioStr)];
    } else if(typeIdx == 1) {
        rule.type = Rules::RuleType::IoPriority;
        QStringList priorities;
        priorities << "Very Low" << "Normal" << "High";
        QString prioStr = QInputDialog::getItem(this, "New Rule", "I/O priority:", priorities, 1, false, &ok);
        if(!ok) return;
        uint32_t vals[] = {Rules::IO_PRIORITY_VERY_LOW, Rules::IO_PRIORITY_NORMAL, Rules::IO_PRIORITY_HIGH};
        rule.value = vals[priorities.indexOf(prioStr)];
    } else if(typeIdx == 2) {
        rule.type = Rules::RuleType::PagePriority;
        QStringList priorities;
        priorities << "Very Low (1)" << "Low (2)" << "Below Normal (4)" << "Normal (5)";
        QString prioStr = QInputDialog::getItem(this, "New Rule", "Page priority:", priorities, 3, false, &ok);
        if(!ok) return;
        int pvals[] = {1, 2, 4, 5};
        rule.value = pvals[priorities.indexOf(prioStr)];
    } else if(typeIdx == 3) {
        rule.type = Rules::RuleType::CpuAffinity;
        uint32_t cpuCount = NtApi::getSystemCpuCount();
        DWORD_PTR currentMask = static_cast<DWORD_PTR>(-1);
        auto* dlg = new CpuAffinityDialog(cpuCount, currentMask, this);
        if(dlg->exec() == QDialog::Accepted) {
            rule.value = static_cast<uint64_t>(dlg->selectedMask());
        } else {
            delete dlg; return;
        }
        delete dlg;
    }
    engine->addRule(rule);
    refreshRulesTable();
}
void SettingsDialog::addWatchdogRuleDialog() {
    auto* engine = dynamic_cast<Rules::ProcessRulesEngine*>(
        Core::FluxCore::instance().moduleManager().getModule("ProcessRulesEngine"));
    if(!engine) return;
    bool ok = false;
    QString pattern = QInputDialog::getText(this, "New Watchdog", "Process name (supports * wildcard):", QLineEdit::Normal, "", &ok);
    if(!ok || pattern.isEmpty()) return;
    int threshold = QInputDialog::getInt(this, "New Watchdog", "Memory threshold (MB):", 500, 1, 100000, 100, &ok);
    if(!ok) return;
    int duration = QInputDialog::getInt(this, "New Watchdog", "Duration before trigger (ms):", 10000, 1000, 600000, 1000, &ok);
    if(!ok) return;
    QStringList actions;
    actions << "Set CPU Priority" << "Set I/O Priority" << "Set CPU Affinity" << "Terminate" << "Restart" << "Log";
    QString actionStr = QInputDialog::getItem(this, "New Watchdog", "Action:", actions, 5, false, &ok);
    if(!ok) return;
    Rules::ProcessRule rule;
    rule.processPattern = pattern.toStdString();
    rule.type = Rules::RuleType::WatchdogMemory;
    rule.enabled = true;
    rule.triggerThreshold = static_cast<uint32_t>(threshold);
    rule.triggerDurationMs = static_cast<uint32_t>(duration);
    int ai = actions.indexOf(actionStr);
    if(ai == 0) {
        rule.watchdogAction = Rules::RuleAction::SetCpuPriority;
        QStringList priorities;
        priorities << "Idle" << "Below Normal" << "Normal" << "Above Normal" << "High";
        QString prioStr = QInputDialog::getItem(this, "New Watchdog", "CPU priority:", priorities, 2, false, &ok);
        if(!ok) return;
        uint32_t vals[] = {Rules::PRIORITY_IDLE, Rules::PRIORITY_BELOW_NORMAL, Rules::PRIORITY_NORMAL, Rules::PRIORITY_ABOVE_NORMAL, Rules::PRIORITY_HIGH};
        rule.value = vals[priorities.indexOf(prioStr)];
    } else if(ai == 1) {
        rule.watchdogAction = Rules::RuleAction::SetIoPriority;
        QStringList priorities;
        priorities << "Very Low" << "Normal" << "High";
        QString prioStr = QInputDialog::getItem(this, "New Watchdog", "I/O priority:", priorities, 1, false, &ok);
        if(!ok) return;
        uint32_t vals[] = {Rules::IO_PRIORITY_VERY_LOW, Rules::IO_PRIORITY_NORMAL, Rules::IO_PRIORITY_HIGH};
        rule.value = vals[priorities.indexOf(prioStr)];
    } else if(ai == 2) {
        rule.watchdogAction = Rules::RuleAction::SetCpuAffinity;
        uint32_t cpuCount = NtApi::getSystemCpuCount();
        DWORD_PTR currentMask = static_cast<DWORD_PTR>(-1);
        auto* dlg = new CpuAffinityDialog(cpuCount, currentMask, this);
        if(dlg->exec() == QDialog::Accepted) {
            rule.value = static_cast<uint64_t>(dlg->selectedMask());
        } else {
            delete dlg; return;
        }
        delete dlg;
    } else if(ai == 3) {
        rule.watchdogAction = Rules::RuleAction::Terminate;
    } else if(ai == 4) {
        rule.watchdogAction = Rules::RuleAction::Restart;
    } else if(ai == 5) {
        rule.watchdogAction = Rules::RuleAction::Log;
    }
    engine->addRule(rule);
    refreshWatchdogTable();
}
void SettingsDialog::editRuleDialog(int row, bool isWatchdog) {
    auto* engine = dynamic_cast<Rules::ProcessRulesEngine*>(
        Core::FluxCore::instance().moduleManager().getModule("ProcessRulesEngine"));
    if(!engine) return;
    auto rules = isWatchdog ? engine->watchdogRules() : engine->persistentRules();
    if(row < 0 || row >= static_cast<int>(rules.size())) return;
    const auto& rule = rules[row];
    QStringList options;
    options << "Enable" << "Disable" << "Delete";
    bool ok = false;
    QString choice = QInputDialog::getItem(this, "Edit Rule", "Action for rule \"" + QString::fromStdString(rule.processPattern) + "\":", options, 0, false, &ok);
    if(!ok) return;
    int ci = options.indexOf(choice);
    if(ci == 0) { engine->setRuleEnabled(rule.id, true); }
    else if(ci == 1) { engine->setRuleEnabled(rule.id, false); }
    else if(ci == 2) { engine->removeRule(rule.id); }
    if(isWatchdog) refreshWatchdogTable();
    else refreshRulesTable();
}
void SettingsDialog::connectUIChangesToCustom() {    auto switchToCustom = [this]() {        if(m_isApplyingProfile) return;        m_isApplyingProfile = true;        m_profileCombo->setCurrentIndex(5);        m_isApplyingProfile = false;    };    connect(m_autoOptimizeCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_gameModeCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_autoGameDetectCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_startMinimizedCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_autostartCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_notifyCleanCheck, &QCheckBox::toggled, this, switchToCustom);
    connect(m_aiHeuristicsCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_miningModeCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_pollingSlider, &QSlider::valueChanged, this, switchToCustom);    connect(m_thresholdSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, switchToCustom);    connect(m_freeMemSlider, &QSlider::valueChanged, this, switchToCustom);    connect(m_cleanStandbyCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_cleanModifiedCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_cleanWorkingSetCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_cleanFileCacheCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_cleanCombinedCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_defragCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_proBalanceCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_proBalanceSlider, &QSlider::valueChanged, this, switchToCustom);    connect(m_startupOptimizeCheck, &QCheckBox::toggled, this, switchToCustom);    connect(m_startupDelaySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, switchToCustom);    connect(m_scheduleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, switchToCustom);
    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, switchToCustom);
    connect(m_rulesEnabledCheck, &QCheckBox::toggled, this, switchToCustom);
    connect(m_watchdogEnabledCheck, &QCheckBox::toggled, this, switchToCustom);
}
} // namespace RAMFlux::UI


