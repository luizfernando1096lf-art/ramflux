// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "SystemTrayManager.h"
#include <QApplication>
#include <QStyle>
namespace RAMFlux::UI {
SystemTrayManager::SystemTrayManager(QObject* parent) : QObject(parent) {}
SystemTrayManager::~SystemTrayManager() {    delete m_trayMenu;
}
bool SystemTrayManager::initialize() {
if(m_initialized)
return true;
    if(!QSystemTrayIcon::isSystemTrayAvailable()) {
return false;    }    m_trayIcon = new QSystemTrayIcon(this);    m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));    m_trayIcon->setToolTip("RAMFlux - Memory Optimizer");    setupMenu();    m_trayIcon->setContextMenu(m_trayMenu);    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &SystemTrayManager::onTrayActivated);    m_trayIcon->show();    m_initialized = true;
    return true;
}
void SystemTrayManager::setupMenu() {    m_trayMenu = new QMenu();
    auto* showAction = m_trayMenu->addAction("Open Dashboard");    connect(showAction, &QAction::triggered, this, &SystemTrayManager::showDashboardRequested);    m_trayMenu->addSeparator();
    auto* smartAction = m_trayMenu->addAction("Smart Optimize");    connect(smartAction, &QAction::triggered, this, &SystemTrayManager::smartOptimizeRequested);
    auto* deepAction = m_trayMenu->addAction("Deep Clean");    connect(deepAction, &QAction::triggered, this, &SystemTrayManager::deepCleanRequested);    m_trayMenu->addSeparator();
    auto* autoAction = m_trayMenu->addAction("Auto-Optimize");    autoAction->setCheckable(true);
    connect(autoAction, &QAction::toggled, this, &SystemTrayManager::autoOptimizeToggled);
    auto* gameAction = m_trayMenu->addAction("Game Mode");    gameAction->setCheckable(true);
    connect(gameAction, &QAction::toggled, this, &SystemTrayManager::gameModeToggled);    m_trayMenu->addSeparator();
    auto* quitAction = m_trayMenu->addAction("Exit");    connect(quitAction, &QAction::triggered, this, &SystemTrayManager::exitRequested);    m_trayMenu->setStyleSheet(R"(        QMenu {            background-color: #1e1e2e;            border: 1px solid #313244;            color: #cdd6f4;        }        QMenu::item:selected {            background-color: #313244;        }        QMenu::separator {            height: 1px;            background: #313244;            margin: 4px 8px;        }    )");
}
void SystemTrayManager::updateTooltip(const QString& text) {
if(m_trayIcon) {        m_trayIcon->setToolTip(text);    }}
void SystemTrayManager::showNotification(const QString& title, const QString& message) {
if(m_trayIcon) {        m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 3000);    }}
void SystemTrayManager::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
if(reason == QSystemTrayIcon::DoubleClick) {        emit showDashboardRequested();    }}

} // namespace RAMFlux::UI


