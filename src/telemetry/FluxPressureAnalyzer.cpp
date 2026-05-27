// RAMFlux - FluxPressureAnalyzer Implementation
// Analisador de pressão de memória para detecção de "fluxo" de dados

#include "FluxPressureAnalyzer.h"

FluxPressureAnalyzer::FluxPressureAnalyzer(QObject *parent)
    : QObject(parent),
      baseline_(16384), // 16GB baseline
      threshold_(75.0), // 75% threshold
      sensitivity_(1.5) // Fator de sensibilidade
{
}

void FluxPressureAnalyzer::setBaseline(qint64 mb)
{
    baseline_ = mb;
}

void FluxPressureAnalyzer::setThreshold(qreal percent)
{
    threshold_ = percent;
}

void FluxPressureAnalyzer::setSensitivity(qreal factor)
{
    sensitivity_ = factor;
}

qint64 FluxPressureAnalyzer::getBaseline() const
{
    return baseline_;
}

qreal FluxPressureAnalyzer::getThreshold() const
{
    return threshold_;
}

qreal FluxPressureAnalyzer::getSensitivity() const
{
    return sensitivity_;
}

bool FluxPressureAnalyzer::hasFluxCondition() const
{
    return hasFluxCondition_;
}

QString FluxPressureAnalyzer::getFluxMessage() const
{
    return fluxMessage_;
}

void FluxPressureAnalyzer::analyze(qint64 currentTotal, qint64 currentUsed)
{
    if (baseline_ == 0) return;
    
    qreal currentPercent = (qreal)currentUsed / currentTotal * 100.0;
    qreal delta = currentPercent - threshold_;
    
    if (delta > 0 && delta > (sensitivity_ * threshold_)) {
        hasFluxCondition_ = true;
        fluxMessage_ = QString("Flux detected: %1MB used, %2% above baseline").arg(currentUsed).arg(delta);
        fluxStartTime_ = QDateTime::currentDateTime();
        
        emit fluxDetected(currentPercent, fluxMessage_);
        
        if (pressureAlert_) {
            pressureAlert_(currentPercent, fluxMessage_);
        }
    } else {
        hasFluxCondition_ = false;
        fluxMessage_ = QString("Flux condition resolved: %1% usage").arg(currentPercent);
    }
}

void FluxPressureAnalyzer::updateBaseline(qint64 mb)
{
    baseline_ = mb;
    hasFluxCondition_ = false;
    fluxMessage_ = QString("Baseline updated to %1MB").arg(mb);
}

void FluxPressureAnalyzer::setPressureAlert(QObject *alert)
{
    pressureAlert_ = alert;
}

int FluxPressureAnalyzer::getFluxDurationMs() const
{
    if (!hasFluxCondition_) return 0;
    
    auto ms = fluxStartTime_.msecsSince(QDateTime::currentDateTime());
    return ms < 0 ? -ms : ms;
}

QVariantMap FluxPressureAnalyzer::getFluxReport() const
{
    return {
        {"hasFluxCondition", hasFluxCondition()},
        {"baselineMB", baseline()},
        {"thresholdPercent", threshold()},
        {"sensitivityFactor", sensitivity()},
        {"fluxStartTime", fluxStartTime().toString(Qt::ISODate)},
        {"fluxDurationMs", getFluxDurationMs()},
        {"message", fluxMessage()},
    };
}

QVector<QVariantMap> FluxPressureAnalyzer::getFluxHistory() const
{
    QVector<QVariantMap> history;
    for (int i = 0; i < fluxHistory_.count() && i < 100; ++i) {
        history.append(fluxHistory_[fluxHistory_.count() - 1 - i]);
    }
    return history;
}

void FluxPressureAnalyzer::addFluxEvent(const QVariantMap &event)
{
    fluxHistory_.append(event);
}

void FluxPressureAnalyzer::clearHistory()
{
    fluxHistory_.clear();
}