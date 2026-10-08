#include "Vehicle/Motorbike.h"
#include <iostream>

Motorbike::Motorbike(int _i, double _s, int _e, double _f)
    : Vehicle(_i, _s), engineCC(_e), fuel(_f) {
    this->safeDistance = 5.0; // Xe máy nhỏ gọn, bám đuôi gần hơn ô tô (5m so với 15m)
}

void Motorbike::setEngineCC(int _e) { engineCC = _e; }
int Motorbike::getEngineCC() const { return engineCC; }
void Motorbike::setFuel(double _f) { fuel = _f; }
double Motorbike::getFuel() const { return fuel; }

// Thông số vật lý của Motorbike
double Motorbike::getMaxSpeed() const { return 20.0; }      // Khoảng 72 km/h
double Motorbike::getAcceleration() const { return 6.0; }   // Gia tốc cao: 6 m/s^2 (bốc hơn ô tô)
double Motorbike::getDeceleration() const { return 8.0; }   // Phanh gấp: 8 m/s^2

void Motorbike::display() const {
    std::cout << "Motorbike no." << getId() << "\n"
              << "Current speed: " << getSpeed() << "\n"
              << "EngineCC: " << getEngineCC() << "\n"
              << "Fuel: " << getFuel() << "\n"
              << "Max speed: " << getMaxSpeed() << "\n";
}