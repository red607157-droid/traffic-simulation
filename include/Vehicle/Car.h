#ifndef CAR_H
#define CAR_H

#include "Vehicle/Vehicle.h"

class Car : public Vehicle {
public:
    Car(int _i, double _s, double _f);

    void display() const override;
    double getMaxSpeed() const override;
    double getAcceleration() const override;
    double getDeceleration() const override;
    double getFuelPerMeter() const override;
};

#endif
