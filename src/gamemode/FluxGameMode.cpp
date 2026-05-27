// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "FluxGameMode.h"
#include "core/EventBus.h"
#include "shared/Constants.h"
#include <windows.h>
#include <tlhelp32.h>
#include "core/Logger.h"
#include <chrono>
#include <thread>
#include <algorithm>
#include <exception>
namespace RAMFlux::GameMode {
using RAMFlux::Core::Logger;
FluxGameMode::FluxGameMode() = default;
FluxGameMode::~FluxGameMode() {    shutdown();
}
bool FluxGameMode::initialize() {    Logger::instance().info("[FluxGameMode] Initializing...");    m_running = true;    m_thread = std::thread(&FluxGameMode::detectionLoop, this);
    Logger::instance().info("[FluxGameMode] Detection thread started");
    return true;
}
void FluxGameMode::shutdown() {
if(m_running.exchange(false)) {
if(m_thread.joinable()) {            m_thread.join();        }        Logger::instance().info("[FluxGameMode] Shutdown complete");    }}
std::string FluxGameMode::name() const {
return "FluxGameMode";
}
std::wstring FluxGameMode::getProcessName(DWORD pid) const {    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if(!hProcess)
return L"";    wchar_t name[MAX_PATH];
DWORD size = MAX_PATH;
std::wstring result;
    if(QueryFullProcessImageNameW(hProcess, 0, name, &size)) {        std::wstring fullPath(name);
    auto pos = fullPath.find_last_of(L"\\/");
    if(pos != std::wstring::npos) {            result = fullPath.substr(pos + 1);        } else {            result = fullPath;        }    }    CloseHandle(hProcess);
    return result;
}
bool FluxGameMode::detectKnownGame() {    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnapshot == INVALID_HANDLE_VALUE)
return false;
    auto knownGames = Constants::knownGames();
bool found = false;
std::wstring foundName;    PROCESSENTRY32W pe32;    pe32.dwSize = sizeof(pe32);
    if(Process32FirstW(hSnapshot, &pe32)) {
do {            std::wstring exeName(pe32.szExeFile);
std::transform(exeName.begin(), exeName.end(), exeName.begin(), ::towlower);
    for(const auto& game : knownGames) {                std::wstring wgame(game.begin(), game.end());
    if(exeName == wgame) {                    found = true;                    foundName = exeName;
    break;                }            }            if (found) break;        } while (Process32NextW(hSnapshot, &pe32));    }    CloseHandle(hSnapshot);
    if(found) {        std::lock_guard<std::mutex> lock(m_gameNameMutex);        m_currentGameName = foundName;    }
return found;
}
bool FluxGameMode::detectFullscreenGame() {    HWND foreground = GetForegroundWindow();
    if(!foreground)
return false;
DWORD pid;    GetWindowThreadProcessId(foreground, &pid);
    if(pid == GetCurrentProcessId())
return false;    RECT windowRect;
    if(!GetWindowRect(foreground, &windowRect))
return false;
int width = windowRect.right - windowRect.left;
int height = windowRect.bottom - windowRect.top;
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
bool isFullscreen = (width == screenWidth && height == screenHeight);
    if(isFullscreen) {        wchar_t className[256];        GetClassNameW(foreground, className, 256);
std::wstring cls(className);
    if(cls.find(L"Windows.UI.Core.CoreWindow") != std::wstring::npos ||            cls.find(L"Shell_TrayWnd") != std::wstring::npos) {
return false;        }        auto fgName = getProcessName(pid);
    if(!fgName.empty()) {            std::lock_guard<std::mutex> lock(m_gameNameMutex);            m_currentGameName = fgName;        }    }
return isFullscreen;
}
void FluxGameMode::detectionLoop() {
while(m_running) {
try {
if(m_gameModeEnabled) {                bool detected = detectKnownGame() || detectFullscreenGame();
bool previous = m_gameDetected.exchange(detected);
    if(detected && !previous) {                    {                        std::lock_guard<std::mutex> lock(m_gameNameMutex);
    Logger::instance().info(std::string("[FluxGameMode] Game detected: ") + std::string(m_currentGameName.begin(), m_currentGameName.end()));                    }                    Logger::instance().info("[FluxGameMode] Activating Gaming profile");                    Core::EventBus::instance().post(Constants::EventType::GameDetected);                } else
if(!detected && previous) {                    Logger::instance().info("[FluxGameMode] Game ended, restoring normal mode");                    {                        std::lock_guard<std::mutex> lock(m_gameNameMutex);                        m_currentGameName.clear();                    }                    Core::EventBus::instance().post(Constants::EventType::GameEnded);                }            }        } catch (const std::exception& e) {            Logger::instance().error(std::string("[FluxGameMode] Error: ") + e.what());        } catch (...) {            Logger::instance().error("[FluxGameMode] Unknown error");        }        std::this_thread::sleep_for(std::chrono::seconds(2));    }}
bool FluxGameMode::isGameRunning() const {
return m_gameDetected.load();
}
std::wstring FluxGameMode::currentGameName() const {    std::lock_guard<std::mutex> lock(m_gameNameMutex);
    return m_currentGameName;
}
void FluxGameMode::setGameModeEnabled(bool enabled) {    m_gameModeEnabled.store(enabled);
}
bool FluxGameMode::isGameModeEnabled() const {
return m_gameModeEnabled.load();
}} // namespace RAMFlux::GameMode


