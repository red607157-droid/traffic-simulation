#ifndef MOTORBIKE_H
#define MOTORBIKE_H

#include "Vehicle.h"

class Motorbike : public Vehicle {
private:
    int engineCC;
    double fuel;

public:
    Motorbike(int _i, double _s, int _e, double _f)
        : Vehicle(_i, _s), engineCC(_e), fuel(_f) {}

    void setEngineCC(int _e) { engineCC = _e; }
    int getEngineCC() const { return engineCC; }
    void setFuel(double _f) { fuel = _f; }
    double getFuel() const { return fuel; }

    void move(double dt) override;
    void display() const override;
    double getMaxSpeed() const override;
};

#endif
