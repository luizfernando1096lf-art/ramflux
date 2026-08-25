// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "ProcessListWidget.h"
#include <windows.h>
#include "core/FluxCore.h"
#include "analyzer/FluxProcessAnalyzer.h"
#include "telemetry/FluxTelemetry.h"
#include "telemetry/MemorySnapshot.h"
#include "ntapi/FluxNTAPI.h"
#include "shared/ProcessUtils.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QApplication>
#include <QSet>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDateTime>
#include <QMenu>
#include <QClipboard>
#include <QColor>
#include <QtConcurrent>
#include <QFutureWatcher>
namespace RAMFlux::UI {
ProcessListWidget::ProcessListWidget(QWidget* parent) : QWidget(parent) {    setupUI();
}
void ProcessListWidget::setupUI() {    auto* layout = new QVBoxLayout(this);    layout->setContentsMargins(0, 0, 0, 0);    layout->setSpacing(8);
    auto* headerLayout = new QHBoxLayout();    m_infoLabel = new QLabel("Top Memory Processes", this);    m_infoLabel->setStyleSheet("color: #cdd6f4; font-size: 13px; font-weight: bold");    headerLayout->addWidget(m_infoLabel);    headerLayout->addStretch();    m_refreshBtn = new QPushButton("Refresh", this);    m_refreshBtn->setStyleSheet(R"(        QPushButton {            background-color: #313244; color: #cdd6f4;            border: 1px solid #45475a; border-radius: 4px;            padding: 4px 12px;        }        QPushButton:hover { background-color: #45475a; }    )");    connect(m_refreshBtn, &QPushButton::clicked, this, &ProcessListWidget::refreshProcessList);    headerLayout->addWidget(m_refreshBtn);    m_trimBtn = new QPushButton("Trim Selected", this);    m_trimBtn->setStyleSheet(R"(        QPushButton {            background-color: #f38ba8; color: #1e1e2e;            border: none; border-radius: 4px;            padding: 4px 12px; font-weight: bold;        }        QPushButton:hover { background-color: #eba0ac; }    )");    connect(m_trimBtn, &QPushButton::clicked, this, &ProcessListWidget::onTrimClicked);    headerLayout->addWidget(m_trimBtn);    layout->addLayout(headerLayout);    m_table = new QTableWidget(this);    m_table->setColumnCount(11);    m_table->setHorizontalHeaderLabels({
"PID", "Process", "Working Set", "Private", "Peak WS", "Page File", "Page Table", "CPU %", "Threads", "Standby", "Cold Pages"}
);    m_table->horizontalHeader()->setStretchLastSection(true);    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);    m_table->setSelectionMode(QAbstractItemView::SingleSelection);    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);    m_table->setAlternatingRowColors(true);    m_table->verticalHeader()->setVisible(false);    m_table->setStyleSheet(R"(        QTableWidget {            background-color: #181825;            color: #cdd6f4;            border: 1px solid #313244;            border-radius: 6px;            gridline-color: #313244;        }        QTableWidget::item { padding: 4px 8px; }        QTableWidget::item:alternate { background-color: #181825; }
        QTableWidget::item:selected { background-color: #45475a; }        QHeaderView::section {            background-color: #1e1e2e;            color: #a6adc8;            border: none;            border-bottom: 1px solid #313244;            padding: 6px 8px;            font-weight: bold;        }    )");    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_table, &QTableWidget::customContextMenuRequested, this, &ProcessListWidget::onContextMenu);
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &ProcessListWidget::onProcessDoubleClicked);    layout->addWidget(m_table);
}
static QString formatBytes(uint64_t bytes) {
if(bytes >= 1024LL * 1024 * 1024)
return QString::number(bytes / (1024.0 * 1024 * 1024), 'f', 2) + " GB";
    if(bytes > 0)
return QString::number(bytes / (1024.0 * 1024), 'f', 0) + " MB";
    return "-";
}
void ProcessListWidget::refreshProcessList() {    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(        Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    if(!telemetry) return;
    auto snap = telemetry->lastSnapshot();
    const auto& processes = snap.topProcesses;    m_table->setRowCount(static_cast<int>(processes.size()));
    for(int i = 0; i < static_cast<int>(processes.size()); ++i) {        const auto& p = processes[i];
    auto* pidItem = new QTableWidgetItem(QString::number(p.pid));        pidItem->setTextAlignment(Qt::AlignCenter);        m_table->setItem(i, 0, pidItem);        m_table->setItem(i, 1, new QTableWidgetItem(QString::fromStdWString(p.name)));
    auto* wsItem = new QTableWidgetItem(formatBytes(p.workingSet));        wsItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        m_table->setItem(i, 2, wsItem);
    auto* privItem = new QTableWidgetItem(formatBytes(p.privateUsage));        privItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        m_table->setItem(i, 3, privItem);
    auto* peakItem = new QTableWidgetItem(formatBytes(p.peakWorkingSet));        peakItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        m_table->setItem(i, 4, peakItem);
    auto* pfItem = new QTableWidgetItem(formatBytes(p.pageFileUsage));        pfItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        m_table->setItem(i, 5, pfItem);
    auto* ptItem = new QTableWidgetItem(formatBytes(p.pageTableUsage));        ptItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        m_table->setItem(i, 6, ptItem);
    auto* cpuItem = new QTableWidgetItem(QString::number(p.cpuPercent, 'f', 1));        cpuItem->setTextAlignment(Qt::AlignCenter);        m_table->setItem(i, 7, cpuItem);
    auto* threadItem = new QTableWidgetItem(QString::number(p.threadCount));        threadItem->setTextAlignment(Qt::AlignCenter);        m_table->setItem(i, 8, threadItem);
    auto* sbItem = new QTableWidgetItem(formatBytes(p.standbyMemory));        sbItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);        m_table->setItem(i, 9, sbItem);
    QString coldText = (p.coldPageRatio > 0.0)
        ? QString("%1 (%2%)").arg(formatBytes(p.coldPageBytes)).arg(p.coldPageRatio * 100.0, 0, 'f', 0)
        : "-";
    auto* coldItem = new QTableWidgetItem(coldText);
    coldItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if(p.coldPageRatio > 0.3) coldItem->setForeground(QColor("#f9e2af"));
    m_table->setItem(i, 10, coldItem);
    }    m_infoLabel->setText(QString("Process Breakdown (%1)").arg(processes.size()));
}
void ProcessListWidget::onTrimClicked() {    int row = m_table->currentRow();
    if(row < 0) return;
    auto* pidItem = m_table->item(row, 0);
    if(!pidItem) return;
int pid = pidItem->text().toInt();
    auto* analyzer = dynamic_cast<Analyzer::FluxProcessAnalyzer*>(        Core::FluxCore::instance().moduleManager().getModule("FluxProcessAnalyzer"));
    if(analyzer) {        analyzer->trimProcess(pid);    }}
