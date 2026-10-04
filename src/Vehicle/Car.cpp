#include "Vehicle/Car.h"
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


