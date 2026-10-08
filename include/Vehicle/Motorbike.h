#ifndef MOTORBIKE_H
#define MOTORBIKE_H

#include "Vehicle/Vehicle.h"

class Motorbike : public Vehicle {
private:
    int engineCC;
    double fuel;

public:
    Motorbike(int _i, double _s, int _e, double _f);

    void setEngineCC(int _e);
    int getEngineCC() const;
    void setFuel(double _f);
    double getFuel() const;

    // Bắt buộc override các hàm ảo từ Vehicle
    void display() const override;
    double getMaxSpeed() const override;
    double getAcceleration() const override;
    double getDeceleration() const override;
};

#endif