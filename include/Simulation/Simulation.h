#ifndef SIMULATION_H
#define SIMULATION_H

#include <memory>
#include <vector>

#include "Infrastructure/RoadComponent.h"

// Phase 2: bộ máy mô phỏng. Gom vòng lặp trước đây nằm trong main() vào một chỗ.
//
// Sở hữu: Simulation SỞ HỮU các thành phần hạ tầng (Road, Intersection, ...) qua RoadComponent.
//   Xe do Lane sở hữu, đèn do Intersection sở hữu -> Simulation không cần biết tới xe hay đèn.
//
// Một bước (step) = dt giây thời gian MÔ PHỎNG (không liên quan thời gian thực):
//   gọi update(dt) cho từng thành phần theo THỨ TỰ ĐÃ THÊM, rồi time += dt.
//   -> thêm Road trước, Intersection sau (xe chạy xong rồi nút giao mới đổi đèn / chuyển xe).
class Simulation {
private:
    double dt;          // độ dài một bước (giây mô phỏng), > 0
    double time;        // thời gian mô phỏng đã trôi qua (giây)
    int stepCount;      // số bước đã chạy
    std::vector<std::unique_ptr<RoadComponent>> components;

public:
    Simulation(double _dt);   // dt <= 0 ném invalid_argument

    void addComponent(std::unique_ptr<RoadComponent> c);   // nullptr ném invalid_argument

    void step();               // chạy 1 bước, không in gì
    void run(int steps);       // in trạng thái ban đầu, rồi chạy `steps` bước và in sau mỗi bước; steps < 0 ném invalid_argument
    void display() const;      // in thời gian và từng thành phần

    double getTime() const;
    int getStepCount() const;
    int getVehicleCount() const;   // tổng số xe trong mọi thành phần
};

#endif
