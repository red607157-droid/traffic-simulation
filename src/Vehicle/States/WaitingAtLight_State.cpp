#include <iostream>
#include <memory>
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Cruising_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Stopped_State.h"
#include "Vehicle/States/WaitingAtLight_State.h"
#include <algorithm>


void WaitingAtLight::accelerate(Vehicle &vehicle){
    Vehicle::trafficLight lightState = vehicle.getTrafficLight();
   if (lightState == Vehicle::greenLight) {
        if (vehicle.getDistanceToVehicleAhead() >= vehicle.getSafeDistance()) {
            vehicle.setState(std::make_unique<Accelerating>());
        } else {
            vehicle.setState(std::make_unique<Stopped>());
        }
    } else {
        std::cout << "Khong the tang toc! Den chua chuyen xanh.\n";
    }
}

void WaitingAtLight::brake(Vehicle& vehicle) {
    std::cout << "Xe da dung yen de cho den!\n";
}

void WaitingAtLight::stop(Vehicle& vehicle) {
    std::cout << "Xe dang dung cho den!\n";
}

void WaitingAtLight::update(Vehicle& vehicle, double dt) {
    vehicle.setSpeed(0.0);

    // Kiểm tra tín hiệu đèn giao thông
    if (vehicle.getTrafficLight() == Vehicle::greenLight) {
        // Đèn chuyển xanh: đánh giá khoảng cách với xe phía trước
        if (vehicle.getDistanceToVehicleAhead() >= vehicle.getSafeDistance()) {
            vehicle.setState(std::make_unique<Accelerating>());
        } else {
            vehicle.setState(std::make_unique<Stopped>());
        }
        return;
    }
}