#ifndef SIMULATION_H
#define SIMULATION_H

#include <memory>
#include <vector>

#include "Vehicle/Vehicle.h"
#include "Traffic/TrafficLight.h"
#include "Infrastructure/Lane.h"

// Phase 2: bộ máy mô phỏng. Gom vòng lặp trước đây nằm trong main() vào một chỗ.
//
// Sở hữu:
//   - vehicles : Simulation SỞ HỮU (unique_ptr). Vehicle giữ con trỏ thô tới xe trước và tới lane,
//                nên các đối tượng đó phải sống ít nhất lâu bằng Simulation -> Simulation không cho copy/move.
//   - lane     : tạm thời là member (Lane hiện chỉ là bản tạm, sẽ đổi ở Phase 6).
//   - light    : có thể không có (nullptr) -> xe giữ tín hiệu mặc định (đèn xanh).
//
// Một bước (step) = dt giây thời gian MÔ PHỎNG (không liên quan thời gian thực):
//   1. đưa tín hiệu đèn hiện tại cho mọi xe
//   2. update xe theo thứ tự từ xe đầu hàng tới xe cuối hàng
//   3. update đèn
//   4. time += dt
class Simulation {
private:
    double dt;          // độ dài một bước (giây mô phỏng), > 0
    double time;        // thời gian mô phỏng đã trôi qua (giây)
    int stepCount;      // số bước đã chạy
    Lane lane;
    std::unique_ptr<TrafficLight> light;
    std::vector<std::unique_ptr<Vehicle>> vehicles;   // thứ tự = từ xe ĐẦU hàng tới xe CUỐI hàng

public:
    explicit Simulation(double _dt);   // dt <= 0 ném invalid_argument

    Simulation(const Simulation&) = delete;
    Simulation& operator=(const Simulation&) = delete;

    // Thêm xe vào CUỐI hàng: gán lane và nối xe trước (xe vừa thêm trước đó). nullptr ném invalid_argument.
    Vehicle& addVehicle(std::unique_ptr<Vehicle> v);
    void setTrafficLight(std::unique_ptr<TrafficLight> tl);

    void step();               // chạy 1 bước, không in gì
    void run(int steps);       // in trạng thái ban đầu, rồi chạy `steps` bước và in sau mỗi bước; steps < 0 ném invalid_argument
    void display() const;      // in trạng thái hiện tại (thời gian, đèn, các xe)

    double getDt() const;
    double getTime() const;
    int getStepCount() const;
    const TrafficLight* getTrafficLight() const;
    const std::vector<std::unique_ptr<Vehicle>>& getVehicles() const;
};

#endif
