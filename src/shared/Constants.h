// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace RAMFlux::Constants {
inline constexpr const char* APP_NAME = "RAMFlux";
inline constexpr const char* APP_VERSION = "1.1.0";
inline constexpr const char* APP_AUTHOR = "RAMFlux Team";
inline constexpr int POLLING_INTERVAL_MS = 1000;
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
    enum class EventType {    MemoryUpdated,    PressureHigh,    PressureCritical,    CleaningStarted,    CleaningFinished,    GameDetected,    GameEnded,    LowPowerModeEntered,    LowPowerModeExited,    ProfileChanged,    LeakDetected,    LeakWarning,    ThresholdCleanTriggered};
enum class ProfileType {    Economy = 0,    Balanced,    Performance,    Gaming,    Custom};
inline constexpr const char* ProfileNames[] = {    "Economy",    "Balanced",    "Performance",    "Gaming",    "Custom"};
inline constexpr int PROFILE_COOLDOWN_ECONOMY_MS = 60000;
inline constexpr int PROFILE_COOLDOWN_BALANCED_MS = 30000;
inline constexpr int PROFILE_COOLDOWN_PERFORMANCE_MS = 15000;
inline constexpr int PROFILE_COOLDOWN_GAMING_MS = 120000;
inline constexpr const char* KNOWN_GAMES[] = {    "csgo.exe", "dota2.exe", "fortnite.exe", "valorant.exe",    "leagueoflegends.exe", "overwatch.exe", "rocketleague.exe",    "minecraft.exe", "gta5.exe", "rdr2.exe", "cyberpunk2077.exe",    "eldenring.exe", "warzone.exe", "apexlegends.exe", "destiny2.exe",    "pubg.exe", "rainbowsix.exe", "wow.exe", "ffxiv.exe", "lostark.exe",    "diablo4.exe", "starfield.exe", "baldursgate3.exe", "hogwartslegacy.exe",    "cod.exe", "battlefield.exe", "fifa.exe", "nba2k.exe", "madden.exe",    "starrail.exe", "genshinimpact.exe", "roblox.exe", "steam.exe",    "epicgameslauncher.exe", "xboxapp.exe"};
inline std::vector<std::string> knownGames() {
return std::vector<std::string>(std::begin(KNOWN_GAMES), std::end(KNOWN_GAMES));
}} // namespace RAMFlux::Constants


