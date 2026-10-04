#ifndef ACCELERATING_STATE_H
#define ACCELERATING_STATE_H

#include <Vehicle/States/Vehicle_State.h>

class Accelerating: public VehicleState{
    public:
        void accelerate(Vehicle& vehicle);
        void brake(Vehicle& vehicle);
        void stop(Vehicle& vehicle);

        void update(Vehicle& vehicle, double dt);

        const char* getName() const override;
};



#endif