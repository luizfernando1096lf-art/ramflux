# Versioning Reference

All version strings must be updated together on each release.

## Files to Update

| File | Field | Example |
|------|-------|---------|
   | `src/shared/Constants.h:9` | `APP_VERSION` | `"2.44.0"` |
   | `CMakeLists.txt:2` | `project(VERSION ...)` | `2.44.0` |
   | `installer.wxs:4` | `Package Version` | `"2.44.0"` |
   | `resources/app.rc:4-5` | `FILEVERSION` / `PRODUCTVERSION` | `2,44,0,0` |
   | `resources/app.rc:18,23` | `FileVersion` / `ProductVersion` | `"2.44.0.0"` |
   | `resources/helper.rc:4-5` | `FILEVERSION` / `PRODUCTVERSION` | `2,44,0,0` |
   | `resources/helper.rc:18,23` | `FileVersion` / `ProductVersion` | `"2.44.0.0"` |
   | `resources/app.manifest:6` | `assemblyIdentity version` | `"2.44.0.0"` |
   | `resources/helper.manifest:6` | `assemblyIdentity version` | `"2.44.0.0"` |
   | `resources/manual_pt.txt:4` | Version text | `Versao 2.44.0` |
   | `resources/manual_en.txt:4` | Version text | `Version 2.44.0` |
...
- `FILEVERSION`/`PRODUCTVERSION` string uses dots: `"2.44.0.0"`
- `APP_VERSION` uses dots: `"2.44.0"`
- Manifest version uses dots: `"2.44.0.0"`

## Critical: MSI Source Directory
The `installer.wxs` references files from `build/deploy/` (not `build/`). After rebuilding with `cmake --build`, always run:
```
copy build\RAMFlux.exe build\deploy\RAMFlux.exe
copy build\RAMFluxHelper.exe build\deploy\RAMFluxHelper.exe
```
before generating the MSI, or the installer will package stale binaries.
