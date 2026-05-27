// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "ModuleManager.h"
#include <algorithm>
#include "core/Logger.h"
namespace RAMFlux::Core {
void ModuleManager::registerModule(std::unique_ptr<IModule> module) {
if(module) {        m_modules.push_back(std::move(module));    }}
bool ModuleManager::initializeAll() {    bool allSuccess = true;
    for(const auto& mod : m_modules) {
if(!mod->initialize()) {            Logger::instance().error(std::string("Failed to initialize module: ") + mod->name());            allSuccess = false;        }    }
return allSuccess;
}
void ModuleManager::shutdownAll() {
for(auto it = m_modules.rbegin(); it != m_modules.rend(); ++it) {        (*it)->shutdown();    }}
IModule* ModuleManager::getModule(const std::string& name) const {    auto it = std::find_if(m_modules.begin(), m_modules.end(),        [&name](const auto& mod) {
return mod->name() == name; }
);
    return (it != m_modules.end()) ? it->get() : nullptr;
}} // namespace RAMFlux::Core


