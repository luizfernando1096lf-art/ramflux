// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <memory>
#include "ModuleManager.h"
#include "EventBus.h"
namespace RAMFlux::Core {
class FluxCore {
public:
    static FluxCore& instance();
bool bootstrap();
void shutdown();    ModuleManager& moduleManager();    EventBus& eventBus();
    private:
    FluxCore() = default;    ~FluxCore() = default;    FluxCore(const FluxCore&) = delete;    FluxCore& operator=(const FluxCore&) = delete;    ModuleManager m_moduleManager;
};
} // namespace RAMFlux::Core


