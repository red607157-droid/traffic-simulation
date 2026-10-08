#include <iostream>
#include <memory>
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Cruising_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Stopped_State.h"
#include "Vehicle/States/WaitingAtLight_State.h"
#include <algorithm>

void Accelerating::accelerate(Vehicle& vehicle) {
    double speedLimit = std::min(vehicle.getTargetSpeed(), vehicle.getMaxSpeed());
    
    if (vehicle.getSpeed() >= speedLimit) {
        std::cout << "Da dat toc do muc tieu hoac toi da!\n";
        vehicle.setState(std::make_unique<Cruising>());
    }
}

void Accelerating::brake(Vehicle& vehicle){
    vehicle.setState(std::make_unique<Braking>());
}

void Accelerating::stop(Vehicle& vehicle) {
    vehicle.setState(std::make_unique<Braking>());
}

void Accelerating::update(Vehicle& vehicle, double dt) {
   // rà quét biến cố bằng khoảng cách động
    if (vehicle.getDistanceToVehicleAhead() < vehicle.getDynamicSafeDistance()) {
        vehicle.setState(std::make_unique<Braking>());
        return;
    }

    // kiểm tra đèn (chỉ xe đầu hàng)
    if (vehicle.getVehicleAhead() == nullptr) {
        if (vehicle.getTrafficLight() == Vehicle::redLight || vehicle.getTrafficLight() == Vehicle::yellowLight) {
            vehicle.setState(std::make_unique<Braking>());
            return;
        }
    }

    // tính toán vận tốc mới
    double newSpeed = vehicle.getSpeed() + vehicle.getAcceleration() * dt;
    double speedLimit = std::min(vehicle.getTargetSpeed(), vehicle.getMaxSpeed());
    
    if (newSpeed >= speedLimit) {
        newSpeed = speedLimit;
        vehicle.setSpeed(newSpeed);
        vehicle.setState(std::make_unique<Cruising>());
    } else {
        vehicle.setSpeed(newSpeed);
    }
    
    // cập nhật vị trí và chặn vị trí không vượt qua xe trước
    vehicle.setPosition(vehicle.getPosition() + newSpeed * dt);
    
    if (vehicle.getVehicleAhead() != nullptr) {
        double maxAllowedPos = vehicle.getVehicleAhead()->getPosition() - vehicle.getSafeDistance();
        if (vehicle.getPosition() > maxAllowedPos) {
            vehicle.setPosition(maxAllowedPos);
        }
    }
}

const char* Accelerating::getName() const { return "Accelerating";}