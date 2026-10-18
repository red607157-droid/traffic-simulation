#include <algorithm>
#include <iostream>
#include <stdexcept>
#include "Infrastructure/Lane.h"
#include "Integration/LightMapper.h"

using namespace std;

Lane::Lane(int _id, const string& _name, double _length, double _speedLimit)
    : RoadComponent(_id, _name), length(_length), speedLimit(_speedLimit), light(nullptr) {
    if (!(_length > 0.0))   throw invalid_argument("Lane: chieu dai phai > 0");
    if (!(_speedLimit >= 0.0)) throw invalid_argument("Lane: gioi han toc do phai >= 0");
}

// ---------- nội bộ ----------
void Lane::refreshOrder() {
    // Xe đầu đoàn (position lớn nhất) ở index 0. stable_sort giữ nguyên thứ tự khi 2 xe bằng vị trí.
    stable_sort(vehicles.begin(), vehicles.end(),
                [](const unique_ptr<Vehicle>& a, const unique_ptr<Vehicle>& b) {
                    return a->getPosition() > b->getPosition();
                });
    for (size_t i = 0; i < vehicles.size(); ++i)
        vehicles[i]->setVehicleAhead(i == 0 ? nullptr : vehicles[i - 1].get());
}

// ---------- đèn ----------
void Lane::setTrafficLight(const TrafficLight* _light) { light = _light; }
bool Lane::hasTrafficLight() const { return light != nullptr; }

// ---------- quản lý xe ----------
void Lane::addVehicle(unique_ptr<Vehicle> v, double entryPosition) {
    if (!v) return;
    v->setPosition(max(entryPosition, 0.0));
    v->setCurrentLane(this);
    vehicles.push_back(move(v));
    refreshOrder();
}

bool Lane::hasExitedVehicle() const {
    return !vehicles.empty() && vehicles.front()->getPosition() > length;
}

unique_ptr<Vehicle> Lane::takeExitedVehicle() {
    if (!hasExitedVehicle()) return nullptr;
    unique_ptr<Vehicle> v = move(vehicles.front());
    vehicles.erase(vehicles.begin());
    v->setCurrentLane(nullptr);
    v->setVehicleAhead(nullptr);
    refreshOrder();   // xe phía sau không được giữ con trỏ tới xe vừa rời làn
    return v;
}

// ---------- truy vấn cho Vehicle ----------
double Lane::getDistanceToNextLight(double position) const {
    if (light == nullptr) return Vehicle::NO_LIGHT;
    double d = length - position;
    if (d < 0.0) return Vehicle::NO_LIGHT;   // đã qua vạch dừng
    return d;
}

Vehicle::trafficLight Lane::getLightSignal() const {
    if (light == nullptr) return Vehicle::unknownLight;
    return LightMapper(*light);
}

// ---------- getter ----------
double Lane::getLength() const { return length; }
double Lane::getSpeedLimit() const { return speedLimit; }

Vehicle* Lane::getVehicleAt(size_t index) const {
    return index < vehicles.size() ? vehicles[index].get() : nullptr;
}
Vehicle* Lane::getFrontVehicle() const { return vehicles.empty() ? nullptr : vehicles.front().get(); }
Vehicle* Lane::getRearVehicle() const  { return vehicles.empty() ? nullptr : vehicles.back().get(); }

bool Lane::hasRoomAtEntry(double minGap) const {
    if (vehicles.empty()) return true;
    return vehicles.back()->getPosition() >= minGap;
}

// ---------- RoadComponent ----------
void Lane::update(double dt) {
    if (!(dt > 0.0)) return;
    refreshOrder();
    Vehicle::trafficLight signal = getLightSignal();

    // Duyệt từ đầu đoàn về cuối đoàn: xe trước chạy xong thì xe sau mới tính khoảng cách với vị trí mới của nó.
    for (auto& v : vehicles) {
        if (v->getPosition() > length) {   // đã ra khỏi làn nhưng chưa được chuyển đi (làn ra đầy): đứng chờ
            v->setSpeed(0.0);
            continue;
        }
        v->setTrafficLight(signal);
        double originalTarget = v->getTargetSpeed();
        bool clamped = (speedLimit > 0.0 && originalTarget > speedLimit);
        if (clamped) v->setTargetSpeed(speedLimit);
 
        v->update(dt);
 
        if (clamped) v->setTargetSpeed(originalTarget);
    }
    refreshOrder();
}

void Lane::display() const {
    cout << "Lane no." << getId() << " (" << getName() << ") - length: " << length
         << "m, speed limit: " << (speedLimit > 0.0 ? to_string(speedLimit) + " m/s" : string("none"))
         << ", vehicles: " << vehicles.size() << "\n";
    if (light != nullptr) { cout << "  "; light->display(); }
    for (const auto& v : vehicles) v->display();
}

int Lane::getVehicleCount() const { return static_cast<int>(vehicles.size()); }
