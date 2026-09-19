#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <string>

// Phase 3: no State Pattern yet — kept intentionally simple with a
// switch statement so the problem is visible before Phase 4 replaces
// it with the State Pattern.
enum class LightColor { RED, GREEN, YELLOW };

class TrafficLight {
private:
    int id;
    LightColor state;
    double redDuration;
    double greenDuration;
    double yellowDuration;
    double timer; // time remaining before the current state ends

public:
    TrafficLight(int _id, double _redDur, double _greenDur, double _yellowDur);

    LightColor getState() const { return state; }
    double getTimeRemaining() const { return timer; }

    void update(double dt);      // advance by dt, same clock the vehicles use
    void transition();           // move to the next state
    std::string getStateName() const;
    void display() const;
};

#endif
