#include "Simulation/Simulation.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace std;

Simulation::Simulation(double _dt) : dt(_dt), time(0.0), stepCount(0) {
    if (!(_dt > 0)) throw invalid_argument("Simulation: dt phai > 0");
}

void Simulation::addComponent(unique_ptr<RoadComponent> c) {
    if (!c) throw invalid_argument("Simulation: khong the them thanh phan nullptr");
    components.push_back(move(c));
}

void Simulation::step() {
    for (auto& c : components) c->update(dt);   // Composite: Road / Intersection đều gọi chung update()
    time += dt;
    ++stepCount;
}

void Simulation::run(int steps) {
    if (steps < 0) throw invalid_argument("Simulation: so buoc phai >= 0");
    cout << "Simulation started (dt=" << dt << "s, steps=" << steps << ")\n";
    display();                       // trạng thái hiện tại (lần đầu gọi run() là Step 0)
    for (int i = 0; i < steps; i++) {
        step();
        display();                   // trạng thái sau mỗi bước
    }
    cout << "Simulation finished (t=" << time << "s)\n";
}

void Simulation::runRealTime(double simSeconds, double speed) {
    if (!(simSeconds >= 0)) throw invalid_argument("Simulation: simSeconds phai >= 0");
    if (!(speed > 0)) throw invalid_argument("Simulation: speed phai > 0");

    // Độ dài 1 tick theo thời gian THẬT. steady_clock: đồng hồ không bị chỉnh lùi/tiến như giờ hệ thống.
    auto tick = chrono::duration_cast<chrono::steady_clock::duration>(chrono::duration<double>(dt / speed));
    int steps = static_cast<int>(simSeconds / dt + 1e-9);   // số bước cần chạy (1e-9: tránh sai số số thực)

    cout << "Real-time simulation started (dt=" << dt << "s, x" << speed << ")\n";
    displaySummary();
    auto nextTick = chrono::steady_clock::now();
    for (int i = 0; i < steps; i++) {
        // Chờ TỚI một thời điểm cố định (sleep_until), không chờ MỘT KHOẢNG (sleep_for):
        // nếu step() chạy lâu thì lần chờ sau ngắn lại -> thời gian không bị trôi dần.
        // Nếu đã trễ hơn nextTick thì sleep_until trả về ngay.
        nextTick += tick;
        this_thread::sleep_until(nextTick);
        step();
        displaySummary();
    }
    cout << "Real-time simulation finished (t=" << time << "s)\n";
}

void Simulation::displaySummary() const {
    cout << "t=" << time << "s";
    for (const auto& c : components) cout << " | " << c->getName() << ": " << c->getVehicleCount() << " xe";
    cout << "\n";
}

void Simulation::display() const {
    cout << "=========== Step " << stepCount << " (t=" << time << "s) ===========\n";
    for (const auto& c : components) c->display();
}

double Simulation::getTime() const { return time; }
int Simulation::getStepCount() const { return stepCount; }

int Simulation::getVehicleCount() const {
    int total = 0;
    for (const auto& c : components) total += c->getVehicleCount();
    return total;
}
