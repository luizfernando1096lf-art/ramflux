// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
namespace RAMFlux::UI {
class ProcessListWidget : public QWidget {
    Q_OBJECT
public:
    explicit ProcessListWidget(QWidget* parent = nullptr);
signals:    void trimRequested(int pid);
    private slots:    void refreshProcessList();
void onTrimClicked();
void onProcessDoubleClicked(int row, int column);
void onContextMenu(const QPoint& pos);
    private:
    void setupUI();    QTableWidget* m_table;    QTimer* m_refreshTimer;    QPushButton* m_trimBtn;    QPushButton* m_refreshBtn;    QLabel* m_infoLabel;
};
} // namespace RAMFlux::UI


