#include <iostream>
#include <memory>
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/States/Stopped_State.h"

// Stopped: đứng yên vì targetSpeed = 0 hoặc bị xe trước chặn.
// Tự động chạy lại khi được phép đi (không cần ai gọi accelerate() mỗi tick).

void Stopped::accelerate(Vehicle& vehicle) {
    if (vehicle.getDesiredSpeed(0.0) > Vehicle::SPEED_EPS) vehicle.setState(std::make_unique<Accelerating>());
    else std::cout << "Chua the di (target=0 hoac chua du khoang cach an toan), tiep tuc cho!\n";
}

void Stopped::brake([[maybe_unused]] Vehicle& vehicle) { std::cout << "Xe dang dung roi!\n"; }
void Stopped::stop([[maybe_unused]] Vehicle& vehicle)  { std::cout << "Xe dang dung roi!\n"; }

void Stopped::update(Vehicle& vehicle, double dt) {
    // speed > 0 (vd. khởi tạo xe đang chạy) hoặc đã được phép đi -> Accelerating xử lý tiếp
    if (vehicle.getSpeed() > Vehicle::SPEED_EPS || vehicle.getDesiredSpeed(dt) > Vehicle::SPEED_EPS) {
        vehicle.transitionTo(std::make_unique<Accelerating>(), dt);
        return;
    }
    vehicle.setSpeed(0.0);
}

const char* Stopped::getName() const { return "Stopped"; }
