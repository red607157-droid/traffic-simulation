#include "Traffic/TrafficLight.h"
#include "Traffic/RedState.h"
#include <iostream>
#include <stdexcept>
using namespace std;

TrafficLight::TrafficLight(int _id, double redDur, double greenDur, double yellowDur)
    : id(_id) {
    if (!(redDur > 0 && greenDur > 0 && yellowDur > 0))
        throw invalid_argument("TrafficLight: moi thoi gian den phai > 0");
    TrafficLightConfig config{redDur, greenDur, yellowDur};
    state = make_unique<RedState>(config); // every light starts on RED
}

void TrafficLight::update(double dt) {
    if (!(dt > 0)) return;
    state->update(dt);
    // while (không phải if): dt lớn có thể đi qua nhiều state; thời gian dư được chuyển sang state kế tiếp
    while (state->isFinished()) {
        double overshoot = -state->getTimeRemaining();   // >= 0
        state = state->nextState();
        state->update(overshoot);
    }
}

string TrafficLight::getStateName() const {
    return state->getName();
}

LightColor TrafficLight::getColor() const {
    return state->getColor();
}

double TrafficLight::getTimeRemaining() const {
    return state->getTimeRemaining();
}

void TrafficLight::display() const {
    cout << "TrafficLight no." << id << " -> " << getStateName()
         << " (time left: " << getTimeRemaining() << "s)\n";
}
