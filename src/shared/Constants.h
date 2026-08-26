// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace RAMFlux::Constants {
inline constexpr const char* APP_NAME = "RAMFlux";
inline constexpr const char* APP_VERSION = "2.52.0";
inline constexpr const char* APP_AUTHOR = "RAMFlux Team";
inline constexpr int POLLING_INTERVAL_MS = 10000;
inline constexpr int UI_UPDATE_INTERVAL_MS = 1000;
inline constexpr int CHART_MAX_POINTS = 120;
inline constexpr int LOW_POWER_POLLING_INTERVAL_MS = 5000;
inline constexpr int HISTORY_MAX_SNAPSHOTS = 3600;
inline constexpr uint64_t BYTES_TO_MB = 1024 * 1024;
inline constexpr uint64_t BYTES_TO_GB = 1024 * 1024 * 1024;
    enum class PressureLevel {    Idle = 0,    Normal,    High,    Critical};
inline constexpr int PRESSURE_IDLE_MAX = 25;
inline constexpr int PRESSURE_NORMAL_MAX = 50;
inline constexpr int PRESSURE_HIGH_MAX = 75;
inline constexpr int PRESSURE_CRITICAL_MAX = 100;
inline constexpr int CLEANER_COOLDOWN_MS = 30000;
inline constexpr int OPTIMIZER_INTERVAL_MS = 5000;
inline constexpr int LEAK_CHECK_INTERVAL_MS = 10000;
inline constexpr int LEAK_HISTORY_SAMPLES = 30;
inline constexpr uint64_t DEFAULT_STANDBY_THRESHOLD_MB = 1024;
inline constexpr uint64_t DEFAULT_FREE_MEM_THRESHOLD_MB = 2048;
inline constexpr int DEFAULT_HARD_FAULT_THRESHOLD = 100;
inline constexpr uint64_t WS_AGE_ACTIVITY_THRESHOLD_BYTES = 5 * 1024 * 1024;
inline constexpr int WS_AGE_IDLE_SECONDS = 60;
inline constexpr double COMPRESSION_RATIO_HARMFUL = 1.2;
inline constexpr uint64_t DISK_QUEUE_LENGTH_HIGH = 2;
inline constexpr int PAGE_PRIORITY_VERY_LOW = 1;
inline constexpr int PAGE_PRIORITY_LOW = 2;
inline constexpr int PAGE_PRIORITY_MEDIUM = 3;
inline constexpr int PAGE_PRIORITY_BELOW_NORMAL = 4;
inline constexpr int PAGE_PRIORITY_NORMAL = 5;
    enum class WorkloadType {    Unknown = 0,    Gaming,    Mining,    Development,    Media,    HeavyComputation,    Office,    Idle};
inline constexpr const char* WorkloadNames[] = {    "Unknown",    "Gaming",    "Mining",    "Development",    "Media",    "HeavyComputation",    "Office",    "Idle"};
    enum class EventType {    MemoryUpdated,    PressureHigh,    PressureCritical,    CleaningStarted,    CleaningFinished,    GameDetected,    GameEnded,    MiningDetected,    MiningEnded,    LowPowerModeEntered,    LowPowerModeExited,    ProfileChanged,    LeakDetected,    LeakWarning,    ThresholdCleanTriggered,    WorkloadChanged,    AnomalyDetected,    PressurePredicted,    PressureChanged,    PressureDropped,    HardFaultStorm,    HardFaultStormCleared,    DiskQueueHigh,    DiskQueueNormalized,    BatteryLow,    BatteryNormalized,    PowerStateChanged};
