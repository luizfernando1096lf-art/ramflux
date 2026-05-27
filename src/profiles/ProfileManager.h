// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <atomic>
#include <functional>
#include <mutex>
#include "core/IModule.h"
#include "shared/Constants.h"
namespace RAMFlux::Profiles {
struct ProfileConfig {    int cooldownMs{
30000};
bool autoCleanEnabled{
true};
bool aggressiveTrim{
false};
bool gameMode{
true};
bool leakDetection{
true};
int pollingIntervalMs{
1000};
int pressureThreshold{
50};
uint64_t standbyThresholdMB{
1024};
uint64_t freeMemThresholdMB{
2048};
std::string name{
"Balanced"};
};
class ProfileManager : public Core::IModule {
public:
    ProfileManager();    ~ProfileManager() override = default;
bool initialize() override;
void shutdown() override;
std::string name() const override;    Constants::ProfileType activeProfile() const;
void setProfile(Constants::ProfileType profile);    ProfileConfig configForProfile(Constants::ProfileType profile) const;    ProfileConfig currentConfig() const;
    using ProfileCallback = std::function<void(Constants::ProfileType)>;
    void onProfileChanged(ProfileCallback cb);
    void setCustomConfig(const ProfileConfig& cfg);
    ProfileConfig customConfig() const;
    private:
    std::atomic<Constants::ProfileType> m_activeProfile{
Constants::ProfileType::Balanced};
std::vector<ProfileCallback> m_callbacks;    mutable std::mutex m_mutex;
ProfileConfig m_customConfig;
bool m_initialized{
false};
};
} // namespace RAMFlux::Profiles


