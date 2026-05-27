// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "ProfileManager.h"
#include "core/EventBus.h"
#include "shared/Constants.h"
#include "core/Logger.h"
namespace RAMFlux::Profiles {
using RAMFlux::Core::Logger;
ProfileManager::ProfileManager() = default;
bool ProfileManager::initialize() {    Logger::instance().info("[ProfileManager] Initializing with Balanced profile");    m_initialized = true;
    return true;
}
void ProfileManager::shutdown() {    m_initialized = false;
    Logger::instance().info("[ProfileManager] Shutdown complete");
}
std::string ProfileManager::name() const {
return "ProfileManager";
}
Constants::ProfileType ProfileManager::activeProfile() const {
return m_activeProfile.load();
}
void ProfileManager::setProfile(Constants::ProfileType profile) {    m_activeProfile.store(profile);
    Logger::instance().info(std::string("[ProfileManager] Switched to profile: ")        + Constants::ProfileNames[static_cast<int>(profile)]);    Core::EventBus::instance().post(Constants::EventType::ProfileChanged);
std::lock_guard<std::mutex> lock(m_mutex);
    for(auto& cb : m_callbacks) {        cb(profile);    }}
ProfileConfig ProfileManager::configForProfile(Constants::ProfileType profile) const {    ProfileConfig cfg;
    switch(profile) {
case Constants::ProfileType::Economy:            cfg.cooldownMs = 60000;            cfg.autoCleanEnabled = true;            cfg.aggressiveTrim = false;            cfg.gameMode = false;            cfg.leakDetection = true;            cfg.pollingIntervalMs = 2000;            cfg.pressureThreshold = 60;            cfg.standbyThresholdMB = 2048;            cfg.freeMemThresholdMB = 1024;            cfg.name = "Economy";
    break;
case Constants::ProfileType::Balanced:            cfg.cooldownMs = 30000;            cfg.autoCleanEnabled = true;            cfg.aggressiveTrim = false;            cfg.gameMode = true;            cfg.leakDetection = true;            cfg.pollingIntervalMs = 1000;            cfg.pressureThreshold = 50;            cfg.standbyThresholdMB = 1024;            cfg.freeMemThresholdMB = 2048;            cfg.name = "Balanced";
    break;
case Constants::ProfileType::Performance:            cfg.cooldownMs = 15000;            cfg.autoCleanEnabled = true;            cfg.aggressiveTrim = true;            cfg.gameMode = false;            cfg.leakDetection = true;            cfg.pollingIntervalMs = 1000;            cfg.pressureThreshold = 35;            cfg.standbyThresholdMB = 512;            cfg.freeMemThresholdMB = 3072;            cfg.name = "Performance";
    break;
case Constants::ProfileType::Gaming:            cfg.cooldownMs = 120000;            cfg.autoCleanEnabled = false;            cfg.aggressiveTrim = false;            cfg.gameMode = true;            cfg.leakDetection = false;            cfg.pollingIntervalMs = 2000;            cfg.pressureThreshold = 75;            cfg.standbyThresholdMB = 2048;            cfg.freeMemThresholdMB = 4096;            cfg.name = "Gaming";
    break;
case Constants::ProfileType::Custom: {            std::lock_guard<std::mutex> lock(m_mutex);            cfg = m_customConfig;            cfg.name = "Custom";
    break; }
    }
return cfg;
}
ProfileConfig ProfileManager::currentConfig() const {
return configForProfile(m_activeProfile.load());
}
void ProfileManager::setCustomConfig(const ProfileConfig& cfg) {    std::lock_guard<std::mutex> lock(m_mutex);    m_customConfig = cfg;
}
ProfileConfig ProfileManager::customConfig() const {    std::lock_guard<std::mutex> lock(m_mutex);    return m_customConfig;
}
void ProfileManager::onProfileChanged(ProfileCallback cb) {    std::lock_guard<std::mutex> lock(m_mutex);    m_callbacks.push_back(std::move(cb));
}} // namespace RAMFlux::Profiles


