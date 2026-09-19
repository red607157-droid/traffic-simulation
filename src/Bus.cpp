#include "Bus.h"
#include <iostream>
using namespace std;

double Bus::getMaxSpeed() const {
    return 80;
}

void Bus::display() const {
    cout << "Bus no." << getId() << "\n"
         << "Current speed: " << getSpeed() << "\n"
         << "Fuel: " << getFuel() << "\n"
         << "Passenger: " << getPassenger() << "\n"
         << "Max speed: " << getMaxSpeed() << "\n";
}

void Bus::move(double dt) {
    if (dt < 0) { return; }
    double fuelConsumed = 0.5 * dt;
    setFuel(getFuel() - fuelConsumed);
    if (getFuel() < 0) { cout << "Het xang roi cuh\n"; return; }
    setSpeed(3 * dt + getSpeed());
    if (getSpeed() > getMaxSpeed()) {
        setSpeed(getMaxSpeed());
    }
}
