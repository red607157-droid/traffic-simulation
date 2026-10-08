#include <iostream>
#include <memory>
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Cruising_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Stopped_State.h"
#include "Vehicle/States/WaitingAtLight_State.h"
#include <algorithm>

void Braking::accelerate(Vehicle& vehicle){
    if (vehicle.getDistanceToVehicleAhead() >= vehicle.getDynamicSafeDistance()) {
        if (vehicle.getVehicleAhead() != nullptr || vehicle.getTrafficLight() == Vehicle::greenLight) {
            vehicle.setState(std::make_unique<Accelerating>());
        }
    }
}

void Braking::brake(Vehicle& vehicle){
    std::cout<<"Xe dang phanh!"<<'\n';
}

void Braking::stop(Vehicle& vehicle){
    std::cout<<"Xe dang phanh!"<<'\n';
}

void Braking::update(Vehicle& vehicle, double dt){
   // kiểm tra có thể tăng tốc lại không (đường thoáng + đèn xanh hoặc chỉ cần theo xe trước)
    if (vehicle.getDistanceToVehicleAhead() >= vehicle.getDynamicSafeDistance()) {
        if (vehicle.getVehicleAhead() != nullptr || vehicle.getTrafficLight() == Vehicle::greenLight) {
            vehicle.setState(std::make_unique<Accelerating>());
            return;
        }
    }

    // tính toán vận tốc giảm dần
    double newSpeed = vehicle.getSpeed() - vehicle.getDeceleration() * dt;
    newSpeed = std::max(newSpeed, 0.0);

    vehicle.setSpeed(newSpeed);
    vehicle.setPosition(vehicle.getPosition() + newSpeed * dt);

    // khi xe đã dừng hẳn ở tốc độ 0
    if (newSpeed <= 0.0) {
        vehicle.setSpeed(0.0);
        
        // phân loại lý do dừng: do đèn (xe đầu) hay do xe phía trước (xe sau)
        if (vehicle.getVehicleAhead() == nullptr && 
           (vehicle.getTrafficLight() == Vehicle::redLight || vehicle.getTrafficLight() == Vehicle::yellowLight)) {
            vehicle.setState(std::make_unique<WaitingAtLight>());
        } else {
            vehicle.setState(std::make_unique<Stopped>());
        }
        return;
    }
}
const char* Braking::getName() const { return "Braking";} 