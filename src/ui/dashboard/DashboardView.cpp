// RAMFlux - Dashboard View Implementation

#include "DashboardView.h"
#include <QPainter>
#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QBarSeries>
#include <QValueAxis>
#include <QFont>
#include <QColor>
#include <QtCharts>
#include <QtWidgets>

// Paleta de cores Fluent
namespace {
    constexpr QColor cardBackground(QColor(255, 255, 255));
    constexpr QColor cardBorder(QColor(240, 240, 240));
    constexpr QColor accentBlue(QColor(0, 120, 212));
    constexpr QColor accentGreen(QColor(0, 180, 92));
    constexpr QColor accentOrange(QColor(240, 150, 64));
    constexpr QColor accentRed(QColor(220, 55, 75));
    constexpr QColor accentPurple(QColor(145, 80, 255));
}

DashboardView::DashboardView(DashboardController *controller, QWidget *parent)
    : QWidget(parent)
    , controller_(controller)
    , cardsContainer_(nullptr)
    , cardsLayout_(nullptr)
    , memoryCard_(nullptr)
    , processesCard_(nullptr)
    , systemLoadCard_(nullptr)
    , memoryCardFrame_(nullptr)
    , processesCardFrame_(nullptr)
    , systemLoadCardFrame_(nullptr)
    , chartCardFrame_(nullptr)
    , memoryChart_(nullptr)
    , processesChart_(nullptr)
    , systemLoadChart_(nullptr)
{
    setupLayout();
}

DashboardView::~DashboardView()
{
}

void DashboardView::setupLayout()
{
    // Container principal dos cards
    cardsContainer_ = new QWidget(this);
    cardsLayout_ = new QHBoxLayout(cardsContainer_);
    cardsLayout_->setSpacing(12);
    cardsLayout_->setContentsMargins(12, 12, 12, 12);

    // Card de Memória
    memoryCard_ = createFluentCard(cardsContainer_, "Memory Usage");
    memoryCardFrame_ = qobject_cast<QFrame*>(memoryCard_);
    memoryCardFrame_->setLayout(new QVBoxLayout());
    memoryCardFrame_->layout()->setContentsMargins(15, 15, 15, 15);

    // Card de Processos
    processesCard_ = createFluentCard(cardsContainer_, "Active Processes");
    processesCardFrame_ = qobject_cast<QFrame*>(processesCard_);
    processesCardFrame_->setLayout(new QVBoxLayout());
    processesCardFrame_->layout()->setContentsMargins(15, 15, 15, 15);

    // Card de System Load
    systemLoadCard_ = createFluentCard(cardsContainer_, "System Load");
    systemLoadCardFrame_ = qobject_cast<QFrame*>(systemLoadCard_);
    systemLoadCardFrame_->setLayout(new QVBoxLayout());
    systemLoadCardFrame_->layout()->setContentsMargins(15, 15, 15, 15);

    // Card de Charts
    chartCardFrame_ = new QFrame(cardsContainer_);
    chartCardFrame_->setLayout(new QVBoxLayout());
    chartCardFrame_->layout()->setContentsMargins(15, 15, 15, 15);

    cardsLayout_->addWidget(memoryCard_, 1);
    cardsLayout_->addWidget(processesCard_, 1);
    cardsLayout_->addWidget(systemLoadCard_, 1);
    cardsLayout_->addWidget(chartCardFrame_, 3);

    setupCharts();
}

QWidget* DashboardView::createFluentCard(QWidget *parent, const QString &title)
{
    QWidget *card = new QWidget(parent);
    card->setFixedHeight(120);
    
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(10, 10, 10, 10);

    // Título
    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setFont(QFont("Segoe UI", 12, QFont::Bold));
    titleLabel->setStyleSheet("color: #1A1A1A;");
    layout->addWidget(titleLabel);

    // Conteúdo do card
    QLabel *titleLabel2 = new QLabel("Dashboard", card);
    titleLabel2->setFont(QFont("Segoe UI", 14, QFont::Bold));
    titleLabel2->setStyleSheet("color: " + QColor(0, 120, 212).name());
    layout->addWidget(titleLabel2);

    return card;
}

void DashboardView::setupCharts()
{
    // ChartView de memória
    QChartView *memoryChartView = new QChartView(this);
    memoryChartView->setChart(memoryChart_);
    memoryChartView->setRenderHint(QPainter::Antialiasing);
    memoryChartView->setRenderHint(QPainter::HighQualityAntialiasing);
    memoryChartView->setMinimumHeight(200);

    chartCardFrame_->layout()->addWidget(memoryChartView);
}

void DashboardView::setupCards()
{
    // Cards já foram criados em setupLayout()
}

void DashboardView::updateDashboard()
{
    // Atualiza os cards com dados do controller
    MemoryMetrics metrics = controller_->getMemoryMetrics();
    int processCount = controller_->getActiveProcessCount();
    double systemLoad = controller_->getSystemLoad();
    QList<double> chartData = controller_->getChartData();

    updateMemoryCard(metrics.usedMB, metrics.totalMB);
    updateProcessesCard(processCount);
    updateSystemLoadCard(systemLoad);
    updateChart(chartData);
}

void DashboardView::updateMemoryCard(double usedMB, double totalMB)
{
    // Atualizar o card de memória
}

void DashboardView::updateProcessesCard(int count)
{
    // Atualizar o card de processos
}

void DashboardView::updateSystemLoadCard(double load)
{
    // Atualizar o card de system load
}

void DashboardView::updateChart(const QList<double> &data)
{
    // Atualizar o chart principal
}

void DashboardView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fundo com gradiente sutil
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(255, 255, 255));
    gradient.setColorAt(1, QColor(248, 250, 252));
    painter.fillRect(rect(), gradient);

    painter.end();
}