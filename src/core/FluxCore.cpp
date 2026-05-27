// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "FluxCore.h"
#include "Logger.h"
namespace RAMFlux::Core {
FluxCore& FluxCore::instance() {    static FluxCore core;
    return core;
}
bool FluxCore::bootstrap() {    Logger::instance().info("[FluxCore] Bootstrapping RAMFlux v2.0.0");
bool ok = m_moduleManager.initializeAll();
    if(ok) {        Logger::instance().info("[FluxCore] All modules initialized successfully");    }
return ok;
}
void FluxCore::shutdown() {    Logger::instance().info("[FluxCore] Shutting down...");    m_moduleManager.shutdownAll();
    Logger::instance().info("[FluxCore] Shutdown complete");
}
ModuleManager& FluxCore::moduleManager() {
return m_moduleManager;
}
EventBus& FluxCore::eventBus() {
return EventBus::instance();
}} // namespace RAMFlux::Core


