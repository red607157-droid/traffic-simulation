#ifndef BUS_H
#define BUS_H

#include "Vehicle/Vehicle.h"

class Bus : public Vehicle {
private:
    int capacity;   // sức chứa (số hành khách)

public:
    Bus(int _i, double _s, double _f, int _c);

    int getCapacity() const;
    void setCapacity(int _c);

    void display() const override;
    double getMaxSpeed() const override;
    double getAcceleration() const override;
    double getDeceleration() const override;
    double getFuelPerMeter() const override;
};

#endif
