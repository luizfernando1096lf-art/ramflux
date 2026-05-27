// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <QWidget>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QStringList>
#include "core/Logger.h"
namespace RAMFlux::UI {
class ConsoleWidget : public QWidget {
    Q_OBJECT
public:
    explicit ConsoleWidget(QWidget* parent = nullptr);
void appendLog(Core::LogLevel level, const std::string& message);
    private slots:    void onClear();
void onFilterChanged(int index);
    private:
    void applyFilter();    QColor levelColor(Core::LogLevel level) const;    QPlainTextEdit* m_textEdit;    QComboBox* m_filterCombo;    QPushButton* m_clearBtn;    QStringList m_allLines;    QStringList m_filteredLevels;    Core::LogLevel m_currentFilter{
Core::LogLevel::Debug};
};
} // namespace RAMFlux::UI


