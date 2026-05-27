@echo off
setlocal enabledelayedexpansion

echo ========================================
echo RAMFlux - Build Script
echo ========================================
echo.

REM Remover build anterior se existir
if exist build (
    echo Removendo pasta build anterior...
    rmdir /s /q build
    echo.
)

REM Criar nova pasta build
mkdir build
cd build

REM Configurar ambiente Visual Studio 2022
echo Configurando ambiente de build...
echo.
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"

REM Configurar CMake
echo Configurando CMake...
cmake .. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -G "Visual Studio 17 2022" ^
    -A x64
if errorlevel 1 (
    echo Erro na configuracao do CMake
    cd /d %~dp0
    pause
    exit /b 1
)
echo.

REM Compilar
echo Compilando o projeto...
cmake --build . --config Release
if errorlevel 1 (
    echo Erro na compilacao
    cd /d %~dp0
    pause
    exit /b 1
)
echo.

echo ========================================
echo Build concluido com sucesso!
echo Executavel: %CD%\Release\RAMFlux.exe
echo ========================================

REM Copiar para pasta release
xcopy "%CD%\Release\RAMFlux.exe" "../release/" /Y
xcopy "%CD%\Release\*.dll" "../release/" /Y /S

echo Arquivos copiados para: %CD%\release\
echo.

cd /d %~dp0
pause