void ProcessListWidget::onProcessDoubleClicked(int row, int) {    auto* pidItem = m_table->item(row, 0);
    if(!pidItem) return;
    int pid = pidItem->text().toInt();
    auto* nameItem = m_table->item(row, 1);
    QString pname = nameItem ? nameItem->text() : "Unknown";
    // Fetch process info off UI thread (M1 fix — 7 NtApi calls = 30-150ms block)
    struct ProcDetails {
        uint64_t ws{0}, standby{0}, ptUsage{0}, createTime{0};
        uint32_t prio{0}, hcount{0};
        NtApi::ProcessIoStats io{};
    };
    auto* watcher = new QFutureWatcher<ProcDetails>(this);
    connect(watcher, &QFutureWatcher<ProcDetails>::finished, this, [this, watcher, pid, pname]() {
        ProcDetails d = watcher->result();
        watcher->deleteLater();
        uint64_t ws = d.ws, standby = d.standby, ptUsage = d.ptUsage, createTime = d.createTime;
        uint32_t prio = d.prio, hcount = d.hcount;
        auto io = d.io;
    // Build dialog
    QDialog dlg(this);    dlg.setWindowTitle(QString("Process: %1 (PID: %2)").arg(pname).arg(pid));
    dlg.setMinimumWidth(450);    dlg.setStyleSheet("QDialog { background-color: #1e1e2e; color: #cdd6f4; } QLabel { color: #cdd6f4; }");
    auto* fl = new QFormLayout(&dlg);    fl->setSpacing(6);
    fl->setContentsMargins(16, 16, 16, 16);
    auto addRow = [&](const QString& label, const QString& value) {        auto* vl = new QLabel(value);        vl->setStyleSheet("color: #a6adc8;");
    fl->addRow(label + ":", vl);    };
    addRow("Process", pname);    addRow("PID", QString::number(pid));
    addRow("Working Set", formatBytes(ws));
    addRow("Standby Memory", standby > 0 ? formatBytes(standby) : "N/A");
    addRow("Page Table", formatBytes(ptUsage));
    QString prioName;    switch(prio) {        case HIGH_PRIORITY_CLASS: prioName = "High"; break;
    case ABOVE_NORMAL_PRIORITY_CLASS: prioName = "Above Normal"; break;
    case NORMAL_PRIORITY_CLASS: prioName = "Normal"; break;
    case BELOW_NORMAL_PRIORITY_CLASS: prioName = "Below Normal"; break;
    case IDLE_PRIORITY_CLASS: prioName = "Idle"; break;
    default: prioName = QString::number(prio); }
    addRow("Priority", prioName);
    addRow("Handles", QString::number(hcount));
    addRow("IO Read", QString("%1 ops / %2").arg(io.readOps).arg(formatBytes(io.readBytes)));
    addRow("IO Write", QString("%1 ops / %2").arg(io.writeOps).arg(formatBytes(io.writeBytes)));
    if(createTime) {        // Convert FILETIME (100-ns intervals since 1601-01-01) to Qt datetime
        qint64 ft = static_cast<qint64>(createTime - 116444736000000000LL) / 10;
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(ft / 1000);
    addRow("Started", dt.toString("yyyy-MM-dd hh:mm:ss"));    }
    // Get telemetry data
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(Core::FluxCore::instance().moduleManager().getModule("FluxTelemetry"));
    if(telemetry) {        auto snap = telemetry->lastSnapshot();
    for(const auto& p : snap.topProcesses) {            if(p.pid == static_cast<uint32_t>(pid)) {                addRow("Private Usage", formatBytes(p.privateUsage));
                addRow("Page File", formatBytes(p.pageFileUsage));                addRow("Page Faults", QString::number(p.pageFaults));
                if(p.coldPageRatio > 0.0) {
                    addRow("Cold Pages", QString("%1 (%2%)")
                        .arg(formatBytes(p.coldPageBytes))
                        .arg(p.coldPageRatio * 100.0, 0, 'f', 0));
                }
    addRow("Threads", QString::number(p.threadCount));                addRow("CPU %", QString::number(p.cpuPercent, 'f', 1));
    break;            }        }    }
    auto* bb = new QDialogButtonBox(QDialogButtonBox::Close);    bb->setStyleSheet("QPushButton { background-color: #313244; color: #cdd6f4; border: 1px solid #45475a; border-radius: 4px; padding: 6px 16px; } QPushButton:hover { background-color: #45475a; }");
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    fl->addRow(bb);    dlg.exec(); });
    watcher->setFuture(QtConcurrent::run([pid]() -> ProcDetails {
        ProcDetails d;
        d.ws = NtApi::getWorkingSetSize(pid);
        d.standby = NtApi::getProcessStandbyMemory(pid);
        d.ptUsage = NtApi::getProcessPageTableUsage(pid);
        d.prio = NtApi::getProcessPriority(pid);
        d.hcount = NtApi::getProcessHandleCount(pid);
        d.io = NtApi::getProcessIoStats(pid);
        d.createTime = NtApi::getProcessCreationTime(pid);
        return d;
    })); }
