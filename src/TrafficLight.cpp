#include "TrafficLight.h"
#include <iostream>
using namespace std;

TrafficLight::TrafficLight(int _id, double _redDur, double _greenDur, double _yellowDur)
    : id(_id), state(LightColor::RED),
      redDuration(_redDur), greenDuration(_greenDur), yellowDuration(_yellowDur),
      timer(_redDur) {}

void TrafficLight::update(double dt) {
    if (dt < 0) { return; }
    timer -= dt;
    if (timer <= 0) {
        transition();
    }
}

// NOTE: this switch is exactly the kind of state-dependent
// conditional logic Phase 4's State Pattern will remove.
void TrafficLight::transition() {
    switch (state) {
        case LightColor::RED:
            state = LightColor::GREEN;
            timer = greenDuration;
            break;
        case LightColor::GREEN:
            state = LightColor::YELLOW;
            timer = yellowDuration;
            break;
        case LightColor::YELLOW:
            state = LightColor::RED;
            timer = redDuration;
            break;
    }
}

string TrafficLight::getStateName() const {
    switch (state) {
        case LightColor::RED:    return "RED";
        case LightColor::GREEN:  return "GREEN";
        case LightColor::YELLOW: return "YELLOW";
    }
    return "UNKNOWN";
}

void TrafficLight::display() const {
    cout << "TrafficLight no." << id << " -> " << getStateName()
         << " (time left: " << timer << "s)\n";
}
