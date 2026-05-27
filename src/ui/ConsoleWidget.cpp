// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "ConsoleWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QApplication>
#include <QClipboard>
#include <QShortcut>
namespace RAMFlux::UI {
ConsoleWidget::ConsoleWidget(QWidget* parent)    : QWidget(parent){    auto* mainLayout = new QVBoxLayout(this);    mainLayout->setContentsMargins(8, 8, 8, 8);    mainLayout->setSpacing(6);
    auto* toolbar = new QHBoxLayout;    toolbar->setSpacing(8);
    auto* filterLabel = new QLabel("Filter:");    filterLabel->setStyleSheet("color: #a6adc8; font-size: 13px");    toolbar->addWidget(filterLabel);    m_filterCombo = new QComboBox;    m_filterCombo->addItems({
"All", "Info", "Warning", "Error"}
);    m_filterCombo->setCurrentIndex(0);    m_filterCombo->setStyleSheet(        "QComboBox { background: #1e1e2e; color: #cdd6f4; border: 1px solid #45475a"        " border-radius: 4px; padding: 4px 8px; font-size: 13px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: #1e1e2e; color: #cdd6f4"        " selection-background-color: #313244; outline: none; }"
    );    toolbar->addWidget(m_filterCombo);    toolbar->addStretch();    m_clearBtn = new QPushButton("Clear");    m_clearBtn->setStyleSheet(        "QPushButton { background: #45475a; color: #cdd6f4; border: none"        " border-radius: 4px; padding: 4px 14px; font-size: 13px; }"
        "QPushButton:hover { background: #585b70; }"
    );    toolbar->addWidget(m_clearBtn);    mainLayout->addLayout(toolbar);    m_textEdit = new QPlainTextEdit;    m_textEdit->setReadOnly(true);    m_textEdit->setMaximumBlockCount(5000);    m_textEdit->setStyleSheet(        "QPlainTextEdit { background: #11111b; color: #cdd6f4; border: 1px solid #313244"        " border-radius: 6px; padding: 8px; font-family: 'Consolas', 'Courier New', monospace"        " font-size: 12px; selection-background-color: #585b70; }"
    );    m_textEdit->setWordWrapMode(QTextOption::NoWrap);    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);    mainLayout->addWidget(m_textEdit, 1);    connect(m_clearBtn, &QPushButton::clicked, this, &ConsoleWidget::onClear);    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),        this, &ConsoleWidget::onFilterChanged);
    auto* copyShortcut = new QShortcut(QKeySequence::Copy, m_textEdit);    connect(copyShortcut, &QShortcut::activated, m_textEdit, &QPlainTextEdit::copy);
}
void ConsoleWidget::appendLog(Core::LogLevel level, const std::string& message) {    QString line = QString::fromStdString(message);    m_allLines.append(line);    QColor color = levelColor(level);
QString colored = QString("<span style='color:%1'>%2</span>")        .arg(color.name())        .arg(line.toHtmlEscaped());
    if(m_currentFilter == Core::LogLevel::Debug || level >= m_currentFilter) {        m_textEdit->appendHtml(colored);    }}
void ConsoleWidget::onClear() {    m_textEdit->clear();    m_allLines.clear();
}
void ConsoleWidget::onFilterChanged(int index) {
switch(index) {
case 0:  m_currentFilter = Core::LogLevel::Debug;
    break;
case 1:  m_currentFilter = Core::LogLevel::Info;
    break;
case 2:  m_currentFilter = Core::LogLevel::Warning;
    break;
case 3:  m_currentFilter = Core::LogLevel::Error;
    break;    }    applyFilter();
}
void ConsoleWidget::applyFilter() {    m_textEdit->clear();
    for(const auto& line : m_allLines) {        bool show = false;
    if(m_currentFilter == Core::LogLevel::Debug) {            show = true;        } else {
if(m_currentFilter == Core::LogLevel::Info)                show = line.contains("[INFO]");            else
if(m_currentFilter == Core::LogLevel::Warning)                show = line.contains("[WARN]");            else
if(m_currentFilter == Core::LogLevel::Error)                show = line.contains("[ERROR]");        }        if (show) {            QColor color;
    if(line.contains("[ERROR]")) color = levelColor(Core::LogLevel::Error);            else
if(line.contains("[WARN]")) color = levelColor(Core::LogLevel::Warning);            else
if(line.contains("[INFO]")) color = levelColor(Core::LogLevel::Info);            else color = levelColor(Core::LogLevel::Debug);
QString colored = QString("<span style='color:%1'>%2</span>")                .arg(color.name())                .arg(line.toHtmlEscaped());            m_textEdit->appendHtml(colored);        }    }}
QColor ConsoleWidget::levelColor(Core::LogLevel level) const {
    switch(level) {
    case Core::LogLevel::Debug:   return QColor("#585b70");
    case Core::LogLevel::Info:    return QColor("#89b4fa");
    case Core::LogLevel::Warning: return QColor("#f9e2af");
    case Core::LogLevel::Error:   return QColor("#f38ba8");
    }
return QColor("#cdd6f4");
}

} // namespace RAMFlux::UI


