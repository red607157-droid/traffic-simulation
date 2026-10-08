#include <iostream>
#include <memory>
#include "Vehicle/States/Accelerating_State.h"
#include "Vehicle/Vehicle.h"
#include "Vehicle/States/Cruising_State.h"
#include "Vehicle/States/Braking_State.h"
#include "Vehicle/States/Stopped_State.h"
#include "Vehicle/States/WaitingAtLight_State.h"
#include <algorithm>

void Cruising::accelerate(Vehicle& vehicle){
    vehicle.setState(std::make_unique<Accelerating>());
}

void Cruising::brake(Vehicle& vehicle){
    std::cout<<"Dang dung..."<<"\n";
    vehicle.setState(std::make_unique<Braking>());
}

void Cruising::stop(Vehicle& vehicle){
    std::cout<<"Dang dung"<<'\n';
    vehicle.setState(std::make_unique<Braking>());
}

void Cruising::update(Vehicle& vehicle, double dt) {
    // kiểm tra va chạm với xe trước 
    if (vehicle.getDistanceToVehicleAhead() < vehicle.getDynamicSafeDistance()) {
        vehicle.setState(std::make_unique<Braking>());
        return;
    }

    // TÍNH TOÁN KHOẢNG CÁCH TỚI ĐÈN
    // Chỉ xe đầu hàng mới quan tâm đến đèn
    if (vehicle.getVehicleAhead() == nullptr) {
        
        // tính quãng đường cần thiết để phanh dừng hẳn từ vận tốc hiện tại
        double a = vehicle.getDeceleration();
        double v = vehicle.getSpeed();
        double brakingDistance = (a > 0) ? (v * v) / (2.0 * a) : 0.0;
        
        // cộng thêm một chút khoảng cách an toàn (ví dụ 5m) để không phanh chúi mũi vào vạch
        double reactionZone = brakingDistance + vehicle.getSafeDistance();

        // NẾU XE ĐÃ VÀO VÙNG CẦN PHANH, LÚC NÀY MỚI NHÌN ĐÈN
        if (vehicle.getDistanceToNextLight() <= reactionZone) {
            if (vehicle.getTrafficLight() == Vehicle::redLight || vehicle.getTrafficLight() == Vehicle::yellowLight) {
                vehicle.setState(std::make_unique<Braking>());
                return; 
            }
        }
    }

    // nếu chưa tới đèn hoặc đèn đang xanh, cứ đi tiếp
    vehicle.setSpeed(vehicle.getTargetSpeed());
    vehicle.setPosition(vehicle.getPosition() + vehicle.getSpeed() * dt);
}

const char* Cruising::getName() const { return "Cruising";}