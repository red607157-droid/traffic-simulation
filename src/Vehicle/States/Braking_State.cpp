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
    vehicle.setState(std::make_unique<Accelerating>());
}

void Braking::brake(Vehicle& vehicle){
    std::cout<<"Xe dang phanh!"<<'\n';
}

void Braking::stop(Vehicle& vehicle){
    std::cout<<"Xe dang phanh!"<<'\n';
}

void Braking::update(Vehicle& vehicle, double dt){
    double newSpeed = vehicle.getSpeed() - vehicle.getDeceleration() * dt;
    newSpeed = std::max(newSpeed, 0.0);

    vehicle.setSpeed(newSpeed);
    vehicle.setPosition(vehicle.getPosition() + newSpeed * dt);
    if(newSpeed <= 0.0){
        vehicle.setSpeed(0.0);
        vehicle.setState(std::make_unique<Stopped>());
    }
}
const char* Braking::getName() const { return "Braking";} 