#ifndef CRUISING_STATE_H
#define CRUISING_STATE_H

#include <Vehicle/States/Vehicle_State.h>

class Cruising: public VehicleState{
    public:
        void accelerate(Vehicle& vehicle) override;
        void brake(Vehicle& vehicle) override;
        void stop(Vehicle& vehicle) override;

        void update(Vehicle& vehicle, double dt) override;
        
        const char* getName() const override;
};

#endif