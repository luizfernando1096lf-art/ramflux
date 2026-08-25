param(
    [string]$Version = "2.51.0",
    [string]$DeployDir = "build2/deploy",
    [string]$OutFile = ""
)
$ErrorActionPreference = "Stop"
if (-not $OutFile) { $OutFile = "RAMFlux-$Version-portable.zip" }
if (!(Test-Path $DeployDir)) { throw "Deploy dir not found: $DeployDir" }
# Ensure Qt deps are present (windeployqt already run in CI)
if (!(Test-Path "$DeployDir/libbrotlidec.dll")) {
    Write-Host "Running windeployqt6..."
    $env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH
    windeployqt6.exe "$DeployDir/RAMFlux.exe" --release
}
Write-Host "Packing $DeployDir -> $OutFile"
if (Test-Path $OutFile) { Remove-Item $OutFile -Force }
Compress-Archive -Path "$DeployDir/*" -DestinationPath $OutFile -CompressionLevel Optimal
$size = (Get-Item $OutFile).Length
Write-Host "Portable ZIP created: $OutFile ($([math]::Round($size/1MB,1)) MB)"
