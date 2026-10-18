#include "Simulation/Simulation.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
using namespace std;

Simulation::Simulation(double _dt, unsigned _maxWorkers)
    : dt(_dt), time(0.0), stepCount(0), maxWorkers(_maxWorkers), running(false) {
    if (!(_dt > 0)) throw invalid_argument("Simulation: dt phai > 0");
    if (maxWorkers == 0) maxWorkers = thread::hardware_concurrency();   // có thể trả về 0 nếu không biết
    if (maxWorkers == 0) maxWorkers = 2;
}

Simulation::~Simulation() { stop(); }

void Simulation::requireStopped() const {
    if (running.load()) throw logic_error("Simulation: dang chay nen, hay goi stop() truoc");
}

void Simulation::addRoad(unique_ptr<Road> r) {
    requireStopped();
    if (!r) throw invalid_argument("Simulation: khong the them Road nullptr");
    roads.push_back(move(r));
}

void Simulation::addIntersection(unique_ptr<Intersection> i) {
    requireStopped();
    if (!i) throw invalid_argument("Simulation: khong the them Intersection nullptr");
    intersections.push_back(move(i));
}

// ---------- một bước ----------
void Simulation::updateRoads() {
    unsigned n = getWorkerCount();
    if (n <= 1) {                                  // 1 thread: chạy luôn trên thread hiện tại
        for (auto& r : roads) r->update(dt);
        return;
    }
    // Thread w update các Road w, w+n, w+2n, ... -> mỗi Road thuộc đúng 1 thread, không thread nào ghi chung.
    // Tạo thread mới mỗi bước: đơn giản, tốn thêm chút thời gian nhưng số thread luôn <= maxWorkers.
    vector<thread> threads;
    for (unsigned w = 0; w < n; w++) {
        threads.emplace_back([this, w, n] {
            for (size_t i = w; i < roads.size(); i += n) roads[i]->update(dt);
        });
    }
    for (auto& t : threads) t.join();              // GĐ 2 chỉ bắt đầu khi mọi Road đã xong
}

void Simulation::doStep() {
    updateRoads();                                          // GĐ 1: xe (song song)
    for (auto& in : intersections) in->update(dt);          // GĐ 2: đèn + chuyển xe (1 thread)
    time.store(time.load() + dt);   // chỉ thread đang chạy mô phỏng ghi time -> load + store là đủ
    stepCount++;
}

void Simulation::step() {
    requireStopped();
    doStep();
}

void Simulation::run(int steps) {
    requireStopped();
    if (steps < 0) throw invalid_argument("Simulation: so buoc phai >= 0");
    cout << "Simulation started (dt=" << dt << "s, steps=" << steps << ")\n";
    display();                       // trạng thái hiện tại (lần đầu gọi run() là Step 0)
    for (int i = 0; i < steps; i++) {
        doStep();
        display();                   // trạng thái sau mỗi bước
    }
    cout << "Simulation finished (t=" << time.load() << "s)\n";
}

// ---------- thời gian thực (Phase 10) ----------
void Simulation::realTimeLoop(int maxSteps, double speed) {
    // Độ dài 1 tick theo thời gian THẬT. steady_clock: đồng hồ không bị chỉnh lùi/tiến như giờ hệ thống.
    auto tick = chrono::duration_cast<chrono::steady_clock::duration>(chrono::duration<double>(dt / speed));
    printSummary();
    auto nextTick = chrono::steady_clock::now();
    int done = 0;
    while (maxSteps < 0 ? running.load() : done < maxSteps) {
        // Chờ TỚI một thời điểm cố định (sleep_until), không chờ MỘT KHOẢNG (sleep_for):
        // nếu bước trước chạy lâu thì lần chờ sau ngắn lại -> thời gian không bị trôi dần.
        nextTick += tick;
        this_thread::sleep_until(nextTick);
        if (maxSteps < 0 && !running.load()) break;   // stop() được gọi trong lúc đang chờ
        doStep();
        printSummary();
        done++;
    }
}

void Simulation::runRealTime(double simSeconds, double speed) {
    requireStopped();
    if (!(simSeconds >= 0)) throw invalid_argument("Simulation: simSeconds phai >= 0");
    if (!(speed > 0)) throw invalid_argument("Simulation: speed phai > 0");
    int steps = static_cast<int>(simSeconds / dt + 1e-9);   // 1e-9: tránh sai số số thực
    cout << "Real-time simulation started (dt=" << dt << "s, x" << speed << ")\n";
    realTimeLoop(steps, speed);
    cout << "Real-time simulation finished (t=" << time.load() << "s)\n";
}

// ---------- chạy nền (Phase 12) ----------
void Simulation::start(double speed) {
    if (!(speed > 0)) throw invalid_argument("Simulation: speed phai > 0");
    if (running.load()) throw logic_error("Simulation: dang chay roi");
    running.store(true);   // đặt TRƯỚC khi tạo thread, nếu không thread có thể thấy false và thoát ngay
    try {
        worker = thread([this, speed] { realTimeLoop(-1, speed); });
    } catch (...) {
        running.store(false);
        throw;
    }
}

void Simulation::stop() {
    running.store(false);
    if (worker.joinable()) worker.join();   // đợi thread nền chạy xong bước hiện tại rồi thoát
}

bool Simulation::isRunning() const { return running.load(); }

// ---------- hiển thị ----------
void Simulation::printSummary() const {
    cout << "t=" << time.load() << "s";
    for (const auto& r : roads) cout << " | " << r->getName() << ": " << r->getVehicleCount() << " xe";
    for (const auto& in : intersections) {
        cout << " | " << in->getName() << ":";
        for (size_t i = 0; i < in->getApproachCount(); i++) cout << " " << in->getLight(i)->getStateName();
    }
    cout << "\n";
}

void Simulation::displaySummary() const {
    requireStopped();
    printSummary();
}

void Simulation::display() const {
    requireStopped();
    cout << "=========== Step " << stepCount.load() << " (t=" << time.load() << "s) ===========\n";
    for (const auto& r : roads) r->display();
    for (const auto& in : intersections) in->display();
}

// ---------- getter ----------
double Simulation::getTime() const { return time.load(); }
int Simulation::getStepCount() const { return stepCount.load(); }

int Simulation::getVehicleCount() const {
    requireStopped();
    int total = 0;
    for (const auto& r : roads) total += r->getVehicleCount();
    return total;
}

unsigned Simulation::getWorkerCount() const {
    return min(maxWorkers, static_cast<unsigned>(roads.size()));
}
