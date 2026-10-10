#include <iostream>
#include <memory>
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Cruising_State.h"

void Cruising::accelerate(Vehicle& vehicle) { vehicle.setState(std::make_unique<Accelerating>()); }

void Cruising::brake(Vehicle& vehicle) {
    std::cout << "Dang phanh...\n";
    vehicle.setTargetSpeed(0.0);
    vehicle.setState(std::make_unique<Braking>());
}

void Cruising::stop(Vehicle& vehicle) { brake(vehicle); }

void Cruising::update(Vehicle& vehicle, double dt) {
    double s = vehicle.getSpeed();
    double D = vehicle.getDesiredSpeed(dt);

    if (s > D + Vehicle::SPEED_EPS) { vehicle.transitionTo(std::make_unique<Braking>(), dt); return; }
    if (s < D - Vehicle::SPEED_EPS) { vehicle.transitionTo(std::make_unique<Accelerating>(), dt); return; }
    if (D <= Vehicle::SPEED_EPS) {                          // s ~ D ~ 0
        vehicle.enterHaltState(dt, true);
        return;
    }
    vehicle.advance(s, dt);   // giữ nguyên tốc độ (không còn "teleport" tốc độ về target)
}

const char* Cruising::getName() const { return "Cruising"; }
