// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "HeuristicEngine.h"
#include "core/EventBus.h"
#include "core/Logger.h"
#include "core/FluxCore.h"
#include "telemetry/FluxTelemetry.h"
#include "scheduler/FluxScheduler.h"
#include "cleaner/FluxCleaner.h"
#include "rules/ProcessRulesEngine.h"
#include <chrono>
#include <thread>
#include <exception>
#include <cmath>
#include <cstddef>
#include <cfloat>
#include <algorithm>
namespace RAMFlux::AI {
using RAMFlux::Core::Logger;
HeuristicEngine::HeuristicEngine() {
    m_lastTuneTime = std::chrono::steady_clock::now();
    m_lastCleanTimeEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
HeuristicEngine::~HeuristicEngine() { shutdown(); }
bool HeuristicEngine::initialize() {
    Logger::instance().info("[HeuristicEngine] Initializing AI heuristics...");
    m_cleanStartedSubId = Core::EventBus::instance().subscribe(Constants::EventType::CleaningStarted,
        [this]() { m_ioCostTracker.beforeClean(); });
    m_cleanFinishedSubId = Core::EventBus::instance().subscribe(Constants::EventType::CleaningFinished,
        [this]() {
            m_lastCleanTimeEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            m_ioCostTracker.afterClean();
        });
    m_running = true;
    m_thread = std::thread(&HeuristicEngine::analysisLoop, this);
    Logger::instance().info("[HeuristicEngine] AI thread started");
    return true;
}
void HeuristicEngine::shutdown() {
    if(m_running.exchange(false)) {
        if(m_thread.joinable()) {
            m_thread.join();
        }
        auto& bus = Core::EventBus::instance();
        if(m_cleanStartedSubId) bus.unsubscribe(Constants::EventType::CleaningStarted, m_cleanStartedSubId);
        if(m_cleanFinishedSubId) bus.unsubscribe(Constants::EventType::CleaningFinished, m_cleanFinishedSubId);
        m_cleanStartedSubId = 0;
        m_cleanFinishedSubId = 0;
        Logger::instance().info("[HeuristicEngine] Shutdown complete");
    }
}
std::string HeuristicEngine::name() const {
    return "HeuristicEngine";
}
void HeuristicEngine::feedSnapshot(const Telemetry::MemorySnapshot& snap) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_predictor.feed(snap);
        m_classifier.classify(snap);
    }
    evaluateAndPost(snap);
}
void HeuristicEngine::analysisLoop() {
    while(m_running) {
        try {
            auto& core = Core::FluxCore::instance();
            auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
                core.moduleManager().getModule("FluxTelemetry"));
            if(telemetry) {
                auto snap = telemetry->lastSnapshot();
                feedSnapshot(snap);
            }
            adjustModuleParams();
        } catch(const std::exception& e) {
            Logger::instance().error(std::string("[HeuristicEngine] Error: ") + e.what());
        } catch(...) {
            Logger::instance().error("[HeuristicEngine] Unknown error");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(m_intervalMs.load()));
    }
}
void HeuristicEngine::storePrediction(double predictedPressure, int horizonSeconds) {
    EffectivenessSample sample;
    sample.madeAt = std::chrono::steady_clock::now();
    sample.horizonSeconds = horizonSeconds;
    sample.predictedPressure = predictedPressure;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_predictionHistory.push_back(sample);
    while(m_predictionHistory.size() > MAX_PREDICTION_HISTORY)
        m_predictionHistory.pop_front();
}
void HeuristicEngine::evaluatePredictionAccuracy(const Telemetry::MemorySnapshot& snap) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_predictionHistory.empty()) return;
    auto now = std::chrono::steady_clock::now();
    int evaluated = 0;
    for(auto& sample : m_predictionHistory) {
        if(sample.evaluated) continue;
        auto elapsed = std::chrono::duration<double>(now - sample.madeAt).count();
        if(elapsed >= sample.horizonSeconds) {
            double actual = static_cast<double>(snap.pressureScore);
            sample.actualPressure = actual;
            double diff = std::abs(sample.predictedPressure - actual);
            double maxVal = std::max(sample.predictedPressure, actual);
            sample.errorPercent = (maxVal > 0.001) ? (diff / maxVal) * 100.0 : 0.0;
            double threshold = std::max(10.0, actual * 0.15);
            sample.wasCorrect = (diff <= threshold);
            sample.evaluated = true;
            evaluated++;
        }
    }
    if(evaluated == 0) return;
    uint64_t total = 0, correct = 0, fp = 0, fn = 0;
    double errorSum = 0.0;
    int errorCount = 0;
    for(const auto& s : m_predictionHistory) {
        if(!s.evaluated) continue;
        total++;
        if(s.wasCorrect) correct++;
        errorSum += s.errorPercent;
        errorCount++;
        if(s.predictedPressure >= Constants::PRESSURE_HIGH_MAX
            && s.actualPressure < Constants::PRESSURE_NORMAL_MAX)
            fp++;
        if(s.predictedPressure < Constants::PRESSURE_NORMAL_MAX
            && s.actualPressure >= Constants::PRESSURE_HIGH_MAX)
            fn++;
    }
    m_effectiveness.totalPredictions = total;
    m_effectiveness.correctPredictions = correct;
    m_effectiveness.accuracy = (total > 0) ? (static_cast<double>(correct) / total) * 100.0 : 0.0;
    m_effectiveness.meanError = (errorCount > 0) ? errorSum / errorCount : 0.0;
    m_effectiveness.falsePositives = fp;
    m_effectiveness.falseNegatives = fn;
    size_t recentCount = std::min(m_predictionHistory.size(), ACCURACY_WINDOW);
    if(recentCount > 0) {
        int recentCorrect = 0;
        int recentTotal = 0;
        auto start = m_predictionHistory.end() - static_cast<ptrdiff_t>(recentCount);
        for(auto it = start; it != m_predictionHistory.end(); ++it) {
            if(!it->evaluated) continue;
            recentTotal++;
            if(it->wasCorrect) recentCorrect++;
        }
        m_effectiveness.recentAccuracy = (recentTotal > 0)
            ? (static_cast<double>(recentCorrect) / recentTotal) * 100.0 : 0.0;
    }
    auto oldest = m_predictionHistory.front();
    auto newest = m_predictionHistory.back();
    auto predAge = std::chrono::duration<double>(newest.madeAt - oldest.madeAt).count();
    if(predAge > 600.0) {
        m_predictionHistory.erase(
            m_predictionHistory.begin(),
            m_predictionHistory.begin() + static_cast<ptrdiff_t>(m_predictionHistory.size() / 2));
    }
}
void HeuristicEngine::tuneFromMetrics() {
    auto& e = m_effectiveness;
    double accuracy = e.accuracy;
    double recentAcc = e.recentAccuracy;
    double useAcc = accuracy;
    if(e.totalPredictions < 5) return;
    if(e.totalPredictions < 10 && recentAcc > 0.0) {
        useAcc = recentAcc;
    }
    TuningParams p;
    if(useAcc >= 80.0) {
        p.aggressiveFactor = 1.3;
        p.recommendedCooldownMs = 20000;
        p.recommendedIntervalMs = 3000;
        p.recommendedConfidenceThreshold = 0.5;
    } else if(useAcc >= 65.0) {
        p.aggressiveFactor = 1.0;
        p.recommendedCooldownMs = 30000;
        p.recommendedIntervalMs = 5000;
        p.recommendedConfidenceThreshold = 0.6;
    } else if(useAcc >= 45.0) {
        p.aggressiveFactor = 0.7;
        p.recommendedCooldownMs = 45000;
        p.recommendedIntervalMs = 8000;
        p.recommendedConfidenceThreshold = 0.7;
    } else {
        p.aggressiveFactor = 0.4;
        p.recommendedCooldownMs = 60000;
        p.recommendedIntervalMs = 12000;
        p.recommendedConfidenceThreshold = 0.8;
    }
    if(e.falsePositives > e.correctPredictions / 2) {
        p.recommendedConfidenceThreshold = std::min(p.recommendedConfidenceThreshold + 0.1, 0.95);
    }
    if(e.falseNegatives > e.correctPredictions / 2) {
        p.recommendedConfidenceThreshold = std::max(p.recommendedConfidenceThreshold - 0.1, 0.3);
    }
    auto& core = Core::FluxCore::instance();
    auto* telemetry = dynamic_cast<Telemetry::FluxTelemetry*>(
        core.moduleManager().getModule("FluxTelemetry"));
    if(telemetry) {
        auto snap = telemetry->lastSnapshot();
        auto trend = m_predictor.trendSlope();
        if(trend > 0.1) {
            p.recommendedIntervalMs = std::max(2000, p.recommendedIntervalMs - 2000);
        } else if(trend < -0.1) {
            p.recommendedIntervalMs = std::min(15000, p.recommendedIntervalMs + 2000);
        }
        auto anomaly = m_predictor.detectAnomaly();
        if(anomaly.isAnomaly && anomaly.deviation > 4.0) {
            p.recommendedCooldownMs = std::max(p.recommendedCooldownMs, 45000);
        }
    }
    m_tuningParams = p;
}
void HeuristicEngine::adjustModuleParams() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastTuneTime).count();
    if(elapsed < m_tuneIntervalMs) return;
    m_lastTuneTime = now;
    try {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            tuneFromMetrics();
        }
        auto& core = Core::FluxCore::instance();
        auto* scheduler = dynamic_cast<Scheduler::FluxScheduler*>(
            core.moduleManager().getModule("FluxScheduler"));
        auto* cleaner = dynamic_cast<Cleaner::FluxCleaner*>(
            core.moduleManager().getModule("FluxCleaner"));
        if(!scheduler || !cleaner) return;
        TuningParams params;
        int recentAccuracy = 0;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            params = m_tuningParams;
            recentAccuracy = static_cast<int>(m_effectiveness.recentAccuracy);
        }
        int currentCooldown = cleaner->cooldownMs();
        int targetCooldown = params.recommendedCooldownMs;
        if(currentCooldown != targetCooldown) {
            cleaner->setCooldownMs(targetCooldown);
            Logger::instance().info(std::string("[HeuristicEngine] Auto-tune: cleaner cooldown ")
                + std::to_string(currentCooldown) + "ms -> " + std::to_string(targetCooldown) + "ms"
                + " (accuracy: " + std::to_string(recentAccuracy) + "%)");
        }
        int currentInterval = scheduler->scheduleIntervalMs();
        int targetInterval = params.recommendedIntervalMs;
        if(currentInterval != targetInterval) {
            scheduler->setScheduleIntervalMs(targetInterval);
            scheduler->setIntervalMs(targetInterval);
            Logger::instance().info(std::string("[HeuristicEngine] Auto-tune: scheduler interval ")
                + std::to_string(currentInterval) + "ms -> " + std::to_string(targetInterval) + "ms"
                + " (accuracy: " + std::to_string(recentAccuracy) + "%)");
        }
    } catch(const std::exception& e) {
        Logger::instance().error(std::string("[HeuristicEngine] Tuning error: ") + e.what());
    } catch(...) {
        Logger::instance().error("[HeuristicEngine] Tuning unknown error");
    }
}
void HeuristicEngine::evaluateAndPost(const Telemetry::MemorySnapshot& snap) {
    auto wType = m_classifier.currentType();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(wType != m_lastPostedWorkload) {
            m_lastPostedWorkload = wType;
            Core::EventBus::instance().post(Constants::EventType::WorkloadChanged);
            auto* rules = dynamic_cast<Rules::ProcessRulesEngine*>(
                Core::FluxCore::instance().moduleManager().getModule("ProcessRulesEngine"));
            if(rules) rules->setCurrentWorkload(static_cast<int>(wType));
            Logger::instance().info(std::string("[HeuristicEngine] Workload changed: ")
                + Constants::WorkloadNames[static_cast<int>(wType)]
                + " (conf: " + std::to_string(m_classifier.confidence()) + ")");
        }
    }
    auto anomaly = m_predictor.detectAnomaly();
    if(anomaly.isAnomaly) {
        bool shouldPost = false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration<double>(now - m_lastAnomalyPost).count();
            if(elapsed > 30.0) {
                m_lastAnomalyPost = now;
                shouldPost = true;
            }
        }
        if(shouldPost) {
            Core::EventBus::instance().post(Constants::EventType::AnomalyDetected);
            Logger::instance().warn(std::string("[HeuristicEngine] Memory anomaly: ")
                + std::to_string(anomaly.deviation) + "σ deviation");
        }
    }
    m_hardFault.feed(snap);
    auto hfPred = m_hardFault.evaluate();
    auto cleanTime = std::chrono::steady_clock::time_point(
        std::chrono::milliseconds(m_lastCleanTimeEpoch.load()));
    auto mlFeats = m_mlEngine.extractFeatures(snap, cleanTime);
    auto mlPred = m_mlEngine.predict(mlFeats);
    m_mlEngine.processTraining(static_cast<double>(snap.pressureScore));
    m_ioCostTracker.evaluateCosts();
    m_standbyScanner.refresh();
    m_pagePrefetcher.refresh();
    auto qosResult = m_qos.enforce(snap);
    auto dedupResult = m_dedup.scan(snap);
    auto pred30 = m_predictor.predict(30);
    auto pred60 = m_predictor.predict(60);
    auto pred120 = m_predictor.predict(120);
    storePrediction(pred30.predictedPressure, 30);
    storePrediction(pred60.predictedPressure, 60);
    storePrediction(pred120.predictedPressure, 120);
    evaluatePredictionAccuracy(snap);
    if(pred30.confidence >= Constants::AI_PREDICTION_CONFIDENCE_MIN
        && pred30.predictedPressure >= Constants::PRESSURE_HIGH_MAX) {
        Core::EventBus::instance().post(Constants::EventType::PressurePredicted);
    }
    if(pred60.confidence >= 0.6 && pred60.predictedPressure >= 85.0) {
        Core::EventBus::instance().post(Constants::EventType::PressurePredicted);
        Logger::instance().warn(std::string("[HeuristicEngine] 60s CRITICAL prediction: ")
            + std::to_string(static_cast<int>(pred60.predictedPressure)) + "% (conf "
            + std::to_string(static_cast<int>(pred60.confidence*100)) + "%) — pre-emptive standby suggested");
    }
    HeuristicReport r;
    r.workload = wType;
    r.workloadConfidence = m_classifier.confidence();
    r.workloadTrigger = m_classifier.triggeredBy();
    r.predictedPressure30s = pred30.predictedPressure;
    r.predictedPressure60s = pred60.predictedPressure;
    r.predictedPressure120s = pred120.predictedPressure;
    r.predictionConfidence = pred60.confidence;
    r.trendSlope = m_predictor.trendSlope();
    r.anomalyDetected = anomaly.isAnomaly;
    r.anomalyDeviation = anomaly.deviation;
    r.totalSamples = static_cast<uint64_t>(m_predictor.sampleCount());
    r.effectiveness = m_effectiveness;
    r.tuning = m_tuningParams;
    r.hardFault = hfPred;
    r.mlScore = mlPred.score30s;
    r.mlConfidence = mlPred.confidence;
    r.mlSampleCount = mlPred.sampleCount;
    r.mlEnabled = m_mlEngine.isEnabled();
    r.ioCost = m_ioCostTracker.report();
    r.standby = m_standbyScanner.report();
    r.prefetch = m_pagePrefetcher.tryPrefetch(
        r.mlScore, r.hardFault.stormWarning,
        r.ioCost.topCostProcesses,
        r.standby.topProcesses);
    r.qos = qosResult;
    r.dedup = dedupResult;
    if(pred30.predictedPressure >= Constants::PRESSURE_CRITICAL_MAX)
        r.currentPressureLevel = Constants::PressureLevel::Critical;
    else if(pred30.predictedPressure >= Constants::PRESSURE_HIGH_MAX)
        r.currentPressureLevel = Constants::PressureLevel::High;
    else if(pred30.predictedPressure >= Constants::PRESSURE_NORMAL_MAX)
        r.currentPressureLevel = Constants::PressureLevel::Normal;
    else
        r.currentPressureLevel = Constants::PressureLevel::Idle;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(r.currentPressureLevel != m_lastPressureLevel) {
            auto prevLevel = m_lastPressureLevel;
            m_lastPressureLevel = r.currentPressureLevel;
            Core::EventBus::instance().post(Constants::EventType::PressureChanged);
            if(r.currentPressureLevel < prevLevel)
                Core::EventBus::instance().post(Constants::EventType::PressureDropped);
        }
        if(r.hardFault.stormWarning && !m_lastStormWarning) {
            m_lastStormWarning = true;
            Core::EventBus::instance().post(Constants::EventType::HardFaultStorm);
        } else if(!r.hardFault.stormWarning && m_lastStormWarning) {
            m_lastStormWarning = false;
            Core::EventBus::instance().post(Constants::EventType::HardFaultStormCleared);
        }
        m_latestReport = r;
    }
}
EffectivenessMetrics HeuristicEngine::currentEffectiveness() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_effectiveness;
}
TuningParams HeuristicEngine::currentTuning() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tuningParams;
}
HeuristicReport HeuristicEngine::currentReport() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_latestReport;
}
void HeuristicEngine::HardFaultPredictor::feed(const Telemetry::MemorySnapshot& snap) {
    HardFaultSample s;
    s.time = std::chrono::steady_clock::now();
    s.hardFaultsPerSec = static_cast<double>(snap.hardFaultsPerSec);
    s.diskQueueLength = static_cast<double>(snap.diskQueueLength);
    s.standbyGB = snap.standbyRamGB();
    m_samples.push_back(s);
    while(m_samples.size() > MAX_SAMPLES)
        m_samples.pop_front();
}
double HeuristicEngine::HardFaultPredictor::computeSlope(
    const std::deque<HardFaultSample>& samples,
    double HardFaultSample::*field) const
{
    if(samples.size() < 3) return 0.0;
    auto t0 = samples.front().time;
    size_t n = samples.size();
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    for(size_t i = 0; i < n; ++i) {
        double x = std::chrono::duration<double>(samples[i].time - t0).count();
        double y = samples[i].*field;
        sumX += x; sumY += y; sumXY += x * y; sumX2 += x * x;
    }
    double denom = n * sumX2 - sumX * sumX;
    return (std::abs(denom) > 1e-10) ? (n * sumXY - sumX * sumY) / denom : 0.0;
}
HardFaultPrediction HeuristicEngine::HardFaultPredictor::evaluate() {
    HardFaultPrediction result;
    if(m_samples.size() < 3) return result;
    double hfSlope = computeSlope(m_samples, &HardFaultSample::hardFaultsPerSec);
    double dqSlope = computeSlope(m_samples, &HardFaultSample::diskQueueLength);
    double sbSlope = computeSlope(m_samples, &HardFaultSample::standbyGB);
    result.hardFaultTrendSlope = hfSlope;
    result.diskQueueTrendSlope = dqSlope;
    result.standbyTrendSlope = sbSlope;
    const auto& latest = m_samples.back();
    double currentHF = latest.hardFaultsPerSec;
    double currentDQ = latest.diskQueueLength;
    double currentSB = latest.standbyGB;
    result.predictedFaultsIn30s = std::max(0.0, currentHF + hfSlope * 30.0);
    double score = 0.0;
    if(currentHF >= 500.0) score += 40.0;
    else if(currentHF >= 200.0) score += 30.0;
    else if(currentHF >= 100.0) score += 20.0;
    else if(currentHF >= 50.0) score += 10.0;
    if(hfSlope > 10.0) score += 25.0;
    else if(hfSlope > 5.0) score += 18.0;
    else if(hfSlope > 2.0) score += 10.0;
    else if(hfSlope > 1.0) score += 5.0;
    if(currentDQ >= 3.0) score += 20.0;
    else if(currentDQ >= 2.0) score += 13.0;
    else if(currentDQ >= 1.0) score += 7.0;
    if(currentSB < 0.5) score += 15.0;
    else if(currentSB < 1.0) score += 10.0;
    else if(currentSB < 2.0) score += 5.0;
    result.severityScore = std::clamp(score, 0.0, 100.0);
    if(score >= 70.0) {
        result.severity = HardFaultSeverity::Critical;
        result.triggerReason = "critical_hard_fault_storm";
    } else if(score >= 50.0) {
        result.severity = HardFaultSeverity::High;
        result.triggerReason = "high_hard_fault_pressure";
    } else if(score >= 30.0) {
        result.severity = HardFaultSeverity::Medium;
        result.triggerReason = "elevated_hard_fault_risk";
    } else if(score >= 15.0) {
        result.severity = HardFaultSeverity::Low;
        result.triggerReason = "slight_fault_increase";
    }
    double sampleConf = std::clamp(static_cast<double>(m_samples.size()) / 30.0, 0.1, 1.0);
    double trendStrength = std::clamp(std::abs(hfSlope) / 10.0, 0.0, 1.0);
    result.confidence = std::clamp(sampleConf * (0.5 + 0.5 * trendStrength), 0.0, 0.95);
    bool severe = (score >= 50.0);
    bool rising = (hfSlope > 3.0 || dqSlope > 0.5);
    bool lowCache = currentSB < 1.0;
    bool highFaults = currentHF >= 100.0;
    result.stormWarning = (severe && rising && (lowCache || highFaults));
    if(result.stormWarning) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - m_lastWarning).count();
        if(elapsed > 30.0) {
            m_lastWarning = now;
            Logger::instance().warn(std::string("[HardFaultPredictor] Storm warning: ")
                + "severity " + std::to_string(static_cast<int>(result.severity))
                + " score " + std::to_string(static_cast<int>(score))
                + " faults/s " + std::to_string(static_cast<int>(currentHF))
                + " diskQ " + std::to_string(currentDQ)
                + " standby " + std::to_string(currentSB) + "GB"
                + " trend " + std::to_string(hfSlope));
        }
    }
    return result;
}
void HeuristicEngine::HardFaultPredictor::reset() {
    m_samples.clear();
}
} // namespace RAMFlux::AI
