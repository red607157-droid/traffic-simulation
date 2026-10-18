#include "Simulation/Simulation.h"

#include <iostream>
#include <stdexcept>
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
