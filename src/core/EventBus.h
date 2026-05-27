// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <functional>
#include <map>
#include <mutex>
#include <vector>
#include <cstdint>
#include "shared/Constants.h"
namespace RAMFlux::Core {
using EventCallback = std::function<void()>;
    class EventBus {
public:
    static EventBus& instance();
uint64_t subscribe(Constants::EventType type, EventCallback callback);
void unsubscribe(Constants::EventType type, uint64_t id);
void post(Constants::EventType type);
    private:
    EventBus() = default;
    struct Subscriber {        uint64_t id;        EventCallback callback;    };
std::map<Constants::EventType, std::vector<Subscriber>> m_subscribers;
std::mutex m_mutex;
uint64_t m_nextId{
1};
};
} // namespace RAMFlux::Core


