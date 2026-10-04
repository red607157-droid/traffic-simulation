#include <iostream>
#include <memory>
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Cruising_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Stopped_State.h"
#include "Vehicle/States/WaitingAtLight_State.h"
#include <algorithm>

void Stopped::accelerate(Vehicle& vehicle){
    vehicle.setState(std::make_unique<Accelerating>());
}

void Stopped::brake(Vehicle& vehicle){
    std::cout<<"Xe dang dung roi!"<<'\n';
}

void Stopped::stop(Vehicle& vehicle){
    std::cout<<"Xe dang dung roi!"<<'\n';
}

void Stopped::update(Vehicle& vehicle, double dt){
    vehicle.setSpeed(0.0);
    vehicle.setPosition(vehicle.getPosition());
}

const char* Stopped::getName() const { return "Stopped";}