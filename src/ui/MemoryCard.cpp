// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "MemoryCard.h"
#include <QVBoxLayout>
#include <QFont>
namespace RAMFlux::UI {
MemoryCard::MemoryCard(const QString& title, const QString& value,                       const QString& unit, QWidget* parent)    : QFrame(parent), m_title(title), m_value(value), m_unit(unit) {    setupUI();
}
void MemoryCard::setupUI() {    setStyleSheet(R"(        MemoryCard {            background-color: #1e1e2e;            border: 1px solid #313244;                        border-radius: 6px;
            padding: 8px;        }        MemoryCard:hover {            border: 1px solid #45475a;        }    )");    setMinimumSize(130, 60);    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto* layout = new QVBoxLayout(this);    layout->setContentsMargins(8, 6, 8, 6);    layout->setSpacing(2);    m_titleLabel = new QLabel(m_title, this);    QFont titleFont = m_titleLabel->font();    titleFont.setPointSize(8);    titleFont.setBold(false);    m_titleLabel->setFont(titleFont);    m_titleLabel->setStyleSheet("color: #a6adc8; border: none");    m_valueLabel = new QLabel(m_value, this);    QFont valueFont = m_valueLabel->font();    valueFont.setPointSize(18);    valueFont.setBold(true);    m_valueLabel->setFont(valueFont);    m_valueLabel->setStyleSheet("color: #cdd6f4; border: none");    m_unitLabel = new QLabel(m_unit, this);    QFont unitFont = m_unitLabel->font();    unitFont.setPointSize(9);    m_unitLabel->setFont(unitFont);    m_unitLabel->setStyleSheet("color: #585b70; border: none");    layout->addWidget(m_titleLabel);    layout->addWidget(m_valueLabel);    layout->addWidget(m_unitLabel);
}
void MemoryCard::setValue(const QString& value) {    m_value = value;
    if(m_valueLabel) m_valueLabel->setText(value);
}
void MemoryCard::setUnit(const QString& unit) {    m_unit = unit;
    if(m_unitLabel) m_unitLabel->setText(unit);
}
void MemoryCard::setColor(const QColor& color) {
if(m_valueLabel) {        m_valueLabel->setStyleSheet(            QString("color: %1; border: none").arg(color.name()));    }}

} // namespace RAMFlux::UI


