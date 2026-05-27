# ==============================================================================
# RAMFlux - Build Script (PowerShell)
# ==============================================================================

param(
    [switch]$Clean,
    [switch]$Quiet
)

Write-Host "=============================================================================" -ForegroundColor Cyan
Write-Host "RAMFlux - Build Script (PowerShell)" -ForegroundColor Cyan
Write-Host "=============================================================================" -ForegroundColor Cyan
Write-Host ""

# Remover build anterior
if ($Clean) {
    Write-Host "[CLEAN] Removendo pasta build anterior..." -ForegroundColor Yellow
    if (Test-Path "build") {
        Remove-Item -Path "build" -Recurse -Force
    }
    Write-Host ""
}

# Criar pasta build
if (-not (Test-Path "build")) {
    Write-Host "[BUILD] Criando pasta build..." -ForegroundColor Green
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Navegar para pasta build
Set-Location -Path "build"

Write-Host "[CMAKE] Configurando CMake..." -ForegroundColor Cyan
Write-Host ""

# Configurar CMake
$cmakeArgs = "-B.."
& cmake $cmakeArgs -DCMAKE_BUILD_TYPE=Release -G"Visual Studio 17 2022" -A x64
$cmakeExitCode = $LASTEXITCODE

if ($cmakeExitCode -ne 0) {
    Write-Host "[CMAKE] Erro na configuracao do CMake (Exit Code: $cmakeExitCode)" -ForegroundColor Red
    Write-Host "Certifique-se de ter o CMake instalado: https://cmake.org/download/" -ForegroundColor Red
    Set-Location -Path ".."
    exit $cmakeExitCode
}

Write-Host ""
Write-Host "[BUILD] Compilando o projeto..." -ForegroundColor Cyan
Write-Host ""

# Compilar
& cmake --build . --config Release -- /p:PlatformToolset=v143
$buildExitCode = $LASTEXITCODE

if ($buildExitCode -ne 0) {
    Write-Host "[BUILD] Erro na compilacao (Exit Code: $buildExitCode)" -ForegroundColor Red
    Set-Location -Path ".."
    exit $buildExitCode
}

Write-Host ""
Write-Host "=============================================================================" -ForegroundColor Green
Write-Host "Build concluido com sucesso!" -ForegroundColor Green
Write-Host "Executavel: $PSScriptRoot\Release\RAMFlux.exe" -ForegroundColor Green
Write-Host "=============================================================================" -ForegroundColor Green

# Copiar para pasta release
Write-Host ""
Write-Host "[COPY] Copiando executavel para pasta release..." -ForegroundColor Cyan
if (Test-Path "$PSScriptRoot\Release\RAMFlux.exe") {
    Copy-Item -Path "$PSScriptRoot\Release\RAMFlux.exe" -Destination "$PSScriptRoot\release\" -Force
    Write-Host "[COPY] Copiado para: $PSScriptRoot\release\" -ForegroundColor Green
}

# Copiar DLLs
Write-Host "[COPY] Copiando DLLs..." -ForegroundColor Cyan
$releaseDir = Join-Path $PSScriptRoot "release"
$qtPluginsDir = Join-Path $releaseDir "Qt6Plugins"
if (Test-Path "$PSScriptRoot\Release\*.dll") {
    Get-ChildItem -Path "$PSScriptRoot\Release\" -Filter "*.dll" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $releaseDir -Force
    }
    Write-Host "[COPY] DLLs copiados para: $releaseDir" -ForegroundColor Green
}

# Copiar plugins Qt
if (Test-Path "$PSScriptRoot\Release\Qt6Plugins") {
    Get-ChildItem -Path "$PSScriptRoot\Release\Qt6Plugins\*" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $releaseDir -Force
    }
    Write-Host "[COPY] Qt6Plugins copiados para: $releaseDir" -ForegroundColor Green
}

# Copiar assets
Write-Host "[COPY] Copiando assets..." -ForegroundColor Cyan
if (Test-Path "$PSScriptRoot\assets") {
    Get-ChildItem -Path "$PSScriptRoot\assets\*" | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $releaseDir -Force
    }
    Write-Host "[COPY] Assets copiados para: $releaseDir" -ForegroundColor Green
}

Set-Location -Path ".."

Write-Host ""
Write-Host "=============================================================================" -ForegroundColor Cyan
Write-Host "PROJETO CONCLUIDO - Todos os arquivos foram copiados para 'release\'" -ForegroundColor Cyan
Write-Host "=============================================================================" -ForegroundColor Cyan