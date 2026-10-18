// Demo RIÊNG cho lớp Simulation (main.cpp là demo chung của nhóm, không dùng Simulation).
//   Phase 2 : run()  - vòng lặp mô phỏng nằm trong Simulation, không nằm trong main()
//
// Build (PowerShell):
//   g++ -std=c++17 -Wall -Wextra -pthread -Iinclude (Get-ChildItem src -Recurse -Filter *.cpp).FullName demo/simulation_demo.cpp -o simulation-demo.exe; ./simulation-demo.exe
// Build (Git Bash / Linux / Mac): make demo

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

    Simulation sim(1.0);   // 1 bước = 1 giây mô phỏng
    sim.addComponent(move(roadA));   // Road trước, Intersection sau (xem Simulation.h)
    sim.addComponent(move(roadB));
    sim.addComponent(move(inter));

    cout << "\n##### Phase 2: run() #####\n";
    sim.run(3);   // in chi tiết từng bước (dài), nên chỉ chạy vài bước
    return 0;
}
