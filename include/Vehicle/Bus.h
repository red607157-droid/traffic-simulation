#ifndef BUS_H
#define BUS_H

#include "Vehicle.h"

class Bus : public Vehicle {
private:
    double fuel;
    int passenger;

public:
    Bus(int _i, double _s, double _f, int _p)
        : Vehicle(_i, _s), fuel(_f), passenger(_p) {}

    void setPassenger(int _p) { passenger = _p; }
    void setFuel(double _f) { fuel = _f; }
    double getFuel() const { return fuel; }
    int getPassenger() const { return passenger; }

    void move(double dt) override;
    void display() const override;
    double getMaxSpeed() const override;
};

#endif
