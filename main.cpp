#include <iostream>
#include <memory>

#include "Simulation/Simulation.h"
#include "Infrastructure/Road.h"
#include "Infrastructure/Intersection.h"
#include "Vehicle/Car.h"
#include "Vehicle/Bus.h"
#include "Vehicle/Motorbike.h"

using namespace std;

int main() {
    // Kịch bản: Road A (500m) -> nút giao có đèn -> Road B (500m)
    auto roadA = make_unique<Road>(1, "A", 500.0);
    auto roadB = make_unique<Road>(2, "B", 500.0);
    Lane* laneA = roadA->addLane(1);
    Lane* laneB = roadB->addLane(2);

    // Đặt targetSpeed TRƯỚC khi move() giao xe cho Lane.
    auto car  = make_unique<Car>(1, 0, 100);
    auto bus  = make_unique<Bus>(2, 0, 200, 30);
    auto bike = make_unique<Motorbike>(3, 0, 125, 30);
    car->setTargetSpeed(20);
    bus->setTargetSpeed(15);
    bike->setTargetSpeed(18);
    laneA->addVehicle(move(car),  400);
    laneA->addVehicle(move(bus),  370);
    laneA->addVehicle(move(bike), 350);

    // Đèn ở cuối làn A: RED=12s, GREEN=10s, YELLOW=3s; qua đèn thì xe sang làn B.
    auto inter = make_unique<Intersection>(1, "I1");
    inter->addApproach(laneA, laneB, 12, 10, 3);

    Simulation sim(1.0);   // 1 bước = 1 giây mô phỏng; số thread cho xe = theo số nhân CPU
    sim.addRoad(move(roadA));          // Simulation tự update Road trước, Intersection sau
    sim.addRoad(move(roadB));
    sim.addIntersection(move(inter));

    cout << "Nhan Enter de dung mo phong...\n";   // in TRƯỚC start(): sau đó chỉ thread nền in
    sim.start(2.0);   // chạy thời gian thực trên 1 thread nền, nhanh gấp 2 lần
    cin.get();        // main chờ người dùng
    sim.stop();       // báo dừng (atomic<bool>) rồi join thread nền
    cout << "Da dung o t=" << sim.getTime() << "s\n";
    return 0;
}
