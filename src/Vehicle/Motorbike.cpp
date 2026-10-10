#include "Vehicle/Motorbike.h"
#include <iostream>

// Xe máy nhỏ gọn, bám đuôi gần hơn ô tô (5m so với 10m)
Motorbike::Motorbike(int _i, double _s, int _e, double _f)
    : Vehicle(_i, _s, _f, 5.0), engineCC(_e) {}

void Motorbike::setEngineCC(int _e) { engineCC = _e; }
int Motorbike::getEngineCC() const { return engineCC; }

// Thông số vật lý của Motorbike
double Motorbike::getMaxSpeed() const { return 20.0; }      // Khoảng 72 km/h
double Motorbike::getAcceleration() const { return 6.0; }   // Gia tốc cao: 6 m/s^2 (bốc hơn ô tô)
double Motorbike::getDeceleration() const { return 8.0; }   // Phanh gấp: 8 m/s^2
double Motorbike::getFuelPerMeter() const { return 0.04; }  // tiết kiệm nhất

void Motorbike::display() const {
    std::cout << "Motorbike no." << getId() << "\n"
              << "EngineCC: " << getEngineCC() << "\n";
    displayCommon();
}
