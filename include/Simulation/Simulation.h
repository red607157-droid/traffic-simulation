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
//
// QUY TẮC THỨ TỰ: thêm tất cả Road TRƯỚC, Intersection SAU.
//   Khi đó trong 1 bước: xe chạy xong -> nút giao mới đổi đèn và chuyển xe sang làn ra.
//   Nếu thêm Intersection trước thì xe vẫn chạy đúng nhưng được chuyển sang làn ra CHẬM HƠN 1 bước
//   (xem test "thu tu" trong tests/test_simulation.cpp). Simulation không tự sắp xếp lại.
//
// Gọi run() / runRealTime() nhiều lần: mô phỏng CHẠY TIẾP từ trạng thái hiện tại (time, stepCount cộng dồn), KHÔNG reset.
//
// Phase 10 - thời gian MÔ PHỎNG và thời gian THẬT là hai thứ khác nhau:
//   - time      : tăng đúng dt mỗi bước, bất kể máy nhanh hay chậm.
//   - speed     : 1 giây mô phỏng ứng với (1 / speed) giây thật. speed = 10 -> nhanh gấp 10 lần.
//   runRealTime() chỉ gọi step() và chờ tới thời điểm của tick kế tiếp, không tự update xe / đèn.
class Simulation {
private:
    double dt;          // độ dài một bước (giây mô phỏng), > 0
    double time;        // thời gian mô phỏng đã trôi qua (giây)
    int stepCount;      // số bước đã chạy
    std::vector<std::unique_ptr<RoadComponent>> components;

public:
    Simulation(double _dt);   // dt <= 0 ném invalid_argument

    void addComponent(std::unique_ptr<RoadComponent> c);   // thêm vào CUỐI thứ tự update; nullptr ném invalid_argument

    void step();               // chạy 1 bước, không in gì
    void run(int steps);       // in trạng thái hiện tại, rồi chạy TIẾP `steps` bước và in sau mỗi bước; steps < 0 ném invalid_argument
    // Phase 10: chạy TIẾP `simSeconds` giây mô phỏng theo thời gian thật, mỗi bước in 1 dòng tóm tắt.
    // simSeconds < 0 hoặc speed <= 0 ném invalid_argument.
    void runRealTime(double simSeconds, double speed = 1.0);
    void display() const;      // in thời gian và từng thành phần (chi tiết)
    void displaySummary() const;   // in 1 dòng: thời gian + số xe của từng thành phần

    double getTime() const;
    int getStepCount() const;
    int getVehicleCount() const;   // tổng số xe trong mọi thành phần
};

#endif
