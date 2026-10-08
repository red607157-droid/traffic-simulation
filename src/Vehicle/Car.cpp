#include "Vehicle/Car.h"
#include <iostream>

Car::Car(int _i, double _s, double _f) : Vehicle(_i, _s), fuel(_f) {
    this->safeDistance = 10.0; // Khoảng cách tĩnh cơ bản cho ô tô là 15m
}

double Car::getFuel() const { return fuel; }
void Car::setFuel(double _f) { fuel = _f; }

// Thông số vật lý của Car
double Car::getMaxSpeed() const { return 25.0; }      // Tối đa 25 m/s (khoảng 90 km/h)
double Car::getAcceleration() const { return 3.0; }   // Gia tốc: 3 m/s^2
double Car::getDeceleration() const { return 5.0; }   // Phanh: 5 m/s^2

void Car::display() const {
    std::cout << "Car no." << getId() << "\n"
              << "Current speed: " << getSpeed() << "\n"
              << "Fuel: " << getFuel() << "\n"
              << "Max speed: " << getMaxSpeed() << "\n";
}