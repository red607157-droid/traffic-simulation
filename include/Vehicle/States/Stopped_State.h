#ifndef STOPPED_STATE_H
#define STOPPED_STATE_H

#include <Vehicle/States/Vehicle_State.h>

class Stopped: public VehicleState{
    public:
        void accelerate(Vehicle& vehicle);
        void brake(Vehicle& vehicle);
        void stop(Vehicle& vehicle);

        void update(Vehicle& vehicle, double dt);
        
        const char* getName() const override;
};

#endif