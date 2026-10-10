#include <algorithm>
#include <iostream>
#include <memory>
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Cruising_State.h"

void Braking::accelerate(Vehicle& vehicle) {
    if (vehicle.getDesiredSpeed(0.0) > vehicle.getSpeed() + Vehicle::SPEED_EPS)
        vehicle.setState(std::make_unique<Accelerating>());
}

void Braking::brake([[maybe_unused]] Vehicle& vehicle) { std::cout << "Xe dang phanh!\n"; }
void Braking::stop([[maybe_unused]] Vehicle& vehicle)  { std::cout << "Xe dang phanh!\n"; }

void Braking::update(Vehicle& vehicle, double dt) {
    double s = vehicle.getSpeed();
    double D = vehicle.getDesiredSpeed(dt);

    if (s <= D + Vehicle::SPEED_EPS) {                      // không cần phanh nữa
        if (D <= Vehicle::SPEED_EPS) {
            vehicle.enterHaltState(dt, true);
        } else if (s < D - Vehicle::SPEED_EPS) {
            vehicle.transitionTo(std::make_unique<Accelerating>(), dt);
        } else {
            vehicle.transitionTo(std::make_unique<Cruising>(), dt);
        }
        return;
    }

    // giảm tốc nhưng không xuống dưới mức cho phép D
    double ns = std::max(D, s - vehicle.getDeceleration() * dt);
    vehicle.advance(ns, dt);

    if (vehicle.getSpeed() <= Vehicle::SPEED_EPS) {         // dừng hẳn: phân loại lý do dừng
        vehicle.setSpeed(0.0);
        vehicle.enterHaltState(dt, false);
    }
}

const char* Braking::getName() const { return "Braking"; }
