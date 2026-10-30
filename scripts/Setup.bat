@echo off
setlocal EnableDelayedExpansion
title URBANTIS — Workstation Installer & Shortcut Setup
color 0A

echo ===============================================================================
echo                URBANTIS: PUNE CYBER-PHYSICAL WORKSTATION
echo                  Desktop & Start Menu Shortcut Setup Wizard
echo ===============================================================================
echo.

set "SCRIPT_DIR=%~dp0"
if exist "%SCRIPT_DIR%Urbantis.exe" (
    set "TARGET_DIR=%SCRIPT_DIR%"
) else if exist "%SCRIPT_DIR%dist_app\Urbantis-win32-x64\Urbantis.exe" (
    set "TARGET_DIR=%SCRIPT_DIR%dist_app\Urbantis-win32-x64"
) else if exist "%SCRIPT_DIR%..\dist_app\Urbantis-win32-x64\Urbantis.exe" (
    set "TARGET_DIR=%SCRIPT_DIR%..\dist_app\Urbantis-win32-x64"
) else (
    echo [ERROR] Could not find Urbantis.exe.
    echo Please make sure this file is run from the extracted application folder or repository root.
    pause
    exit /b 1
)

echo [1/2] Creating Windows Desktop and Start Menu shortcuts...
powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%create_shortcut.ps1" -TargetDir "!TARGET_DIR!"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ===============================================================================
    echo   Setup Complete! URBANTIS is ready to use from your Desktop or Start Menu.
    echo ===============================================================================
    echo.
    set /p LAUNCH="Do you want to launch URBANTIS now? (Y/N) [default Y]: "
    if /i "!LAUNCH!"=="" set LAUNCH=Y
    if /i "!LAUNCH!"=="Y" (
        echo Launching URBANTIS...
        start "" "!TARGET_DIR!\Urbantis.exe"
    )
) else (
    echo [ERROR] Setup encountered an issue creating shortcuts.
    pause
)
