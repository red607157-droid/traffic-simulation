#include <iostream>
#include <vector>
#include <memory>

#include "Vehicle/Vehicle.h"
#include <Vehicle/States/Vehicle_State.h>
#include "Vehicle/States/Cruising_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Stopped_State.h"
#include "Vehicle/States/Accelerating_State.h"
#include "Infrastructure/Lane.h"

// hàm khởi tạo(constructor)
Vehicle::Vehicle(int _i, double _s)  { 
    id = _i; speed = _s; 
    currentState= std::make_unique<Stopped>();
    vehicleAhead = nullptr;
    currentLane = nullptr;
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
double Vehicle::getSafeDistance() const                  { return safeDistance;}
Vehicle* Vehicle::getVehicleAhead() const                { return vehicleAhead;}

void Vehicle::setSpeed( double _s)                       { speed = _s;} 
void Vehicle::setPosition(double _p)                     { position = _p;}
void Vehicle::setTargetSpeed(double _ts)                 { targetSpeed = _ts;}
void Vehicle:: setTrafficLight(trafficLight _tl)         { currentTrafficLight = _tl;}
void Vehicle::setVehicleAhead(Vehicle* v)                { vehicleAhead = v;}

void Vehicle::setCurrentLane(Lane* lane)                 {currentLane = lane;}
double Vehicle::getDistanceToNextLight() const {
    if (currentLane == nullptr) {
        return 9999.0; // xe chưa được gán vào đường, coi như không có đèn
    }
    
    // gọi hàm từ hệ thống của Duy
    return currentLane->getDistanceToNextLight(this->position);
}

double Vehicle::getDistanceToVehicleAhead() const {
    if (vehicleAhead == nullptr)    {return 9999.0;} // xe đứng đầu nên khoảng cách vô hạn 
    return vehicleAhead->getPosition() - this->getPosition();
}

double Vehicle::getDynamicSafeDistance() const {
    double v = getSpeed();
    double a = getDeceleration();
    // Quãng đường phanh = v^2 / (2*a). Cộng thêm khoảng cách an toàn cơ bản.
    if (a <= 0) return safeDistance; 
    return safeDistance + (v * v) / (2.0 * a);
}
