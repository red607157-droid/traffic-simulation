#include "Vehicle/Car.h"
#include <iostream>

// Khoảng cách an toàn cơ bản cho ô tô là 10m
Car::Car(int _i, double _s, double _f) : Vehicle(_i, _s, _f, 10.0) {}

// Thông số vật lý của Car
double Car::getMaxSpeed() const { return 25.0; }      // Tối đa 25 m/s (khoảng 90 km/h)
double Car::getAcceleration() const { return 3.0; }   // Gia tốc: 3 m/s^2
double Car::getDeceleration() const { return 5.0; }   // Phanh: 5 m/s^2
double Car::getFuelPerMeter() const { return 0.08; }  // 0.08 đơn vị nhiên liệu / mét

void Car::display() const {
    std::cout << "Car no." << getId() << "\n";
    displayCommon();
}
