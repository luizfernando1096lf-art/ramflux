# Versioning Reference

All version strings must be updated together on each release.

## Files to Update

| File | Field | Example |
|------|-------|---------|
   | `src/shared/Constants.h:9` | `APP_VERSION` | `"2.44.1"` |
   | `CMakeLists.txt:2` | `project(VERSION ...)` | `2.44.1` |
   | `installer.wxs:4` | `Package Version` | `"2.44.1"` |
   | `resources/app.rc:4-5` | `FILEVERSION` / `PRODUCTVERSION` | `2,44,1,0` |
   | `resources/app.rc:18,23` | `FileVersion` / `ProductVersion` | `"2.44.1.0"` |
   | `resources/helper.rc:4-5` | `FILEVERSION` / `PRODUCTVERSION` | `2,44,1,0` |
   | `resources/helper.rc:18,23` | `FileVersion` / `ProductVersion` | `"2.44.1.0"` |
   | `resources/app.manifest:6` | `assemblyIdentity version` | `"2.44.1.0"` |
   | `resources/helper.manifest:6` | `assemblyIdentity version` | `"2.44.1.0"` |
   | `resources/manual_pt.txt:4` | Version text | `Versao 2.44.1` |
   | `resources/manual_en.txt:4` | Version text | `Version 2.44.1` |
...
- `FILEVERSION`/`PRODUCTVERSION` string uses dots: `"2.44.1.0"`
- `APP_VERSION` uses dots: `"2.44.1"`
- Manifest version uses dots: `"2.44.1.0"`

## Critical: MSI Source Directory
The `installer.wxs` references files from `build/deploy/`. After rebuilding with `cmake --build`, always run:
```
copy build\RAMFlux.exe build\deploy\RAMFlux.exe
copy build\RAMFluxHelper.exe build\deploy\RAMFluxHelper.exe
```
before generating the MSI, or the installer will package stale binaries.
Note: this project deploys to `build2/deploy/` (which contains the Qt plugin/asset subdirectories); copy the same two binaries there:
```
copy build\RAMFlux.exe build2\deploy\RAMFlux.exe
copy build\RAMFluxHelper.exe build2\deploy\RAMFluxHelper.exe
```
