#include "Vehicle/Bus.h"
#include <iostream>

// xe buýt dài và nặng -> giữ khoảng cách xa hơn (12m)
Bus::Bus(int _i, double _s, double _f, int _c) : Vehicle(_i, _s, _f, 12.0), capacity(_c) {}

int Bus::getCapacity() const { return capacity; }
void Bus::setCapacity(int _c) { capacity = _c; }

// Thông số vật lý của Bus (chậm, nặng: tăng/giảm tốc yếu hơn ô tô)
double Bus::getMaxSpeed() const { return 20.0; }      // 20 m/s (72 km/h)
double Bus::getAcceleration() const { return 2.0; }
double Bus::getDeceleration() const { return 4.0; }
double Bus::getFuelPerMeter() const { return 0.15; }  // tốn nhiên liệu hơn ô tô

void Bus::display() const {
    std::cout << "Bus no." << getId() << "\n"
              << "Capacity: " << getCapacity() << "\n";
    displayCommon();
}
