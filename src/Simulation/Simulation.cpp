#include "Simulation/Simulation.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>
using namespace std;

namespace {
const double MAX_TICK_SECONDS = 3600.0;   // 1 tick thật dài hơn 1 giờ coi như speed sai (và tránh tràn số khi đổi sang steady_clock)
}

Simulation::Simulation(double _dt) : dt(_dt), time(0.0), stepCount(0) {
    if (!isfinite(_dt) || _dt <= 0) throw invalid_argument("Simulation: dt phai la so huu han > 0");
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
    // isfinite: chặn cả vô cực (inf) và NaN, không chỉ số âm.
    if (!isfinite(simSeconds) || simSeconds < 0) throw invalid_argument("Simulation: simSeconds phai la so huu han >= 0");
    if (!isfinite(speed) || speed <= 0) throw invalid_argument("Simulation: speed phai la so huu han > 0");

    // Chỉ chạy số bước TRỌN VẸN, nên simSeconds phải là bội số của dt.
    // Vd. dt = 1 thì runRealTime(2.5) bị từ chối, thay vì lặng lẽ chỉ chạy 2 bước.
    double exactSteps = simSeconds / dt;
    double wholeSteps = round(exactSteps);
    if (fabs(exactSteps - wholeSteps) > 1e-9 * max(1.0, wholeSteps))   // 1e-9: bỏ qua sai số số thực (vd. 0.3 / 0.1)
        throw invalid_argument("Simulation: simSeconds phai la boi so cua dt");
    if (wholeSteps > numeric_limits<int>::max()) throw invalid_argument("Simulation: simSeconds qua lon");
    int steps = static_cast<int>(wholeSteps);

    // Độ dài 1 tick theo thời gian THẬT. steady_clock: đồng hồ không bị chỉnh lùi/tiến như giờ hệ thống.
    double tickSeconds = dt / speed;
    if (tickSeconds > MAX_TICK_SECONDS) throw invalid_argument("Simulation: speed qua nho (1 tick dai hon 1 gio)");
    auto tick = chrono::duration_cast<chrono::steady_clock::duration>(chrono::duration<double>(tickSeconds));
    if (tick <= chrono::steady_clock::duration::zero())   // speed quá lớn: tick bị làm tròn về 0 -> vòng lặp không chờ nữa
        throw invalid_argument("Simulation: speed qua lon (1 tick ngan hon do chia cua dong ho)");

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
