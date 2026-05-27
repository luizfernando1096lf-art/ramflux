// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "HistoryChart.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPen>
#include <QLabel>
namespace RAMFlux::UI {
HistoryChart::HistoryChart(QWidget* parent) : QWidget(parent) {    setupChart();
}
void HistoryChart::setupChart() {    auto* layout = new QVBoxLayout(this);    layout->setContentsMargins(6, 6, 6, 6);    layout->setSpacing(6);    setMinimumHeight(280);
    auto* controlLayout = new QHBoxLayout();
    auto* label = new QLabel("Overlay:", this);    label->setStyleSheet("color: #a6adc8; font-size: 11px");    controlLayout->addWidget(label);    m_metricCombo = new QComboBox(this);    m_metricCombo->addItems({        "None", "RAM Usage (GB)", "Free RAM (GB)",        "Commit Usage (GB)", "Standby Memory (GB)"    });    m_metricCombo->setCurrentIndex(1);    m_metricCombo->setStyleSheet(R"(        QComboBox {            background-color: #313244; color: #cdd6f4;            border: 1px solid #45475a; border-radius: 4px;            padding: 4px 8px;        }        QComboBox::drop-down { border: none; }        QComboBox QAbstractItemView {            background-color: #313244; color: #cdd6f4;            selection-background-color: #45475a;        }    )");    connect(m_metricCombo, &QComboBox::currentIndexChanged, this, [this]() { updateSeries(); }
);    controlLayout->addWidget(m_metricCombo);    controlLayout->addStretch();    layout->addLayout(controlLayout);    m_chart = new QChart();    m_chart->setTitle("Memory Timeline");    m_chart->setMargins(QMargins(12, 8, 12, 8));    m_chart->setBackgroundBrush(QColor("#1e1e2e"));    m_chart->setPlotAreaBackgroundBrush(QColor("#181825"));    m_chart->setPlotAreaBackgroundVisible(true);    m_chart->legend()->setVisible(true);    m_chart->legend()->setLabelColor(QColor("#cdd6f4"));    m_chart->setTitleBrush(QColor("#cdd6f4"));    m_loadSeries = new QLineSeries();    m_loadSeries->setName("Memory Load %");    QPen loadPen(QColor("#89b4fa"));    loadPen.setWidth(2);    m_loadSeries->setPen(loadPen);    m_pressureSeries = new QLineSeries();    m_pressureSeries->setName("Pressure Score");    QPen pressurePen(QColor("#f38ba8"));    pressurePen.setWidth(2);    m_pressureSeries->setPen(pressurePen);    m_overlaySeries = new QLineSeries();    QPen overlayPen(QColor("#a6e3a1"));    overlayPen.setWidth(2);    m_overlaySeries->setPen(overlayPen);    m_chart->addSeries(m_loadSeries);    m_chart->addSeries(m_pressureSeries);    m_axisX = new QDateTimeAxis();    m_axisX->setFormat("hh:mm:ss");    m_axisX->setLabelsColor(QColor("#a6adc8"));    m_axisX->setLinePenColor(QColor("#45475a"));    m_axisX->setGridLineColor(QColor("#313244"));    m_chart->addAxis(m_axisX, Qt::AlignBottom);    m_loadSeries->attachAxis(m_axisX);    m_pressureSeries->attachAxis(m_axisX);    m_axisY = new QValueAxis();    m_axisY->setRange(0, 100);    m_axisY->setLabelsColor(QColor("#a6adc8"));    m_axisY->setLinePenColor(QColor("#45475a"));    m_axisY->setGridLineColor(QColor("#313244"));    m_chart->addAxis(m_axisY, Qt::AlignLeft);    m_loadSeries->attachAxis(m_axisY);    m_pressureSeries->attachAxis(m_axisY);    m_chartView = new QChartView(m_chart);    m_chartView->setRenderHint(QPainter::Antialiasing);    m_chartView->setStyleSheet("background: transparent");    layout->addWidget(m_chartView, 1);
}
void HistoryChart::addDataPoint(double usedGB, double freeGB, double loadPct,                                 double pressure, double commitGB, double standbyGB) {    QDateTime now = QDateTime::currentDateTime();    m_timestamps.push_back(now);    m_usedValues.push_back(usedGB);    m_freeValues.push_back(freeGB);    m_loadValues.push_back(loadPct);    m_pressureValues.push_back(pressure);    m_commitValues.push_back(commitGB);    m_standbyValues.push_back(standbyGB);
    while(static_cast<int>(m_timestamps.size()) > m_maxPoints) {        m_timestamps.pop_front();        m_usedValues.pop_front();        m_freeValues.pop_front();        m_loadValues.pop_front();        m_pressureValues.pop_front();        m_commitValues.pop_front();        m_standbyValues.pop_front();    }    updateSeries();
}
void HistoryChart::updateSeries() {    m_loadSeries->clear();    m_pressureSeries->clear();    m_overlaySeries->clear();
    for(size_t i = 0; i < m_timestamps.size(); ++i) {        qreal x = m_timestamps[i].toMSecsSinceEpoch();        m_loadSeries->append(x, m_loadValues[i]);        m_pressureSeries->append(x, m_pressureValues[i]);    }    if (m_chart->series().contains(m_overlaySeries))        m_chart->removeSeries(m_overlaySeries);
std::deque<double>* overlayValues = nullptr;
QString overlayName;
double maxVal = 100;    QColor overlayColor;
    switch(m_metricCombo->currentIndex()) {
case 1:            overlayValues = &m_usedValues;            overlayName = "RAM Usage (GB)";            maxVal = 64;            overlayColor = QColor("#a6e3a1");
    break;
case 2:            overlayValues = &m_freeValues;            overlayName = "Free RAM (GB)";            maxVal = 64;            overlayColor = QColor("#94e2d5");
    break;
case 3:            overlayValues = &m_commitValues;            overlayName = "Commit (GB)";            maxVal = 64;            overlayColor = QColor("#cba6f7");
    break;
case 4:            overlayValues = &m_standbyValues;            overlayName = "Standby (GB)";            maxVal = 64;            overlayColor = QColor("#fab387");
    break;
default:            break;    }    if (overlayValues) {
for(size_t i = 0; i < m_timestamps.size(); ++i) {            m_overlaySeries->append(m_timestamps[i].toMSecsSinceEpoch(), (*overlayValues)[i]);        }        QPen pen(overlayColor);        pen.setWidth(2);        m_overlaySeries->setPen(pen);        m_overlaySeries->setName(overlayName);        m_chart->addSeries(m_overlaySeries);        m_overlaySeries->attachAxis(m_axisX);        m_overlaySeries->attachAxis(m_axisY);    }    if (!m_timestamps.empty()) {        m_axisX->setRange(m_timestamps.front(), m_timestamps.back());    }    double dataMax = maxVal;
    for(size_t i = 0; i < m_loadValues.size(); ++i) {
if(m_loadValues[i] > dataMax) dataMax = m_loadValues[i];
    if(m_pressureValues[i] > dataMax) dataMax = m_pressureValues[i];    }    m_axisY->setRange(0, std::max(dataMax + 10, 100.0));
}
void HistoryChart::setMaxPoints(int points) {    m_maxPoints = points;
}

} // namespace RAMFlux::UI


