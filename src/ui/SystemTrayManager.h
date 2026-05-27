// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
namespace RAMFlux::UI {
class SystemTrayManager : public QObject {
    Q_OBJECT
public:
    explicit SystemTrayManager(QObject* parent = nullptr);    ~SystemTrayManager() override;
bool initialize();
void updateTooltip(const QString& text);
void showNotification(const QString& title, const QString& message);
signals:    void showDashboardRequested();
void smartOptimizeRequested();
void deepCleanRequested();
void autoOptimizeToggled(bool enabled);
void gameModeToggled(bool enabled);
void exitRequested();
    private slots:    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    private:
    void setupMenu();    QSystemTrayIcon* m_trayIcon{
nullptr};
QMenu* m_trayMenu{
nullptr};
bool m_initialized{
false};
};
} // namespace RAMFlux::UI


