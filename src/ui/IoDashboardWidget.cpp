// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "IoDashboardWidget.h"
#include "scheduler/FluxScheduler.h"
#include "ntapi/FluxNTAPI.h"
#include "core/FluxCore.h"
#include "ai/HeuristicEngine.h"
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGroupBox>
#include <QFormLayout>
namespace RAMFlux::UI {
static QString formatIoRate(uint64_t bps) {
    if(bps >= 1073741824ULL) return QString("%1 GB/s").arg(bps / 1073741824.0, 0, 'f', 1);
    if(bps >= 1048576ULL) return QString("%1 MB/s").arg(bps / 1048576.0, 0, 'f', 1);
    if(bps >= 1024ULL) return QString("%1 KB/s").arg(bps / 1024.0, 0, 'f', 1);
    return QString("%1 B/s").arg(bps);
}
IoDashboardWidget::IoDashboardWidget(Scheduler::FluxScheduler* scheduler, QWidget* parent)
    : QWidget(parent), m_scheduler(scheduler) {
    setupUI();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &IoDashboardWidget::refresh);
    m_timer->start(3000);
    connect(&m_queueWatcher, &QFutureWatcher<double>::finished, this, [this]() {
        m_queuePending = false;
        double queue = m_queueWatcher.result();
        m_diskQueueLabel->setText(QString("Disk Queue: %1").arg(queue, 0, 'f', 2));
        if(queue >= m_scheduler->ioBandwidthThrottler().diskQueueThreshold())
            m_diskQueueLabel->setStyleSheet("font-weight: bold; color: #f38ba8; padding: 2px 8px;");
        else
            m_diskQueueLabel->setStyleSheet("font-weight: bold; color: #a6e3a1; padding: 2px 8px;");
        m_throttledLabel->setText(QString("Throttled: %1").arg(m_scheduler->ioBandwidthThrottler().throttledProcessCount()));
    });
}
void IoDashboardWidget::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);
    // System I/O
    auto* sysGroup = new QGroupBox(QStringLiteral("System I/O"));
    auto* sysLayout = new QHBoxLayout(sysGroup);
    m_sysReadLabel = new QLabel(QStringLiteral("Read: --"));
    m_sysWriteLabel = new QLabel(QStringLiteral("Write: --"));
    m_sysReadLabel->setStyleSheet("font-size:14px; font-weight:bold; color:#4CAF50;");
    m_sysWriteLabel->setStyleSheet("font-size:14px; font-weight:bold; color:#FF5722;");
    sysLayout->addWidget(m_sysReadLabel);
    sysLayout->addWidget(m_sysWriteLabel);
    sysLayout->addStretch();
    mainLayout->addWidget(sysGroup);
    // Top Readers
    auto* readGroup = new QGroupBox(QStringLiteral("Top Readers"));
    auto* readLayout = new QVBoxLayout(readGroup);
    m_readTable = new QTableWidget(0, 4);
    m_readTable->setHorizontalHeaderLabels({QStringLiteral("PID"), QStringLiteral("Process"), QStringLiteral("Read Rate"), QStringLiteral("Total Read")});
    m_readTable->horizontalHeader()->setStretchLastSection(true);
    m_readTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_readTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_readTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_readTable->setAlternatingRowColors(true);
    m_readTable->verticalHeader()->setVisible(false);
    readLayout->addWidget(m_readTable);
    mainLayout->addWidget(readGroup);
    // Top Writers
    auto* writeGroup = new QGroupBox(QStringLiteral("Top Writers"));
    auto* writeLayout = new QVBoxLayout(writeGroup);
    m_writeTable = new QTableWidget(0, 4);
    m_writeTable->setHorizontalHeaderLabels({QStringLiteral("PID"), QStringLiteral("Process"), QStringLiteral("Write Rate"), QStringLiteral("Total Write")});
    m_writeTable->horizontalHeader()->setStretchLastSection(true);
    m_writeTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_writeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_writeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_writeTable->setAlternatingRowColors(true);
    m_writeTable->verticalHeader()->setVisible(false);
    writeLayout->addWidget(m_writeTable);
    mainLayout->addWidget(writeGroup);
    setupThrottlePanel(mainLayout);
    setupCostPanel(mainLayout);
}
void IoDashboardWidget::setupThrottlePanel(QVBoxLayout* layout) {
    auto* group = new QGroupBox("I/O Bandwidth Throttle");
    auto* form = new QFormLayout(group);
    auto* topLayout = new QHBoxLayout;
    m_throttleToggle = new QCheckBox("Enabled");
    connect(m_throttleToggle, &QCheckBox::toggled, this, &IoDashboardWidget::onThrottleToggled);
    topLayout->addWidget(m_throttleToggle);
    m_diskQueueLabel = new QLabel("Disk Queue: --");
    m_diskQueueLabel->setStyleSheet("font-weight: bold; padding: 2px 8px;");
    topLayout->addWidget(m_diskQueueLabel);
    m_throttledLabel = new QLabel("Throttled: 0");
    m_throttledLabel->setStyleSheet("padding: 2px 8px;");
    topLayout->addWidget(m_throttledLabel);
    topLayout->addStretch();
    form->addRow(topLayout);
    m_thresholdSpin = new QDoubleSpinBox;
    m_thresholdSpin->setRange(0.5, 10.0);
    m_thresholdSpin->setSingleStep(0.5);
    m_thresholdSpin->setValue(2.0);
    m_thresholdSpin->setDecimals(1);
    connect(m_thresholdSpin, &QDoubleSpinBox::valueChanged, this, &IoDashboardWidget::onThresholdChanged);
    form->addRow("Disk Queue Threshold:", m_thresholdSpin);
    layout->addWidget(group);
}
void IoDashboardWidget::setupCostPanel(QVBoxLayout* layout) {
    auto* group = new QGroupBox("Process I/O Cost (AI)");
    auto* groupLayout = new QVBoxLayout(group);
    m_costTable = new QTableWidget(0, 3);
    m_costTable->setHorizontalHeaderLabels({QStringLiteral("PID"), QStringLiteral("Process"), QStringLiteral("Cost Score")});
    m_costTable->horizontalHeader()->setStretchLastSection(true);
    m_costTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_costTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_costTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_costTable->setAlternatingRowColors(true);
    m_costTable->verticalHeader()->setVisible(false);
    groupLayout->addWidget(m_costTable);
    layout->addWidget(group);
}
void IoDashboardWidget::refresh() {
    if(!m_scheduler) return;
    // System I/O
    if(m_scheduler->isIoMonitorEnabled()) {
        auto sysStats = m_scheduler->ioMonitor().systemStats();
        m_sysReadLabel->setText(QString("Read: %1").arg(formatIoRate(sysStats.readRateBps)));
        m_sysWriteLabel->setText(QString("Write: %1").arg(formatIoRate(sysStats.writeRateBps)));
        auto readers = m_scheduler->ioMonitor().topReaders(10);
        m_readTable->setRowCount(static_cast<int>(readers.size()));
        for(int i = 0; i < static_cast<int>(readers.size()); ++i) {
            const auto& r = readers[i];
            m_readTable->setItem(i, 0, new QTableWidgetItem(QString::number(r.pid)));
            m_readTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdWString(r.name)));
            m_readTable->setItem(i, 2, new QTableWidgetItem(formatIoRate(r.readRateBps)));
            m_readTable->setItem(i, 3, new QTableWidgetItem(formatIoRate(r.totalRead)));
        }
        auto writers = m_scheduler->ioMonitor().topWriters(10);
        m_writeTable->setRowCount(static_cast<int>(writers.size()));
        for(int i = 0; i < static_cast<int>(writers.size()); ++i) {
            const auto& w = writers[i];
            m_writeTable->setItem(i, 0, new QTableWidgetItem(QString::number(w.pid)));
            m_writeTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdWString(w.name)));
            m_writeTable->setItem(i, 2, new QTableWidgetItem(formatIoRate(w.writeRateBps)));
            m_writeTable->setItem(i, 3, new QTableWidgetItem(formatIoRate(w.totalWrite)));
        }
    }
    // Disk queue & throttle status (async — M1 fix, avoid blocking UI 80-300ms)
    if(!m_queuePending) {
        m_queuePending = true;
        m_queueWatcher.setFuture(QtConcurrent::run([]() {
            return RAMFlux::NtApi::getDiskQueueLength();
        }));
    }
    m_throttleToggle->blockSignals(true);
    m_throttleToggle->setChecked(m_scheduler->isIoBandwidthEnabled());
    m_throttleToggle->blockSignals(false);
    // I/O cost table
    auto* engine = dynamic_cast<AI::HeuristicEngine*>(
        Core::FluxCore::instance().moduleManager().getModule("HeuristicEngine"));
    if(engine) {
        auto ioCosts = engine->currentReport().ioCost;
        m_costTable->setRowCount(static_cast<int>(ioCosts.topCostProcesses.size()));
        for(int i = 0; i < static_cast<int>(ioCosts.topCostProcesses.size()); ++i) {
            const auto& c = ioCosts.topCostProcesses[i];
            m_costTable->setItem(i, 0, new QTableWidgetItem(QString::number(c.pid)));
            m_costTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdWString(c.name)));
            auto* scoreItem = new QTableWidgetItem(QString::number(c.costScore, 'f', 1));
            scoreItem->setTextAlignment(Qt::AlignCenter);
            if(c.costScore >= 50.0)
                scoreItem->setForeground(QColor("#f38ba8"));
            else if(c.costScore >= 25.0)
                scoreItem->setForeground(QColor("#f9e2af"));
            m_costTable->setItem(i, 2, scoreItem);
        }
    }
}
void IoDashboardWidget::onThrottleToggled(bool checked) {
    if(m_scheduler) m_scheduler->setIoBandwidthEnabled(checked);
}
void IoDashboardWidget::onThresholdChanged(double value) {
    if(m_scheduler) m_scheduler->ioBandwidthThrottler().setDiskQueueThreshold(value);
}
} // namespace RAMFlux::UI
