// SPDX-License-Identifier: MIT
// Copyright (C) 2026 RAMFlux Technologies
#pragma once
#include <cstdint>
#include <chrono>
#include <vector>
#include <string>
namespace RAMFlux::Telemetry {
struct ProcessMemoryBreakdown {    uint32_t pid{
0};
std::wstring name;
uint64_t workingSet{
0};
uint64_t privateUsage{
0};
uint64_t peakWorkingSet{
0};
uint64_t standbyMemory{
0};
    uint64_t pageTableUsage{
0};
uint64_t pageFileUsage{
0};
uint64_t pageFaults{
0};
uint32_t threadCount{
0};
double cpuPercent{
0.0};
};
struct FileCacheEntry {    std::wstring filePath;
std::wstring fileName;
uint64_t sizeBytes{
0};
uint64_t activeSize{
0};
uint64_t standbySize{
0};
uint64_t modifiedSize{
0};
};
struct MemorySnapshot {    uint64_t totalRam{
0};
uint64_t usedRam{
0};
uint64_t freeRam{
0};
uint64_t memoryLoad{
0};
uint64_t standbyMemory{
0};
uint64_t modifiedMemory{
0};
uint64_t cachedMemory{
0};
uint64_t committedMemory{
0};
uint64_t commitLimit{
0};
uint64_t compressedMemory{
0};
uint64_t compressionTotalData{
0};
uint64_t hardFaultsPerSec{
0};
uint64_t pressureScore{
0};
uint64_t totalVirtual{
0};
uint64_t usedVirtual{
0};
uint64_t totalPageFile{
0};
uint64_t usedPageFile{
0};
uint64_t kernelMemory{
0};
uint64_t kernelPaged{
0};
uint64_t kernelNonpaged{
0};
int processCount{
0};
double cpuUsage{
0.0};
double compressionRatio{
0.0};
double pageFaultTrend{
0.0};
std::vector<ProcessMemoryBreakdown> topProcesses;
std::vector<FileCacheEntry> topFileCache;
std::chrono::steady_clock::time_point timestamp;
double usedRamMB() const {
return static_cast<double>(usedRam) / (1024.0 * 1024.0); }    double totalRamMB() const {
return static_cast<double>(totalRam) / (1024.0 * 1024.0); }    double freeRamMB() const {
return static_cast<double>(freeRam) / (1024.0 * 1024.0); }    double usedRamGB() const {
return usedRamMB() / 1024.0; }    double totalRamGB() const {
return totalRamMB() / 1024.0; }    double freeRamGB() const {
return freeRamMB() / 1024.0; }    double standbyRamGB() const {
return static_cast<double>(standbyMemory) / (1024.0 * 1024 * 1024); }    double compressedRamMB() const {
return static_cast<double>(compressedMemory) / (1024.0 * 1024); }    double compressedRamGB() const {
return static_cast<double>(compressedMemory) / (1024.0 * 1024 * 1024); }    double compressionSavingsGB() const {
return (compressionTotalData > compressedMemory)            ? static_cast<double>(compressionTotalData - compressedMemory) / (1024.0 * 1024 * 1024)            : 0.0;    }    double compressionSavingsPercent() const {
return (compressionTotalData > 0)            ? (1.0 - static_cast<double>(compressedMemory) / compressionTotalData) * 100.0            : 0.0;    }    double usedPercent() const {
return totalRam > 0 ? (static_cast<double>(usedRam) / totalRam) * 100.0 : 0.0; }};
struct HistoryBuffer {    std::vector<MemorySnapshot> snapshots;
size_t maxSize{
3600};
void add(const MemorySnapshot& snap) {        snapshots.push_back(snap);
    while(snapshots.size() > maxSize) {            snapshots.erase(snapshots.begin());        }    }    void clear() { snapshots.clear(); }    size_t size() const {
return snapshots.size(); }    bool empty() const {
return snapshots.empty(); }    const MemorySnapshot& latest() const { static MemorySnapshot empty{};
return snapshots.empty() ? empty : snapshots.back(); }};
} // namespace RAMFlux::Telemetry 