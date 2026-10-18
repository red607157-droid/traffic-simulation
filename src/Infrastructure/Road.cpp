#include <iostream>
#include <stdexcept>
#include <string>
#include "Infrastructure/Road.h"

using namespace std;

Road::Road(int _id, const string& _name, double _length)
    : RoadComponent(_id, _name), length(_length) {
    if (!(_length > 0.0)) throw invalid_argument("Road: chieu dai phai > 0");
}

Lane* Road::addLane(int laneId, double speedLimit) {
    for (const auto& l : lanes)
        if (l->getId() == laneId) return nullptr;   // id làn phải duy nhất trong 1 Road
    lanes.push_back(make_unique<Lane>(laneId, getName() + "-lane" + to_string(laneId), length, speedLimit));
    return lanes.back().get();
}

double Road::getLength() const { return length; }
size_t Road::getLaneCount() const { return lanes.size(); }
Lane* Road::getLane(size_t index) const { return index < lanes.size() ? lanes[index].get() : nullptr; }

void Road::update(double dt) {
    if (!(dt > 0.0)) return;
    for (auto& l : lanes) l->update(dt);
}

void Road::display() const {
    cout << "Road no." << getId() << " (" << getName() << ") - length: " << length
         << "m, lanes: " << lanes.size() << ", vehicles: " << getVehicleCount() << "\n";
    for (const auto& l : lanes) l->display();
}

int Road::getVehicleCount() const {
    int total = 0;
    for (const auto& l : lanes) total += l->getVehicleCount();
    return total;
}
