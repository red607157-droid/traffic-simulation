#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <memory>
#include <string>
#include "TrafficLightState.h"

// Phase 4: the old switch-based transition() is gone. TrafficLight
// is now just a "context" object (State Pattern terminology) that
// holds whatever state it's currently in and forwards to it.
class TrafficLight {
private:
    int id;
    std::unique_ptr<TrafficLightState> state;

public:
    TrafficLight(int _id, double redDur, double greenDur, double yellowDur);

    void update(double dt);   // same public signature as Phase 3
    std::string getStateName() const;
    double getTimeRemaining() const;
    void display() const;
};

#endif
