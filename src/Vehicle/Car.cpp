#include "Car.h"
#include <iostream>
using namespace std;

double Car::getMaxSpeed() const {
    return 120;
}

void Car::display() const {
    cout << "Car no." << getId() << "\n"
         << "Current speed: " << getSpeed() << "\n"
         << "Fuel: " << getFuel() << "\n"
         << "Max speed: " << getMaxSpeed() << "\n";
}

void Car::move(double dt) {
    if (dt < 0) { return; }
    double fuelConsumed = 0.4 * dt;
    setFuel(getFuel() - fuelConsumed);
    if (getFuel() < 0) { cout << "Het xang roi cuh\n"; return; }
    setSpeed(5 * dt + getSpeed());
    if (getSpeed() > getMaxSpeed()) {
        setSpeed(getMaxSpeed());
    }
}
