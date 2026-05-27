#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include "src/core/ISnapshotProcessor.h"

class QChart;
class QChartView;

class DashboardWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit DashboardWidget(QWidget *parent = nullptr);
    ~DashboardWidget() override = default;

protected:
    virtual void loadSnapshotData(ISnapshotProcessor& processor) = 0;
    virtual void updateView() = 0;
};

#endif // DASHBOARDWIDGET_H