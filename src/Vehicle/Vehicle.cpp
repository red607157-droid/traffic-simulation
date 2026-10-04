#include <iostream>
#include <vector>
#include <memory>

#include "Vehicle/Vehicle.h"
#include <Vehicle/States/Vehicle_State.h>
#include "Vehicle/States/Cruising_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Stopped_State.h"
#include "Vehicle/States/Accelerating_State.h"

// hàm khởi tạo(constructor)
Vehicle::Vehicle(int _i, double _s)  { 
    id = _i; speed = _s; 
    currentState= std::make_unique<Stopped>();
    currentTrafficLight = greenLight;
}

// hàm điều chỉnh state:
void Vehicle::setState(std::unique_ptr<VehicleState> newState){ currentState = std::move(newState);}
VehicleState* Vehicle:: getState() const  { return currentState.get();}

void Vehicle::accelerate()         {currentState->accelerate(*this);}
void Vehicle::brake()              {currentState->brake(*this);}
void Vehicle::stop()               {currentState->stop(*this);}
void Vehicle::update(double dt)    {currentState->update(*this,dt);}

// hàm getter/setter
int Vehicle::getId() const                               { return id;}
double Vehicle::getSpeed() const                         { return speed;}
double Vehicle::getPosition() const                      { return position;}
double Vehicle::getTargetSpeed() const                   { return targetSpeed;}
Vehicle::trafficLight Vehicle::getTrafficLight() const   { return currentTrafficLight;}
void Vehicle::setSpeed( double _s)                       { speed = _s;} 
void Vehicle::setPosition(double _p)                     { position = _p;}
void Vehicle::setTargetSpeed(double _ts)                 { targetSpeed = _ts;}
void Vehicle:: setTrafficLight(trafficLight _tl)         { currentTrafficLight = _tl;}
