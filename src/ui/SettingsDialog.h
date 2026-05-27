// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <QLabel>
namespace RAMFlux::Profiles { struct ProfileConfig; }
namespace RAMFlux::UI {
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
signals:    void profileChanged(int profileIndex);
void automationToggled(bool enabled);
void gameModeToggled(bool enabled);
void pollingIntervalChanged(int ms);
void startupOptimizationChanged(int delaySec);
    private:
    void setupUI();
void loadSettings();
void saveSettings();
void applyProfileToUI(Profiles::ProfileConfig cfg, bool isCustom);
void connectUIChangesToCustom();
bool m_isApplyingProfile{false};
    QComboBox* m_profileCombo;    QCheckBox* m_autoOptimizeCheck;    QCheckBox* m_gameModeCheck;    QCheckBox* m_autoGameDetectCheck;    QCheckBox* m_startMinimizedCheck;    QCheckBox* m_autostartCheck;    QSlider* m_pollingSlider;    QLabel* m_pollingLabel;    QSpinBox* m_thresholdSpin;    QSlider* m_freeMemSlider;    QLabel* m_freeMemLabel;    QCheckBox* m_cleanStandbyCheck;    QCheckBox* m_cleanModifiedCheck;    QCheckBox* m_cleanWorkingSetCheck;    QCheckBox* m_cleanFileCacheCheck;    QCheckBox* m_cleanCombinedCheck;    QCheckBox* m_defragCheck;    QCheckBox* m_proBalanceCheck;    QSlider* m_proBalanceSlider;    QLabel* m_proBalanceLabel;    QCheckBox* m_startupOptimizeCheck;    QSpinBox* m_startupDelaySpin;    QLabel* m_startupDelayLabel;    QCheckBox* m_notifyCleanCheck;    QComboBox* m_scheduleCombo;
};
} // namespace RAMFlux::UI
