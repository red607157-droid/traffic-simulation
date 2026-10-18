#ifndef SIMULATION_H
#define SIMULATION_H

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include "Infrastructure/Road.h"
#include "Infrastructure/Intersection.h"

// Phase 2: bộ máy mô phỏng. Gom vòng lặp trước đây nằm trong main() vào một chỗ.
//
// Sở hữu: Simulation SỞ HỮU các Road (Road sở hữu Lane, Lane sở hữu xe) và các Intersection (sở hữu đèn).
//
// Một bước (step) = dt giây thời gian MÔ PHỎNG, chia 2 giai đoạn (Phase 12):
//   GĐ 1 - XE       : update mọi Road, SONG SONG trên tối đa maxWorkers thread (không phải 1 thread / xe).
//                     Mỗi Road chỉ do đúng 1 thread ghi; xe chỉ đọc xe trước trong CÙNG lane và đèn (chỉ đọc).
//   join            : đợi mọi thread của GĐ 1 xong.
//   GĐ 2 - NÚT GIAO : update mọi Intersection trên thread hiện tại (đổi đèn + chuyển xe giữa các lane).
//   -> không có 2 thread nào cùng ghi một dữ liệu, nên không cần mutex. Chi tiết: docs/concurrency.md
//   Kết quả giống hệt khi chạy 1 thread (xem test "song song = tuan tu").
//   Road luôn chạy trước Intersection, bất kể thứ tự thêm vào.
//
// Phase 10 - thời gian MÔ PHỎNG và thời gian THẬT là hai thứ khác nhau:
//   - time  : tăng đúng dt mỗi bước, bất kể máy nhanh hay chậm.
//   - speed : 1 giây mô phỏng ứng với (1 / speed) giây thật. speed = 10 -> nhanh gấp 10 lần.
//
// Chạy nền (Phase 12): start(speed) chạy vòng lặp thời gian thực trên 1 thread nền cho tới khi stop().
//   Trong lúc chạy nền, chỉ được gọi stop(), isRunning(), getTime(), getStepCount() (đều an toàn giữa các thread).
//   Các hàm khác ném logic_error vì sẽ đọc/ghi xe và đèn mà thread nền đang dùng.
//
// Gọi run() / runRealTime() nhiều lần: mô phỏng CHẠY TIẾP từ trạng thái hiện tại, KHÔNG reset.
class Simulation {
private:
    double dt;                       // độ dài một bước (giây mô phỏng), > 0
    std::atomic<double> time;        // atomic: thread nền ghi, main có thể đọc trong lúc chạy
    std::atomic<int> stepCount;
    unsigned maxWorkers;             // số thread tối đa cho GĐ 1
    std::vector<std::unique_ptr<Road>> roads;
    std::vector<std::unique_ptr<Intersection>> intersections;
    std::atomic<bool> running;       // main ghi (stop), thread nền đọc
    std::thread worker;              // thread nền của start()

    void doStep();                                // 1 bước (GĐ 1 + GĐ 2), dùng chung cho mọi cách chạy
    void updateRoads();                           // GĐ 1
    std::chrono::steady_clock::duration tickFor(double speed) const;   // độ dài 1 tick thật; speed sai ném invalid_argument
    void realTimeLoop(int maxSteps, std::chrono::steady_clock::duration tick);   // maxSteps < 0: chạy tới khi stop()
    void printSummary() const;
    void requireStopped() const;                  // ném logic_error nếu đang chạy nền

public:
    // maxWorkers = 0: lấy theo số nhân CPU. dt <= 0 hoặc không hữu hạn (inf, NaN) ném invalid_argument.
    Simulation(double _dt, unsigned _maxWorkers = 0);
    ~Simulation();   // tự gọi stop(): không bao giờ huỷ std::thread khi chưa join (sẽ bị std::terminate)

    void addRoad(std::unique_ptr<Road> r);                    // nullptr ném invalid_argument
    void addIntersection(std::unique_ptr<Intersection> i);    // nullptr ném invalid_argument

    void step();                                              // chạy 1 bước, không in gì
    void run(int steps);                                      // in chi tiết sau mỗi bước; steps < 0 ném invalid_argument
    // Phase 10: chạy đúng simSeconds / dt bước, nên simSeconds phải là bội số của dt (vd. dt = 1 thì 2.5 bị từ chối).
    // Ném invalid_argument nếu: simSeconds < 0, không hữu hạn, không phải bội số của dt, hoặc quá lớn (số bước > INT_MAX);
    // speed <= 0, không hữu hạn, quá lớn (1 tick thật bị làm tròn về 0) hoặc quá nhỏ (1 tick thật > 1 giờ).
    void runRealTime(double simSeconds, double speed = 1.0);

    void start(double speed = 1.0);   // Phase 12: chạy nền; đang chạy rồi -> logic_error; speed sai (như runRealTime) -> invalid_argument
    void stop();                      // báo dừng rồi join thread nền; gọi nhiều lần không sao
    bool isRunning() const;

    void display() const;          // in chi tiết từng Road và Intersection
    void displaySummary() const;   // in 1 dòng: thời gian, số xe mỗi Road, màu đèn mỗi nút giao

    double getTime() const;
    int getStepCount() const;
    int getVehicleCount() const;   // tổng số xe trên mọi Road
    unsigned getWorkerCount() const;   // số thread thực sự dùng cho GĐ 1 = min(maxWorkers, số Road)
};

#endif
