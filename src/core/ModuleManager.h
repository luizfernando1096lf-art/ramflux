// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <memory>
#include <vector>
#include "IModule.h"
namespace RAMFlux::Core {
class ModuleManager {
public:
    void registerModule(std::unique_ptr<IModule> module);
bool initializeAll();
void shutdownAll();    IModule* getModule(const std::string& name) const;
    private:
    std::vector<std::unique_ptr<IModule>> m_modules;
};
} // namespace RAMFlux::Core


