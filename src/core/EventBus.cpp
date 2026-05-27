// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#include "EventBus.h"
namespace RAMFlux::Core {
EventBus& EventBus::instance() {    static EventBus instance;
    return instance;
}
uint64_t EventBus::subscribe(Constants::EventType type, EventCallback callback) {    std::lock_guard<std::mutex> lock(m_mutex);
uint64_t id = m_nextId++;    m_subscribers[type].push_back({
id, std::move(callback)}
);
    return id;
}
void EventBus::unsubscribe(Constants::EventType type, uint64_t id) {    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_subscribers.find(type);
    if(it != m_subscribers.end()) {        auto& vec = it->second;        vec.erase(std::remove_if(vec.begin(), vec.end(),            [id](const Subscriber& s) {
return s.id == id; }
), vec.end());    }}
void EventBus::post(Constants::EventType type) {    std::vector<EventCallback> callbacks;    {        std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_subscribers.find(type);
    if(it != m_subscribers.end()) {
for(const auto& sub : it->second) {                callbacks.push_back(sub.callback);            }        }    }    for (const auto& cb : callbacks) {        cb();    }}} // namespace RAMFlux::Core


