// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <QWidget>
#include <QComboBox>
#include <QDateTime>
#include <QLineSeries>
#include <QChart>
#include <QChartView>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <deque>
namespace RAMFlux::UI {
class HistoryChart : public QWidget {
    Q_OBJECT
public:
    explicit HistoryChart(QWidget* parent = nullptr);
void addDataPoint(double usedGB, double freeGB, double loadPct,                      double pressure, double commitGB, double standbyGB);
void setMaxPoints(int points);
    private:
    void setupChart();
void updateSeries();    QChart* m_chart;    QChartView* m_chartView;    QComboBox* m_metricCombo;    QLineSeries* m_loadSeries;    QLineSeries* m_pressureSeries;    QLineSeries* m_overlaySeries;    QValueAxis* m_axisY;    QDateTimeAxis* m_axisX;
std::deque<QDateTime> m_timestamps;
std::deque<double> m_usedValues;
std::deque<double> m_freeValues;
std::deque<double> m_loadValues;
std::deque<double> m_pressureValues;
std::deque<double> m_commitValues;
std::deque<double> m_standbyValues;
int m_maxPoints{
120};
};
} // namespace RAMFlux::UI


