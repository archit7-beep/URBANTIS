#include "SimulationEngine.hpp"
#include "Dashboard.hpp"
#include <iostream>
#include <string>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

void enableVirtualTerminalProcessing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void portableSleep(int milliseconds) {
    Sleep(milliseconds);
}
#else
#include <thread>
void enableVirtualTerminalProcessing() {}
void portableSleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
#endif

using namespace urbantis;

void runShowcase(SimulationEngine& engine, Dashboard& dashboard) {
    std::cout << "\n>>> INITIATING URBANTIS AUTONOMOUS DEMONSTRATION SHOWCASE <<<\n";
    portableSleep(800);

    // Phase 1: Baseline Initialization
    std::cout << "[Step 1/5] Initializing Smart City Grid & Baseline Operations...\n";
    for (int i = 0; i < 5; ++i) {
        engine.submitRandomDeliveryOrder();
        engine.step(1.0);
    }
    dashboard.render(engine);
    portableSleep(1200);

    // Phase 2: Cyber Attack Injection
    std::cout << "\n[Step 2/5] Injecting Staged Killchain Cyber Attack on SIG-CTRL-13...\n";
    engine.triggerKillchainAttack("192.168.1.100", "SIG-CTRL-13");
    engine.step(1.0);
    dashboard.render(engine);
    portableSleep(1500);

    // Phase 3: Volumetric Burst / DDoS Flood on Regional Gateway
    std::cout << "\n[Step 3/5] Injecting Volumetric Microcluster DDoS Flood on GW-NORTH-01...\n";
    engine.triggerDDoSAttackOnGateway("GW-NORTH-01", 300);
    engine.step(1.0);
    dashboard.render(engine);
    portableSleep(1500);

    // Phase 4: Emergency Accident & Dynamic Green Corridor Preemption
    std::cout << "\n[Step 4/5] Multi-Vehicle Collision Reported at Intersection 14 (911 Dispatch)...\n";
    engine.triggerTrafficAccident(14, 5);
    for (int i = 0; i < 4; ++i) {
        engine.step(1.0);
    }
    dashboard.render(engine);
    portableSleep(1500);

    // Phase 5: Run Full Benchmark Report
    std::cout << "\n[Step 5/5] Executing Algorithmic Benchmark Suite...\n";
    BenchmarkSuiteResults bench = engine.runFullBenchmarkSuite(100);
    dashboard.renderBenchmarkReport(bench);
    std::cout << "Showcase completed successfully!\n";
}

int main(int argc, char* argv[]) {
    enableVirtualTerminalProcessing();

    SimulationEngine engine;
    Dashboard dashboard;
    engine.initializeScenario(5, 5);

    // Check command line arguments
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--benchmark") {
            BenchmarkSuiteResults bench = engine.runFullBenchmarkSuite(100);
            dashboard.renderBenchmarkReport(bench);
            return 0;
        } else if (arg == "--showcase") {
            runShowcase(engine, dashboard);
            return 0;
        }
    }

    // Default: Interactive Console
    dashboard.render(engine);

    int choice = -1;
    while (true) {
        dashboard.printHelpMenu();
        if (!(std::cin >> choice)) {
            break;
        }

        if (choice == 0) {
            std::cout << "Exiting URBANTIS Simulator. Goodbye!\n";
            break;
        } else if (choice == 1) {
            engine.step(1.0);
            dashboard.render(engine);
        } else if (choice == 2) {
            for (int i = 0; i < 10; ++i) {
                engine.step(1.0);
            }
            dashboard.render(engine);
        } else if (choice == 3) {
            engine.triggerDDoSAttackOnGateway("GW-NORTH-01", 300);
            engine.step(1.0);
            dashboard.render(engine);
        } else if (choice == 4) {
            engine.triggerKillchainAttack("192.168.1.100", "SIG-CTRL-13");
            engine.step(1.0);
            dashboard.render(engine);
        } else if (choice == 5) {
            engine.triggerTrafficAccident(14, 5);
            engine.step(1.0);
            dashboard.render(engine);
        } else if (choice == 6) {
            for (int i = 0; i < 5; ++i) {
                engine.submitRandomDeliveryOrder();
            }
            engine.step(1.0);
            dashboard.render(engine);
        } else if (choice == 7) {
            BenchmarkSuiteResults bench = engine.runFullBenchmarkSuite(100);
            dashboard.renderBenchmarkReport(bench);
        } else if (choice == 8) {
            runShowcase(engine, dashboard);
        } else {
            std::cout << "Invalid choice. Please enter 0 - 8.\n";
        }
    }

    return 0;
}
