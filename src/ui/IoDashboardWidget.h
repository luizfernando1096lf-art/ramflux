// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QTimer>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include "scheduler/IoMonitor.h"
namespace RAMFlux::Scheduler { class FluxScheduler; }
namespace RAMFlux::UI {
class IoDashboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit IoDashboardWidget(Scheduler::FluxScheduler* scheduler, QWidget* parent = nullptr);
private slots:
    void refresh();
    void onThrottleToggled(bool checked);
    void onThresholdChanged(double value);
private:
    void setupUI();
    void setupThrottlePanel(QVBoxLayout* layout);
    void setupCostPanel(QVBoxLayout* layout);
    Scheduler::FluxScheduler* m_scheduler;
    QLabel* m_sysReadLabel;
    QLabel* m_sysWriteLabel;
    QTableWidget* m_readTable;
    QTableWidget* m_writeTable;
    QLabel* m_diskQueueLabel;
    QLabel* m_throttledLabel;
    QCheckBox* m_throttleToggle;
    QDoubleSpinBox* m_thresholdSpin;
    QTableWidget* m_costTable;
    QTimer* m_timer;
    QFutureWatcher<double> m_queueWatcher;
    bool m_queuePending{false};
};
} // namespace RAMFlux::UI
