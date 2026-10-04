#include "Vehicle/Bus.h"
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


