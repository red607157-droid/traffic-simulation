// Thu thuc hanh GitHub
// Demo RIÊNG cho lớp Simulation (main.cpp là demo chung của nhóm, không dùng Simulation).
//   Phase 2 : run()          - vòng lặp mô phỏng nằm trong Simulation, không nằm trong main()
//   Phase 10: runRealTime()  - tự chạy theo thời gian thật, không cần gọi từng bước
//   Phase 12: start()/stop() - mô phỏng chạy trên thread nền (các Road update song song),
//                              main làm việc khác cùng lúc rồi dừng thread gọn gàng
//
// Build (PowerShell):
//   g++ -std=c++17 -Wall -Wextra -pthread -Iinclude (Get-ChildItem src -Recurse -Filter *.cpp).FullName demo/simulation_demo.cpp -o simulation-demo.exe; ./simulation-demo.exe
// Build (Git Bash / Linux / Mac): make demo

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

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

    cout << "\n##### Phase 2: run() #####\n";
    sim.run(3);   // in chi tiết từng bước (dài), nên chỉ chạy vài bước

    cout << "\n##### Phase 10: runRealTime() #####\n";
    sim.runRealTime(30, 10.0);   // chạy TIẾP 30 giây mô phỏng, nhanh gấp 10 lần -> khoảng 3 giây thật

    cout << "\n##### Phase 12: start() / stop() #####\n";
    cout << "So thread cho xe moi buoc: " << sim.getWorkerCount() << " (mot thread / Road, khong phai mot thread / xe)\n";
    sim.start(10.0);   // từ đây chỉ thread nền in ra; main KHÔNG in trong lúc đang chạy
    this_thread::sleep_for(chrono::seconds(2));   // main làm "việc khác" trong 2 giây
    sim.stop();        // báo dừng (atomic<bool>) rồi join thread nền
    cout << "Main da dung thread nen o t=" << sim.getTime() << "s, buoc " << sim.getStepCount() << "\n";
    return 0;
}
