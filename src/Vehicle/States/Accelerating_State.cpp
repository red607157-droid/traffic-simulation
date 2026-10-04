#include <iostream>
#include <memory>
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Cruising_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Stopped_State.h"
#include "Vehicle/States/WaitingAtLight_State.h"
#include <algorithm>

void Accelerating::accelerate(Vehicle& vehicle){
    if( vehicle.getSpeed() >= vehicle.getMaxSpeed()) {
        std::cout<<"Da dat toc do toi da!"<<"\n";
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
    double newSpeed = vehicle.getSpeed() + vehicle.getAcceleration() * dt;
    newSpeed = std::min(newSpeed, vehicle.getMaxSpeed());

    vehicle.setSpeed(newSpeed);
    if(vehicle.getSpeed() >= std::min(vehicle.getTargetSpeed(), vehicle.getMaxSpeed())) {
        vehicle.setState(std::make_unique<Cruising>());
    }

        vehicle.setPosition(vehicle.getPosition() + newSpeed * dt);
    }

const char* Accelerating::getName() const { return "Accelerating";}