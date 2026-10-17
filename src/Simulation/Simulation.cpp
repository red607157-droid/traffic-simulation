#include "Simulation/Simulation.h"
#include "Integration/LightMapper.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>
using namespace std;

Simulation::Simulation(double _dt) : dt(_dt), time(0.0), stepCount(0), lane(), light(nullptr), vehicles() {
    if (!(_dt > 0)) throw invalid_argument("Simulation: dt phai > 0");
}

Vehicle& Simulation::addVehicle(unique_ptr<Vehicle> v) {
    if (!v) throw invalid_argument("Simulation: khong the them xe nullptr");
    v->setCurrentLane(&lane);
    if (!vehicles.empty()) v->setVehicleAhead(vehicles.back().get());
    vehicles.push_back(std::move(v));
    return *vehicles.back();
}

void Simulation::setTrafficLight(unique_ptr<TrafficLight> tl) { light = std::move(tl); }

void Simulation::step() {
    if (light) {
        Vehicle::trafficLight signal = LightMapper(*light);   // nối đèn -> xe
        for (auto& v : vehicles) v->setTrafficLight(signal);
    }
    for (auto& v : vehicles) v->update(dt);   // xe đầu hàng trước
    if (light) light->update(dt);
    time += dt;
    ++stepCount;
}

void Simulation::run(int steps) {
    if (steps < 0) throw invalid_argument("Simulation: so buoc phai >= 0");
    cout << "Simulation started (dt=" << dt << "s, steps=" << steps << ")\n";
    display();                       // trạng thái ban đầu (Step 0)
    for (int i = 0; i < steps; i++) {
        step();
        display();                   // trạng thái sau mỗi bước
    }
    cout << "Simulation finished (t=" << time << "s)\n";
}

void Simulation::display() const {
    ios oldState(nullptr);
    oldState.copyfmt(cout);   // giữ nguyên định dạng cout của người gọi
    cout << fixed << setprecision(2);
    cout << "=========== Step " << stepCount << " (t=" << time << "s) ===========\n";
    if (light) light->display();
    for (const auto& v : vehicles) {
        cout << "  #" << v->getId() << " " << setw(14) << left << v->getState()->getName() << right
             << " pos=" << setw(7) << v->getPosition()
             << " speed=" << setw(6) << v->getSpeed() << "\n";
    }
    cout.copyfmt(oldState);
}

double Simulation::getDt() const { return dt; }
double Simulation::getTime() const { return time; }
int Simulation::getStepCount() const { return stepCount; }
const TrafficLight* Simulation::getTrafficLight() const { return light.get(); }
const vector<unique_ptr<Vehicle>>& Simulation::getVehicles() const { return vehicles; }
