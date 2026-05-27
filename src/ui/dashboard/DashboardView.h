// RAMFlux - Dashboard View
// Componente visual do dashboard premium com Fluent Design

#ifndef DASHBOARD_VIEW_H
#define DASHBOARD_VIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QChart>
#include <QLayout>

#include "DashboardController.h"

class DashboardView : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardView(DashboardController *controller, QWidget *parent = nullptr);
    ~DashboardView();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupLayout();
    void setupCards();
    void setupCharts();
    void updateDashboard();
    void updateMemoryCard(double usedMB, double totalMB);
    void updateProcessesCard(int count);
    void updateSystemLoadCard(double load);
    void updateChart(const QList<double> &data);

    DashboardController *controller_;

    // Layout cards
    QWidget *cardsContainer_;
    QHBoxLayout *cardsLayout_;

    // Cards Fluent
    QWidget *memoryCard_;
    QWidget *processesCard_;
    QWidget *systemLoadCard_;

    QFrame *memoryCardFrame_;
    QFrame *processesCardFrame_;
    QFrame *systemLoadCardFrame_;

    QFrame *chartCardFrame_;

    // Charts
    QChart *memoryChart_;
    QChart *processesChart_;
    QChart *systemLoadChart_;
};

#endif // DASHBOARD_VIEW_H