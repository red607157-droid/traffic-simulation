#include <iostream>
#include <memory>
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Cruising_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Stopped_State.h"
#include "Vehicle/States/WaitingAtLight_State.h"
#include <algorithm>

void Cruising::accelerate(Vehicle& vehicle){
    vehicle.setState(std::make_unique<Accelerating>());
}

void Cruising::brake(Vehicle& vehicle){
    std::cout<<"Dang dung..."<<"\n";
    vehicle.setState(std::make_unique<Braking>());
}

void Cruising::stop(Vehicle& vehicle){
    std::cout<<"Dang dung"<<'\n';
    vehicle.setState(std::make_unique<Braking>());
}

void Cruising::update(Vehicle& vehicle, double dt){ 
    vehicle.setSpeed(vehicle.getTargetSpeed());
    vehicle.setPosition(vehicle.getPosition() + vehicle.getTargetSpeed() * dt);
}
const char* Cruising::getName() const { return "Cruising";}