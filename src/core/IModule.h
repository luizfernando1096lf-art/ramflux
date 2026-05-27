// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <string>
namespace RAMFlux::Core {
class IModule {
public:
    virtual ~IModule() = default;
virtual bool initialize() = 0;
virtual void shutdown() = 0;
virtual std::string name() const = 0;
};
} // namespace RAMFlux::Core


