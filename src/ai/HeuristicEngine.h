// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>
#include <deque>
#include "core/IModule.h"
#include "shared/Constants.h"
#include "telemetry/MemorySnapshot.h"
#include "WorkloadClassifier.h"
#include "PressurePredictor.h"
#include "MLEngine.h"
#include "IoCostTracker.h"
#include "StandbyScanner.h"
#include "PagePrefetcher.h"
#include "qos/MemoryQoS.h"
#include "dedup/MemoryDedup.h"
namespace RAMFlux::AI {
struct EffectivenessSample {
    std::chrono::steady_clock::time_point madeAt;
    int horizonSeconds{30};
    double predictedPressure{0.0};
    double actualPressure{0.0};
    double errorPercent{0.0};
    bool evaluated{false};
    bool wasCorrect{false};
};
struct EffectivenessMetrics {
    double accuracy{0.0};
    double meanError{0.0};
    double recentAccuracy{0.0};
    uint64_t totalPredictions{0};
    uint64_t correctPredictions{0};
    uint64_t falsePositives{0};
    uint64_t falseNegatives{0};
};
struct TuningParams {
    int recommendedCooldownMs{30000};
    int recommendedIntervalMs{5000};
    double recommendedConfidenceThreshold{0.6};
    double aggressiveFactor{1.0};
};
enum class HardFaultSeverity {
    None = 0,
    Low,
    Medium,
    High,
    Critical
};
struct HardFaultPrediction {
    HardFaultSeverity severity{HardFaultSeverity::None};
    double severityScore{0.0};
    double hardFaultTrendSlope{0.0};
    double diskQueueTrendSlope{0.0};
    double standbyTrendSlope{0.0};
    double predictedFaultsIn30s{0.0};
    double confidence{0.0};
    std::string triggerReason;
    bool stormWarning{false};
};
struct HeuristicReport {
    Constants::WorkloadType workload{Constants::WorkloadType::Unknown};
    double workloadConfidence{0.0};
    std::string workloadTrigger;
    double predictedPressure30s{0.0};
    double predictedPressure60s{0.0};
    double predictedPressure120s{0.0};
    double predictionConfidence{0.0};
    double trendSlope{0.0};
    bool anomalyDetected{false};
    double anomalyDeviation{0.0};
    uint64_t totalSamples{0};
    Constants::PressureLevel currentPressureLevel{Constants::PressureLevel::Idle};
    EffectivenessMetrics effectiveness;
    TuningParams tuning;
    HardFaultPrediction hardFault;
    double mlScore{0.0};
    double mlConfidence{0.0};
    int mlSampleCount{0};
    bool mlEnabled{false};
    IoCostReport ioCost;
    StandbyReport standby;
    PrefetchReport prefetch;
    QoS::QoSEnforcement qos;
    Dedup::DedupReport dedup;
};
class HeuristicEngine : public Core::IModule {
public:
    HeuristicEngine();
    ~HeuristicEngine() override;
    bool initialize() override;
    void shutdown() override;
    std::string name() const override;
    void feedSnapshot(const Telemetry::MemorySnapshot& snap);
    HeuristicReport currentReport() const;
    Dedup::MemoryDedup& dedup() { return m_dedup; }
    QoS::MemoryQoS& qos() { return m_qos; }
    EffectivenessMetrics currentEffectiveness() const;
    TuningParams currentTuning() const;
    void balanceNumaIfNeeded(const Telemetry::MemorySnapshot& snap);
private:
    void analysisLoop();
    void evaluateAndPost(const Telemetry::MemorySnapshot& snap);
    void evaluatePredictionAccuracy(const Telemetry::MemorySnapshot& snap);
    void storePrediction(double predictedPressure, int horizonSeconds);
    void adjustModuleParams();
    void tuneFromMetrics();
    std::thread m_thread;
    std::atomic<bool> m_running{false};
    std::atomic<int> m_intervalMs{2000};
    mutable std::mutex m_mutex;
    WorkloadClassifier m_classifier;
    PressurePredictor m_predictor;
    MLEngine m_mlEngine;
    IoCostTracker m_ioCostTracker;
    StandbyScanner m_standbyScanner;
    PagePrefetcher m_pagePrefetcher;
    QoS::MemoryQoS m_qos;
    Dedup::MemoryDedup m_dedup;
    HeuristicReport m_latestReport;
    Constants::PressureLevel m_lastPressureLevel{Constants::PressureLevel::Idle};
    bool m_lastStormWarning{false};
    Constants::WorkloadType m_lastPostedWorkload{Constants::WorkloadType::Unknown};
    std::chrono::steady_clock::time_point m_lastAnomalyPost;
    std::deque<EffectivenessSample> m_predictionHistory;
    static constexpr size_t MAX_PREDICTION_HISTORY{50};
    static constexpr size_t ACCURACY_WINDOW{20};
    EffectivenessMetrics m_effectiveness;
    TuningParams m_tuningParams;
    std::chrono::steady_clock::time_point m_lastTuneTime;
    std::chrono::steady_clock::time_point m_lastNumaBalance{};
    std::atomic<int64_t> m_lastCleanTimeEpoch{0};
    uint64_t m_cleanStartedSubId{0};
    uint64_t m_cleanFinishedSubId{0};
    int m_tuneIntervalMs{30000};
    struct HardFaultSample {
        std::chrono::steady_clock::time_point time;
        double hardFaultsPerSec{0.0};
        double diskQueueLength{0.0};
        double standbyGB{0.0};
    };
    class HardFaultPredictor {
    public:
        void feed(const Telemetry::MemorySnapshot& snap);
        HardFaultPrediction evaluate();
        void reset();
    private:
        std::deque<HardFaultSample> m_samples;
        std::chrono::steady_clock::time_point m_lastWarning;
        static constexpr size_t MAX_SAMPLES{60};
        double computeSlope(const std::deque<HardFaultSample>& samples,
            double HardFaultSample::*field) const;
    } m_hardFault;
};
} // namespace RAMFlux::AI
