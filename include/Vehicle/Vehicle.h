#ifndef VEHICLE_H
#define VEHICLE_H

#include "Vehicle/States/Vehicle_State.h"
#include <memory>
#include <string>

class Vehicle {
protected:
    int id;
    double speed;              
    double position;  
    double targetSpeed; 
    enum trafficLight{redLight, greenLight, yellowLight};
    trafficLight currentTrafficLight;
    std::unique_ptr<VehicleState> currentState;

public:
    Vehicle(int _i, double _s = 0.0) ;  

// điều chỉnh state của vehicle
    void setState(std::unique_ptr<VehicleState> newState);
    VehicleState* getState() const;

    void accelerate();
    void brake();
    void stop();
    void update(double dt);     

// các hàm getter/setter
    int getId() const;
    double getSpeed() const;  
    double getPosition() const;
    double getTargetSpeed() const;
    trafficLight getTrafficLight() const;
    void setSpeed(double _s);
    void setPosition(double _p);
    void setTargetSpeed(double _ts);
    void setTrafficLight(trafficLight _tl);


    virtual double getMaxSpeed() const = 0;
    virtual double getAcceleration() const = 0;   
    virtual double getDeceleration() const = 0;   
    virtual void display() const = 0;

    virtual ~Vehicle() = default;
};

#endif