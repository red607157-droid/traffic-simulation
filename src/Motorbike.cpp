#include "Motorbike.h"
#include <iostream>
using namespace std;

double Motorbike::getMaxSpeed() const {
    return 100;
}

void Motorbike::display() const {
    cout << "Motorbike no." << getId() << "\n"
         << "Current speed: " << getSpeed() << "\n"
         << "EngineCC: " << getEngineCC() << "\n"
         << "Fuel: " << getFuel() << "\n"
         << "Max speed: " << getMaxSpeed() << "\n";
}

void Motorbike::move(double dt) {
    if (dt < 0) { return; }
    double fuelConsumed = 0.3 * dt;
    setFuel(getFuel() - fuelConsumed);
    if (getFuel() < 0) { cout << "Het xang roi cuh\n"; return; }
    setSpeed(6 * dt + getSpeed());
    if (getSpeed() > getMaxSpeed()) {
        setSpeed(getMaxSpeed());
    }
}
