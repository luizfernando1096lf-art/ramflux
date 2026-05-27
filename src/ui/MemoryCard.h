// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <QFrame>
#include <QLabel>
#include <QString>
namespace RAMFlux::UI {
class MemoryCard : public QFrame {
    Q_OBJECT
public:
    explicit MemoryCard(const QString& title, const QString& value = "---",                        const QString& unit = "", QWidget* parent = nullptr);
void setValue(const QString& value);
void setUnit(const QString& unit);
void setColor(const QColor& color);
    private:
    void setupUI();    QLabel* m_titleLabel;    QLabel* m_valueLabel;    QLabel* m_unitLabel;
QString m_title;
QString m_value;
QString m_unit;
};
} // namespace RAMFlux::UI


