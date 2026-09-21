#include "TrafficLight.h"
#include "RedState.h"
#include <iostream>
using namespace std;

TrafficLight::TrafficLight(int _id, double redDur, double greenDur, double yellowDur)
    : id(_id) {
    TrafficLightConfig config{redDur, greenDur, yellowDur};
    state = make_unique<RedState>(config); // every light starts on RED
}

void TrafficLight::update(double dt) {
    state->update(dt);
    if (state->isFinished()) {
        // No "if RED then GREEN, if GREEN then YELLOW..." here —
        // each state already knows what comes after it.
        state = state->nextState();
    }
}

string TrafficLight::getStateName() const {
    return state->getName();
}

double TrafficLight::getTimeRemaining() const {
    return state->getTimeRemaining();
}

void TrafficLight::display() const {
    cout << "TrafficLight no." << id << " -> " << getStateName()
         << " (time left: " << getTimeRemaining() << "s)\n";
}
