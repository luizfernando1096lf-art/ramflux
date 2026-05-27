// RAMFlux - MemoryChartWidget Implementation
// Widget de gráfico de uso de memória para dashboard

#include "MemoryChartWidget.h"

MemoryChartWidget::MemoryChartWidget(QWidget *parent)
    : QWidget(parent),
      chart_(nullptr),
      chartView_(nullptr),
      usageSeries_(nullptr),
      availableSeries_(nullptr),
      lastUsed_(0),
      lastCached_(0),
      lastFree_(0)
{
    setupChart();
    setupUi();
}

MemoryChartWidget::~MemoryChartWidget()
{
    delete chart_;
    delete chartView_;
    delete usageSeries_;
    delete availableSeries_;
}

void MemoryChartWidget::setupChart()
{
    // Criar chart
    chart_ = new QChart();
    chart_->setTitle("Uso de Memória - RAM");
    chart_->setAnimationTypes(QChart::SeriesAnimation);
    
    // Criar e configurar séries
    usageSeries_ = new QLineSeries();
    usageSeries_->setName("Em Uso");
    usageSeries_->setPen(QPen(QColor("#e74c3c"), 2));
    usageSeries_->setBrush(QBrush(QColor("#e74c3c", 100)));
    
    availableSeries_ = new QLineSeries();
    availableSeries_->setName("Disponible");
    availableSeries_->setPen(QPen(QColor("#3498db"), 2));
    availableSeries_->setBrush(QBrush(QColor("#3498db", 100)));
    
    chart_->addSeries(usageSeries_);
    chart_->addSeries(availableSeries_);
    
    // Configurar axes
    chart_->createDefaultAxes();
    chart_->axisX("xAxis")->setTitle("Tempo");
    chart_->axisY("yAxis")->setTitle("MB");
    chart_->axisY("yAxis")->setLabelFormat("%1 MB");
    
    // Criar chart view
    chartView_ = new QChartView(chart_);
    chartView_->setWindowTitle("Gráfico de Memória");
    chartView_->setStyleSheet(
        "QChartView {"
        "    background: transparent;"
        "    border: none;"
        "}"
    );
}

void MemoryChartWidget::setupUi()
{
    setMinimumSize(400, 300);
    setStyleSheet(
        "QWidget {"
        "    background: rgba(0, 0, 0, 0.5);"
        "    border-radius: 10px;"
        "}"
    );
    
    // Adicionar à hierarquia
    if (parent()) {
        parent()->layout()->addWidget(chartView_);
    }
}

void MemoryChartWidget::setMemoryData(qint64 used, qint64 total, qint64 cached, qint64 free)
{
    // Adicionar novo ponto de dados
    if (usageSeries_ && availableSeries_) {
        // Adicionar timestamp atual
        usageSeries_->append(QDateTime::currentDateTime(), used);
        availableSeries_->append(QDateTime::currentDateTime(), free);
        
        // Manter apenas últimos 50 pontos
        if (usageSeries_->count() > 50) {
            usageSeries_->remove(0);
            availableSeries_->remove(0);
        }
    }
    
    lastUsed_ = used;
    lastCached_ = cached;
    lastFree_ = free;
    
    emit memoryUpdated();
}

void MemoryChartWidget::setHistoryData(const QVector<QVariantMap> &history)
{
    if (usageSeries_ && availableSeries_ && !history.isEmpty()) {
        // Preencher gráfico com histórico
        for (const auto &event : history) {
            if (event.contains("used")) {
                usageSeries_->append(
                    QDateTime::fromString(event["timestamp"].toString(), Qt::ISODate),
                    event["used"].toLongLong()
                );
            }
            if (event.contains("free")) {
                availableSeries_->append(
                    QDateTime::fromString(event["timestamp"].toString(), Qt::ISODate),
                    event["free"].toLongLong()
                );
            }
        }
    }
}

void MemoryChartWidget::adjustZoom(qreal factor)
{
    if (chart_) {
        chart_->applyGeneralTransform(QChart::ScaleX(factor));
    }
}

void MemoryChartWidget::resetZoom()
{
    if (chart_) {
        chart_->applyGeneralTransform(QChart::NoTransform);
    }
}

void MemoryChartWidget::onMemoryUpdate()
{
    // Reconfigurar gráfico com novos dados
    if (usageSeries_ && availableSeries_) {
        // Limpar gráfico atual
        usageSeries_->clear();
        availableSeries_->clear();
        
        // Adicionar dados históricos se disponível
        auto *engine = qobject_cast<QObject*>(sender());
        if (engine) {
            auto *telemetry = qobject_cast<FluxTelemetry*>(engine->parent());
            if (telemetry) {
                setHistoryData(telemetry->getEventHistory());
            }
        }
    }
}