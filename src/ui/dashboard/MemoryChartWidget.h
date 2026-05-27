// RAMFlux - MemoryChartWidget Header
// Widget de gráfico de memória para dashboard

#ifndef MEMORYCHARTWIDGET_H
#define MEMORYCHARTWIDGET_H

#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QScatterSeries>

/**
 * @brief Widget de gráfico de uso de memória
 */
class MemoryChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MemoryChartWidget(QWidget *parent = nullptr);
    ~MemoryChartWidget();

    // Configurar gráfico
    void setMemoryData(qint64 used, qint64 total, qint64 cached, qint64 free);
    void setHistoryData(const QVector<QVariantMap> &history);
    
    // Ajustar visualização
    void adjustZoom(qreal factor);
    void resetZoom();

signals:
    void memoryUpdated();

private slots:
    void onMemoryUpdate();

private:
    QChart *chart_;
    QChartView *chartView_;
    QLineSeries *usageSeries_;
    QLineSeries *availableSeries_;
    
    // Cache de dados
    qint64 lastUsed_;
    qint64 lastCached_;
    qint64 lastFree_;
};

#endif // MEMORYCHARTWIDGET_H