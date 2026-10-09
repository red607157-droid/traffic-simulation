#include <iostream>
#include <memory>
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/States/WaitingAtLight_State.h"

void WaitingAtLight::accelerate(Vehicle& vehicle) {
    if (vehicle.getDesiredSpeed(0.0) > Vehicle::SPEED_EPS) vehicle.setState(std::make_unique<Accelerating>());
    else std::cout << "Khong the tang toc! Den chua chuyen xanh hoac con xe chan phia truoc.\n";
}

void WaitingAtLight::brake([[maybe_unused]] Vehicle& vehicle) { std::cout << "Xe da dung yen de cho den!\n"; }
void WaitingAtLight::stop([[maybe_unused]] Vehicle& vehicle)  { std::cout << "Xe dang dung cho den!\n"; }

void WaitingAtLight::update(Vehicle& vehicle, double dt) {
    // Đèn xanh (và đường phía trước thoáng) -> đi. Mọi điều kiện đã gộp trong getDesiredSpeed.
    if (vehicle.getDesiredSpeed(dt) > Vehicle::SPEED_EPS) {
        vehicle.transitionTo(std::make_unique<Accelerating>(), dt);
        return;
    }
    vehicle.setSpeed(0.0);
}

const char* WaitingAtLight::getName() const { return "WaitingAtLight"; }
