#ifndef CAR_H
#define CAR_H

#include "Vehicle.h"

class Car : public Vehicle {
private:
    double fuel;

public:
    Car(int _i, double _s, double _f) : Vehicle(_i, _s), fuel(_f) {}

    double getFuel() const { return fuel; }
    void setFuel(double _f) { fuel = _f; }

    void move(double dt) override;
    void display() const override;
    double getMaxSpeed() const override;
};

#endif