void ProcessListWidget::onContextMenu(const QPoint& pos) {    int row = m_table->rowAt(pos.y());
    if(row < 0) return;
    auto* pidItem = m_table->item(row, 0);
    if(!pidItem) return;
    int pid = pidItem->text().toInt();
    auto* nameItem = m_table->item(row, 1);
    QString pname = nameItem ? nameItem->text() : "Unknown";
    QMenu menu(this);    menu.setStyleSheet("QMenu { background-color: #1e1e2e; color: #cdd6f4; border: 1px solid #313244; } QMenu::item:selected { background-color: #313244; }");
    auto* copyAct = menu.addAction("Copy Name && PID");
    connect(copyAct, &QAction::triggered, this, [pid, pname]() {        QApplication::clipboard()->setText(QString("%1 (PID: %2)").arg(pname).arg(pid));    });
    menu.addSeparator();
    auto* trimAct = menu.addAction("Trim Working Set");
    connect(trimAct, &QAction::triggered, this, [pid]() { QtConcurrent::run([pid](){ NtApi::trimProcessWorkingSet(pid); }); });
    auto* highAct = menu.addAction("Set Priority > High");
    connect(highAct, &QAction::triggered, this, [pid]() { QtConcurrent::run([pid](){ NtApi::setProcessPriority(pid, HIGH_PRIORITY_CLASS); }); });
    auto* aboveAct = menu.addAction("Set Priority > Above Normal");
    connect(aboveAct, &QAction::triggered, this, [pid]() { QtConcurrent::run([pid](){ NtApi::setProcessPriority(pid, ABOVE_NORMAL_PRIORITY_CLASS); }); });
    auto* belowAct = menu.addAction("Set Priority > Below Normal");
    connect(belowAct, &QAction::triggered, this, [pid]() { QtConcurrent::run([pid](){ NtApi::setProcessPriority(pid, BELOW_NORMAL_PRIORITY_CLASS); }); });
    auto* idleAct = menu.addAction("Set Priority > Idle");
    connect(idleAct, &QAction::triggered, this, [pid]() { QtConcurrent::run([pid](){ NtApi::setProcessPriority(pid, IDLE_PRIORITY_CLASS); }); });
    menu.addSeparator();
    auto* killAct = menu.addAction("Terminate Process");
    killAct->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical));
    connect(killAct, &QAction::triggered, this, [pid, pname, this]() {
        QString msg = QString("Are you sure you want to terminate %1 (PID %2)?").arg(pname).arg(pid);
        if(QMessageBox::warning(this, "Terminate Process", msg, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, static_cast<DWORD>(pid));
        if(hProcess) {
            if(!pname.isEmpty() && !ProcessUtils::verifyProcessName(hProcess, pname.toStdWString())) {
                CloseHandle(hProcess);
                QMessageBox::warning(this, "Terminate Process",
                    QString("The process identity changed (PID reuse detected). Operation aborted to avoid terminating the wrong process."));
            } else {
                TerminateProcess(hProcess, 1);
                CloseHandle(hProcess);
            }
        }    });
    menu.exec(m_table->viewport()->mapToGlobal(pos)); }

} // namespace RAMFlux::UI