inline constexpr const char* EventTypeNames[] = {    "MemoryUpdated",    "PressureHigh",    "PressureCritical",    "CleaningStarted",    "CleaningFinished",    "GameDetected",    "GameEnded",    "MiningDetected",    "MiningEnded",    "LowPowerModeEntered",    "LowPowerModeExited",    "ProfileChanged",    "LeakDetected",    "LeakWarning",    "ThresholdCleanTriggered",    "WorkloadChanged",    "AnomalyDetected",    "PressurePredicted", "PressureChanged", "PressureDropped", "HardFaultStorm",     "HardFaultStormCleared", "DiskQueueHigh", "DiskQueueNormalized", "BatteryLow", "BatteryNormalized", "PowerStateChanged"};
enum class ProfileType {    Economy = 0,    Balanced,    Performance,    Gaming,    Mining,    Custom};
inline constexpr const char* ProfileNames[] = {    "Economy",    "Balanced",    "Performance",    "Gaming",    "Mining",    "Custom"};
inline constexpr int PROFILE_COOLDOWN_ECONOMY_MS = 60000;
inline constexpr int PROFILE_COOLDOWN_BALANCED_MS = 30000;
inline constexpr int PROFILE_COOLDOWN_PERFORMANCE_MS = 15000;
inline constexpr int PROFILE_COOLDOWN_GAMING_MS = 120000;
inline constexpr int PROFILE_COOLDOWN_MINING_MS = 60000;
inline constexpr const char* KNOWN_GAMES[] = {    "csgo.exe", "dota2.exe", "fortnite.exe", "valorant.exe",    "leagueoflegends.exe", "overwatch.exe", "rocketleague.exe",    "minecraft.exe", "gta5.exe", "rdr2.exe", "cyberpunk2077.exe",    "eldenring.exe", "warzone.exe", "apexlegends.exe", "destiny2.exe",    "pubg.exe", "rainbowsix.exe", "wow.exe", "ffxiv.exe", "lostark.exe",    "diablo4.exe", "starfield.exe", "baldursgate3.exe", "hogwartslegacy.exe",    "cod.exe", "battlefield.exe", "fifa.exe", "nba2k.exe", "madden.exe",    "starrail.exe", "genshinimpact.exe", "roblox.exe", "steam.exe",    "epicgameslauncher.exe", "xboxapp.exe"};
inline constexpr const char* KNOWN_DEVELOPMENT[] = {    "devenv.exe", "code.exe", "clion64.exe", "pycharm64.exe",    "idea64.exe", "clangd.exe", "gcc.exe", "csc.exe",    "msbuild.exe", "cmake.exe", "ninja.exe", "git.exe",    "node.exe", "npm.exe", "dotnet.exe", "rustc.exe"};
inline constexpr const char* KNOWN_MEDIA[] = {    "vlc.exe", "mpv.exe", "potplayer.exe", "wmplayer.exe",    "spotify.exe", "chrome.exe", "firefox.exe", "msedge.exe"};
inline constexpr const char* KNOWN_HEAVY[] = {    "vmware.exe", "virtualbox.exe", "docker.exe", "mstsc.exe",    "blender.exe", "maya.exe", "3dsmax.exe", "x64dbg.exe",    "ida.exe", "windbg.exe"};
inline std::vector<std::string> knownGames() {
return std::vector<std::string>(std::begin(KNOWN_GAMES), std::end(KNOWN_GAMES));
}
inline std::vector<std::string> knownDevelopment() {
return std::vector<std::string>(std::begin(KNOWN_DEVELOPMENT), std::end(KNOWN_DEVELOPMENT));
}
inline std::vector<std::string> knownMedia() {
return std::vector<std::string>(std::begin(KNOWN_MEDIA), std::end(KNOWN_MEDIA));
}
inline constexpr const char* KNOWN_MINERS[] = {    "xmrig.exe", "xmrig-mo.exe", "xmrig-cuda.exe",    "ccminer.exe", "ccminer-x64.exe",    "nbminer.exe", "nicehashminer.exe", "nheqminer.exe",    "t-rex.exe", "trex.exe",    "lolminer.exe", "lolMiner.exe",    "phoenixminer.exe", "phoenix.exe",    "teamredminer.exe", "trm.exe",    "gminer.exe", "miniZ.exe", "miniz.exe",    "ethminer.exe", "sgminer.exe", "bfgminer.exe",    "cpuminer.exe", "cpuminer-multi.exe", "cpuminer-avx2.exe",    "cpuminer-zen3.exe", "cpuminer-avx.exe",    "cpuminer-avx512.exe", "cpuminer-avx2-sha.exe",    "cpuminer-sse2.exe", "cpuminer-opt.exe",    "cpu-miner.exe",    "minerd.exe", "pooler.exe",    "srbminer.exe", "srbminer-multi.exe",    "wildrig.exe", "wildrig-multi.exe",    "nanominer.exe", "rigel.exe", "bzminer.exe",    "sugarchain-miner.exe", "sugarmaker.exe",    "sha256csm.exe", "sha256-miner.exe",    "cpu-q.exe"};
inline std::vector<std::string> knownMiners() {
return std::vector<std::string>(std::begin(KNOWN_MINERS), std::end(KNOWN_MINERS));
}
inline std::vector<std::string> knownHeavyCompute() {
return std::vector<std::string>(std::begin(KNOWN_HEAVY), std::end(KNOWN_HEAVY));
}
inline constexpr int BATTERY_CHECK_INTERVAL_MS = 5000;
inline constexpr int BATTERY_LOW_THRESHOLD = 20;
inline constexpr int BATTERY_CRITICAL_THRESHOLD = 10;
inline constexpr int AI_PREDICTION_HORIZON_SEC = 120;
inline constexpr int AI_ANOMALY_WINDOW_SIZE = 30;
inline constexpr int AI_ANOMALY_STDDEV_THRESHOLD = 3;
inline constexpr double AI_WORKLOAD_STABILITY_SEC = 30.0;
inline constexpr double AI_PREDICTION_CONFIDENCE_MIN = 0.6;
inline constexpr int PREDICTIVE_CLEAN_CHECK_INTERVAL_MS = 10000;
inline constexpr int PREDICTIVE_CLEAN_MIN_RUN_INTERVAL_MS = 120000;
inline constexpr double PREDICTIVE_CLEAN_LOOKAHEAD_HIGH_MS = 30000.0;
inline constexpr double PREDICTIVE_CLEAN_MIN_STANDBY_GB = 0.5;
// NUMA awareness constants
inline constexpr double NUMA_IMBALANCE_THRESHOLD = 0.20;
inline constexpr int NUMA_NODE_MIN_AVAILABLE_MB = 512;
// WS aging constants
inline constexpr int WS_AGE_TRIM_THRESHOLD_SECONDS = 60;
inline constexpr uint64_t WS_AGE_TRIM_THRESHOLD_BYTES = 100 * 1024 * 1024;
// Compression tuning constants
inline constexpr int COMPRESSION_CONSECUTIVE_HARMFUL_LIMIT = 3;
inline constexpr double COMPRESSION_EFFICIENCY_MIN = 0.15;
// Hard fault prediction constants
inline constexpr double HF_PREDICTION_WARNING_THRESHOLD = 50.0;
// Game Mode 3.0 constants
inline constexpr double GM_VRAM_PRESSURE_HIGH = 75.0;
inline constexpr double GM_VRAM_PRESSURE_CRITICAL = 90.0;
inline constexpr double GM_SYSTEM_PRESSURE_THROTTLE = 75.0;
inline constexpr uint64_t GM_PREGAME_STANDBY_CLEAN_THRESHOLD = 512ULL * 1024 * 1024;
inline constexpr uint64_t GM_PREGAME_PROCESS_TRIM_THRESHOLD = 200ULL * 1024 * 1024;
inline constexpr int GM_PREGAME_IDLE_SECONDS = 30;
inline constexpr int GM_VRAM_MONITOR_INTERVAL_MS = 5000;
inline constexpr double GM_VRAM_BUDGET_SAFETY_MARGIN = 0.85;
inline constexpr int GM_TIMER_RESOLUTION_MS = 1;
inline constexpr int GM_BG_PAGE_VERYLOW_THRESHOLD_BYTES = 200 * 1024 * 1024;
inline constexpr int GM_BG_PAGE_LOW_THRESHOLD_BYTES = 100 * 1024 * 1024;
inline constexpr int GM_BG_PROCESS_THRESHOLD_BYTES = 50 * 1024 * 1024;
// Mining Mode constants
inline constexpr int MM_DETECTION_INTERVAL_MS = 2000;
inline constexpr int MM_BG_PAGE_VERYLOW_THRESHOLD_BYTES = 200 * 1024 * 1024;
inline constexpr int MM_BG_PAGE_LOW_THRESHOLD_BYTES = 100 * 1024 * 1024;
inline constexpr int MM_BG_PROCESS_THRESHOLD_BYTES = 50 * 1024 * 1024;
inline constexpr int MM_MINER_CPU_PRIORITY = 0x80; // HIGH_PRIORITY_CLASS
inline constexpr int MM_MINER_IO_PRIORITY = 2;
inline constexpr int MM_MINER_PAGE_PRIORITY = 5;
inline constexpr int MM_TIMER_RESOLUTION_MS = 1;
// Page File Auto-Tuning constants
inline constexpr int PF_TUNING_INTERVAL_MS = 120000;
inline constexpr double PF_RESIZE_PRESSURE_THRESHOLD = 90.0;
inline constexpr double PF_CLEAN_PRESSURE_THRESHOLD = 80.0;
inline constexpr uint64_t PF_RECOMMENDED_MIN_BASE_MB = 4096;
inline constexpr uint64_t PF_RECOMMENDED_MIN_RATIO = 4; // 1/4 of RAM
inline constexpr uint64_t PF_RECOMMENDED_MAX_RATIO = 3; // 3x RAM
inline constexpr uint64_t PF_CRITICAL_FREE_MB = 512;
inline constexpr uint64_t PF_WARN_FREE_MB = 1024;
inline constexpr uint64_t PF_HEADROOM_RATIO = 2; // /2 of RAM min headroom
inline constexpr double PF_PRESSURE_HIGH = 60.0;
// Cache Pressure constants
inline constexpr double CP_CACHE_SCORE_WEIGHT = 0.05;
inline constexpr double CP_L3_CONTENDED_BONUS = 10.0;
inline constexpr double CP_L3_CONTENDED_RATIO = 1.5;
inline constexpr double CP_L3_WEIGHT = 0.5;
inline constexpr double CP_L2_WEIGHT = 0.3;
inline constexpr double CP_L1_WEIGHT = 0.2;
// Advanced Compression constants
inline constexpr double AC_POOL_LOW_HIT_RATE = 0.3;
inline constexpr double AC_POOL_LOW_HIT_PRESSURE = 5.0;
inline constexpr double AC_HARMFUL_RATIO_THRESHOLD = 1.0;
inline constexpr double AC_HEALTHY_RATIO_THRESHOLD = 2.0;
inline constexpr double AC_GAME_HIT_RATE_THRESHOLD = 0.5;
inline constexpr uint32_t AC_DECODER_POOL_EXPAND_STEP = 256;
inline constexpr int AC_TUNING_INTERVAL_MS = 60000;
inline constexpr double AC_POOL_USAGE_HIGH = 80.0;
// Process Memory Classifier constants
inline constexpr uint64_t CL_MIN_WS_TRACK_BYTES = 10ULL * 1024 * 1024;
inline constexpr size_t CL_MAX_SAMPLES_PER_PROCESS = 60;
inline constexpr double CL_LEAKY_GROWTH_THRESHOLD_MBPM = 50.0;
inline constexpr double CL_BURST_PEAK_MEAN_RATIO = 2.0;
inline constexpr int CL_PERIODIC_CYCLES_REQUIRED = 3;
inline constexpr uint64_t CL_TRIM_THRESHOLD_BYTES = 100ULL * 1024 * 1024;
inline constexpr int CL_CLASSIFICATION_INTERVAL_MS = 30000;
inline constexpr double CL_STEADY_CV_THRESHOLD = 0.15;
inline constexpr double CL_STEADY_PEAK_THRESHOLD = 1.3;
inline constexpr double CL_BURST_CV_THRESHOLD = 0.5;
inline constexpr double CL_PERIODIC_CV_MIN = 0.2;
inline constexpr double CL_PERIODIC_CV_MAX = 0.8;
inline constexpr double CL_LEAKY_MIN_SAMPLES = 20;
inline constexpr uint64_t CL_TRIM_WS_MIN_BYTES = 512ULL * 1024 * 1024;
// I/O-aware memory cleaning — protect disk from thrashing caused by aggressive cache cleanup
// The standby list IS Windows' disk cache; flushing it forces every file read to hit physical disk.
// These thresholds ensure cleaning only happens when it won't cause more harm than good.
inline constexpr double IO_DISK_QUEUE_SKIP_THRESHOLD = 1.5;   // skip cleaning if disk queue exceeds this (1.5 = moderate queue)
inline constexpr uint64_t IO_STANDBY_MIN_CLEAN_BYTES = 2048ULL * 1024 * 1024; // only clean standby if >= 2GB (prevents thrashing for small caches)
inline constexpr uint64_t IO_MODIFIED_MIN_CLEAN_BYTES = 1024ULL * 1024 * 1024; // only clean modified list if >= 1GB
inline constexpr int IO_TRIM_IDLE_SECONDS = 120;               // more conservative idle time before trimming WS
inline constexpr int IO_FILE_CACHE_HIGH_FAULT_THRESHOLD = 150; // only flush file cache if hard faults exceed this
inline constexpr int IO_CLEAN_COOLDOWN_AFTER_WRITE_MS = 45000; // extra cooldown after any write-heavy cleaning op
// Intelligent Standby List constants
inline constexpr uint32_t SL_STANDBY_PRIORITY_LOWEST = 1;
inline constexpr uint32_t SL_STANDBY_PRIORITY_BELOW_NORMAL = 2;
inline constexpr uint32_t SL_STANDBY_PRIORITY_NORMAL = 5;
inline constexpr uint32_t SL_STANDBY_PRIORITY_FOREGROUND = 6;
inline constexpr uint64_t SL_FOREGROUND_WS_THRESHOLD = 500ULL * 1024 * 1024;
inline constexpr int SL_ORCHESTRATION_INTERVAL_MS = 30000;
// System File Cache Tuner constants
inline constexpr uint64_t FC_GAME_CACHE_LIMIT_BYTES = 128ULL * 1024 * 1024;
inline constexpr uint64_t FC_MINING_CACHE_LIMIT_BYTES = 64ULL * 1024 * 1024;
inline constexpr uint64_t FC_DEFAULT_CACHE_LIMIT_BYTES = 512ULL * 1024 * 1024;
inline constexpr uint64_t FC_MIN_CACHE_BYTES = 16ULL * 1024 * 1024;
inline constexpr int FC_TUNE_INTERVAL_MS = 60000;
} // namespace RAMFlux::Constants
