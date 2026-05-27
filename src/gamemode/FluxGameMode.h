// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <windows.h>
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "core/IModule.h"
namespace RAMFlux::GameMode {
class FluxGameMode : public Core::IModule {
public:
    FluxGameMode();    ~FluxGameMode() override;
bool initialize() override;
void shutdown() override;
std::string name() const override;
bool isGameRunning() const;
std::wstring currentGameName() const;
void setGameModeEnabled(bool enabled);
bool isGameModeEnabled() const;
    private:
    void detectionLoop();
bool detectFullscreenGame();
bool detectKnownGame();
std::wstring getProcessName(DWORD pid) const;
std::thread m_thread;
std::atomic<bool> m_running{
false};
std::atomic<bool> m_gameDetected{
false};
std::atomic<bool> m_gameModeEnabled{
true};
mutable std::mutex m_gameNameMutex;
std::wstring m_currentGameName;
};
} // namespace RAMFlux::GameMode


