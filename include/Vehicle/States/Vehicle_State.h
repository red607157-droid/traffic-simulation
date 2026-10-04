#ifndef VEHICLE_STATE_H
#define VEHICLE_STATE_H

class Vehicle;  

class VehicleState {
public:
    virtual ~VehicleState() = default;

// hành vi của xe
    virtual void accelerate(Vehicle& vehicle) = 0;
    virtual void brake(Vehicle& vehicle) = 0;
    virtual void stop(Vehicle& vehicle) = 0;

    virtual void update(Vehicle& vehicle, double dt) = 0;

    virtual const char* getName() const = 0;
};

#endif