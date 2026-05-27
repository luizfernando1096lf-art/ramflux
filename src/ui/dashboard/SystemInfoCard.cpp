#include "SystemInfoCard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QColor>

SystemInfoCard::SystemInfoCard(const QString& title, const QString& iconName, QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
    , m_iconLabel(nullptr)
    , m_valueLabel(nullptr)
    , m_percentageLabel(nullptr)
{
    setupWidget(title, iconName);
}

void SystemInfoCard::setupWidget(const QString& title, const QString& iconName)
{
    createLayout();
    
    m_titleLabel->setText(title);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    
    m_iconLabel->setText(iconName);
    m_iconLabel->setStyleSheet("font-size: 18px;");
    
    m_valueLabel->setText("0");
    m_valueLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    
    m_percentageLabel->setText("");
    m_percentageLabel->setStyleSheet("color: #888; font-size: 10px;");
    
    layout()->setContentsMargins(5, 5, 5, 5);
}

void SystemInfoCard::createLayout()
{
    auto *hLayout = new QHBoxLayout(this);
    
    m_iconLabel = new QLabel();
    m_iconLabel->setFixedWidth(30);
    hLayout->addWidget(m_iconLabel);
    
    auto *vLayout = new QVBoxLayout();
    hLayout->addLayout(vLayout);
    
    m_titleLabel = new QLabel();
    vLayout->addWidget(m_titleLabel);
    
    m_valueLabel = new QLabel();
    vLayout->addWidget(m_valueLabel);
    
    m_percentageLabel = new QLabel();
    vLayout->addWidget(m_percentageLabel);
}

void SystemInfoCard::setValue(const QString& value)
{
    m_valueLabel->setText(value);
}

void SystemInfoCard::setPercentage(int percentage)
{
    m_percentageLabel->setText(QString::number(percentage) + "%");
}

void SystemInfoCard::refreshInfo()
{
    // Atualizar informações do widget
}