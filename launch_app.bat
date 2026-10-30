@echo off
title URBANTIS — Pune Smart City Operations & Cyber-Resilience
echo =================================================================
echo   URBANTIS: Pune Smart City Workstation (Native Windows Application)
echo   Autonomous Cyber-Resilience & 108 Emergency Preemption
echo =================================================================

if exist "%~dp0dist_app\Urbantis-win32-x64\Urbantis.exe" (
    echo Launching standalone application executable...
    start "" "%~dp0dist_app\Urbantis-win32-x64\Urbantis.exe"
) else (
    echo Standalone binary not found in dist_app.
    echo Launching desktop application via frontend runner...
    cd /d "%~dp0frontend"
    npm run app
)

