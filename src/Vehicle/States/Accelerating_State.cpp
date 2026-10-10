#include <algorithm>
#include <iostream>
#include <memory>
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Cruising_State.h"

// Quy ước chung cho mọi state: dùng v.getDesiredSpeed(dt) (đã gộp target, maxSpeed, xe trước, đèn)
// để quyết định tăng / giữ / giảm tốc. Các điều kiện chuyển state loại trừ nhau nên không bị lặp vô hạn.

void Accelerating::accelerate(Vehicle& vehicle) {
    if (vehicle.getSpeed() >= vehicle.getDesiredSpeed(0.0) - Vehicle::SPEED_EPS) {
        std::cout << "Da dat toc do muc tieu hoac toi da!\n";
        vehicle.setState(std::make_unique<Cruising>());
    }
}

void Accelerating::brake(Vehicle& vehicle) {
    vehicle.setTargetSpeed(0.0);
    vehicle.setState(std::make_unique<Braking>());
}

void Accelerating::stop(Vehicle& vehicle) { brake(vehicle); }

void Accelerating::update(Vehicle& vehicle, double dt) {
    double s = vehicle.getSpeed();
    double D = vehicle.getDesiredSpeed(dt);

    if (s > D + Vehicle::SPEED_EPS) {                       // đang nhanh hơn mức cho phép -> phanh
        vehicle.transitionTo(std::make_unique<Braking>(), dt);
        return;
    }
    if (D <= Vehicle::SPEED_EPS && s <= Vehicle::SPEED_EPS) {     // không được phép đi -> đứng chờ
        vehicle.enterHaltState(dt, true);
        return;
    }

    double ns = std::min(s + vehicle.getAcceleration() * dt, D);
    vehicle.advance(ns, dt);
    if (ns >= D - Vehicle::SPEED_EPS) vehicle.setState(std::make_unique<Cruising>());   // đã di chuyển rồi, đổi state là xong
}

const char* Accelerating::getName() const { return "Accelerating"; }
