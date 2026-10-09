#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Stopped_State.h"
#include "Vehicle/States/WaitingAtLight_State.h"
#include "Infrastructure/Lane.h"

namespace {
const double INF = std::numeric_limits<double>::infinity();

// Tốc độ lớn nhất v sao cho: đi v*dt trong tick này rồi phanh với gia tốc a vẫn dừng trong khoảng G:
//   v*dt + v^2/(2a) <= G   =>   v = a * ( sqrt(dt^2 + 2G/a) - dt )
double stopSpeed(double G, double a, double dt) {
    if (G <= 0.0 || a <= 0.0) return 0.0;
    return a * (std::sqrt(dt * dt + 2.0 * G / a) - dt);
}
}

Vehicle::Vehicle(int _i, double _s, double _fuel, double _safeDistance)
    : id(_i), speed(_s), position(0.0), targetSpeed(0.0), safeDistance(_safeDistance),
      fuel(_fuel > 0.0 ? _fuel : 0.0),
      vehicleAhead(nullptr), currentTrafficLight(greenLight), currentLane(nullptr),
      currentState(std::make_unique<Stopped>()), dispatchDepth(0) {}

// ---------- state ----------
void Vehicle::setState(std::unique_ptr<VehicleState> newState) { currentState = std::move(newState); }
VehicleState* Vehicle::getState() const { return currentState.get(); }

void Vehicle::transitionTo(std::unique_ptr<VehicleState> newState, double dt) {
    setState(std::move(newState));
    if (dispatchDepth < 4) {            // tối đa 4 lần chuyển liên tiếp / tick
        ++dispatchDepth;
        currentState->update(*this, dt);
        --dispatchDepth;
    }
}

void Vehicle::enterHaltState(double dt, bool runUpdateNow) {
    std::unique_ptr<VehicleState> next;
    if (isBlockedByLight(dt)) next = std::make_unique<WaitingAtLight>();
    else                      next = std::make_unique<Stopped>();
    if (runUpdateNow) transitionTo(std::move(next), dt);
    else              setState(std::move(next));
}

void Vehicle::accelerate() { currentState->accelerate(*this); }
void Vehicle::brake()      { currentState->brake(*this); }
void Vehicle::stop()       { currentState->stop(*this); }
void Vehicle::update(double dt) {
    if (!(dt > 0.0)) return;            // dt <= 0 (hoặc NaN): không làm gì
    currentState->update(*this, dt);
}

// ---------- getter / setter ----------
int Vehicle::getId() const                             { return id; }
double Vehicle::getSpeed() const                       { return speed; }
double Vehicle::getPosition() const                    { return position; }
double Vehicle::getTargetSpeed() const                 { return targetSpeed; }
Vehicle::trafficLight Vehicle::getTrafficLight() const { return currentTrafficLight; }
double Vehicle::getSafeDistance() const                { return safeDistance; }
double Vehicle::getFuel() const                        { return fuel; }
Vehicle* Vehicle::getVehicleAhead() const              { return vehicleAhead; }

void Vehicle::setSpeed(double _s)                      { speed = _s; }
void Vehicle::setPosition(double _p)                   { position = _p; }
void Vehicle::setTargetSpeed(double _ts)               { targetSpeed = _ts; }
void Vehicle::setTrafficLight(trafficLight _tl)        { currentTrafficLight = _tl; }
void Vehicle::setVehicleAhead(Vehicle* v)              { vehicleAhead = (v == this) ? nullptr : v; }
void Vehicle::setCurrentLane(Lane* lane)               { currentLane = lane; }
void Vehicle::setFuel(double _f)                       { fuel = (_f > 0.0) ? _f : 0.0; }

double Vehicle::getDistanceToNextLight() const {
    if (currentLane == nullptr) return NO_LIGHT;   // chưa gán làn đường: coi như không có đèn
    return currentLane->getDistanceToNextLight(position);
}

// ---------- vật lý / an toàn ----------
double Vehicle::getLeaderLimit(double dt) const {
    if (vehicleAhead == nullptr) return INF;
    double gap = vehicleAhead->position - position;
    if (gap < 0.0) return INF;   // xe "phía trước" thực ra đã ở phía sau mình -> bỏ qua

    // Xe trước cũng cần quãng phanh của nó -> được "cộng" vào khoảng trống khả dụng.
    double aL = vehicleAhead->getDeceleration();
    double leaderBrake = (aL > 0.0) ? (vehicleAhead->speed * vehicleAhead->speed) / (2.0 * aL) : 0.0;
    double brakingLimit = stopSpeed(gap - safeDistance + leaderBrake, getDeceleration(), dt);
    // Ngoài ra sau tick này vẫn phải còn >= safeDistance tới xe trước (không bao giờ sát bội / chồng lên nhau).
    double gapLimit = (dt > 0.0) ? std::max(gap - safeDistance, 0.0) / dt : 0.0;   // dt=0: tránh chia cho 0
    return std::min(brakingLimit, gapLimit);
}

double Vehicle::getLightLimit(double dt) const {
    if (currentTrafficLight == greenLight) return INF;
    double d = getDistanceToNextLight();
    if (d >= NO_LIGHT) return INF;   // không có đèn, hoặc đã qua vạch dừng

    double a = getDeceleration();
    if (currentTrafficLight == yellowLight) {
        // Đèn vàng: nếu có phanh gấp cũng không kịp dừng trước vạch thì đi tiếp (an toàn hơn dừng giữa ngã tư).
        double brakingDistance = (a > 0.0) ? (speed * speed) / (2.0 * a) : INF;
        if (brakingDistance > d + EPS) return INF;
    }
    // Đỏ hoặc unknownLight (an toàn: coi như đỏ) hoặc vàng mà còn kịp dừng: vạch dừng là vật cản đứng yên.
    return stopSpeed(d, a, dt);
}

bool Vehicle::isBlockedByLight(double dt) const {
    return getLightLimit(dt) <= SPEED_EPS && getLeaderLimit(dt) > SPEED_EPS;
}

double Vehicle::getDesiredSpeed(double dt) const {
    if (fuel <= 0.0) return 0.0;   // hết nhiên liệu -> không được phép chạy, phải phanh dừng
    double v = std::min(targetSpeed, getMaxSpeed());
    v = std::min(v, getLeaderLimit(dt));
    v = std::min(v, getLightLimit(dt));
    return std::max(v, 0.0);
}

void Vehicle::advance(double newSpeed, double dt) {
    if (!(dt > 0.0)) return;
    newSpeed = std::max(newSpeed, 0.0);
    double oldPos = position;
    double newPos = oldPos + newSpeed * dt;
    // Không bao giờ chồng lên / vượt xe trước, và cũng không bị kéo lùi.
    if (vehicleAhead != nullptr && vehicleAhead->position >= oldPos && newPos > vehicleAhead->position) {
        newPos = vehicleAhead->position;
        newSpeed = (newPos - oldPos) / dt;
    }
    speed = newSpeed;
    position = newPos;
    fuel = std::max(0.0, fuel - getFuelPerMeter() * (newPos - oldPos));   // trừ theo quãng đường thực tế đã đi
}

void Vehicle::displayCommon() const {
    std::cout << "State: " << getState()->getName() << "\n"
              << "Position: " << position << "\n"
              << "Current speed: " << speed << "\n"
              << "Fuel: " << fuel << "\n"
              << "Max speed: " << getMaxSpeed() << "\n";
}
