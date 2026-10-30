@echo off
echo ===================================================
echo             URBANTIS BUILD SCRIPT (C++)
echo ===================================================

if not exist bin mkdir bin

echo Compiling URBANTIS Simulator (bin\urbantis.exe)...
g++ -std=c++14 -O3 -Wall -Wextra -Iinclude src\CityGraph.cpp src\TrafficEngine.cpp src\ServiceAssignment.cpp src\EmergencyManager.cpp src\ThreatDetector.cpp src\ImpactAnalyzer.cpp src\ResponseOptimizer.cpp src\SimulationEngine.cpp src\Dashboard.cpp src\main.cpp -o bin\urbantis.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile urbantis.exe!
    exit /b %ERRORLEVEL%
)

echo Compiling Test Suite (bin\test_suite.exe)...
g++ -std=c++14 -O3 -Wall -Wextra -Iinclude src\CityGraph.cpp src\TrafficEngine.cpp src\ServiceAssignment.cpp src\EmergencyManager.cpp src\ThreatDetector.cpp src\ImpactAnalyzer.cpp src\ResponseOptimizer.cpp tests\benchmark.cpp -o bin\test_suite.exe
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile test_suite.exe!
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Build completed successfully!
echo Executables generated:
echo   - bin\urbantis.exe
echo   - bin\test_suite.exe
