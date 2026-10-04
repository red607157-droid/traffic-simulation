#include "Vehicle/Motorbike.h"
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

