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
    if(lightState == Vehicle::greenLight) {
        vehicle.setState(std::make_unique<Accelerating>());}
    else if(lightState == Vehicle::redLight){
        vehicle.setState(std::make_unique<Stopped>());}
    else if(lightState == Vehicle::yellowLight){
        vehicle.setState(std::make_unique<Braking>());}
    else{}
}