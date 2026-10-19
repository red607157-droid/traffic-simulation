#ifndef SIMULATION_H
#define SIMULATION_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
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
//   Vì sao giữ Road và Intersection ở 2 danh sách riêng (không gọi chung qua RoadComponent)?
//   -> 2 loại cần cách chạy khác nhau: Road chạy song song, Intersection phải chạy 1 thread sau khi Road xong.
//      Composite vẫn dùng BÊN TRONG Road (Road -> Lane) và giữ nguyên giao diện RoadComponent (xem docs/concurrency.md).
//   Exception ném ra trong thread của GĐ 1 được bắt lại, join đủ các thread rồi ném tiếp ở thread gọi step().
//
// Phase 10 - thời gian MÔ PHỎNG và thời gian THẬT là hai thứ khác nhau:
//   - time  : tăng đúng dt mỗi bước, bất kể máy nhanh hay chậm.
//   - speed : 1 giây mô phỏng ứng với (1 / speed) giây thật. speed = 10 -> nhanh gấp 10 lần.
//
// Chạy nền (Phase 12): start(speed) chạy vòng lặp thời gian thực trên 1 thread nền cho tới khi stop().
//   Trong lúc chạy nền, chỉ được gọi stop(), isRunning(), getTime(), getStepCount() (đều an toàn giữa các thread).
//   Các hàm khác ném logic_error vì sẽ đọc/ghi xe và đèn mà thread nền đang dùng.
//   stop() dừng NGAY (không phải chờ hết tick hiện tại): thread nền chờ bằng condition_variable, stop() đánh thức nó.
//   Nếu thread nền gặp exception: nó tự dừng (isRunning() = false), exception được giữ lại và stop() ném lại.
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
    std::mutex stopMutex;            // đi cùng stopCv: stop() đổi `running` trong lúc giữ khoá để không lỡ lần đánh thức
    std::condition_variable stopCv;  // thread nền chờ tới tick kế tiếp trên đây; stop() đánh thức ngay
    std::exception_ptr workerError;  // exception của thread nền; thread nền ghi, main chỉ đọc SAU khi join

    void doStep();                                // 1 bước (GĐ 1 + GĐ 2), dùng chung cho mọi cách chạy
    void updateRoads();                           // GĐ 1
    std::chrono::steady_clock::duration tickFor(double speed) const;   // độ dài 1 tick thật; speed sai ném invalid_argument
    void realTimeLoop(int maxSteps, std::chrono::steady_clock::duration tick);   // maxSteps < 0: chạy tới khi stop()
    void printSummary() const;
    void requireStopped() const;                  // ném logic_error nếu đang chạy nền
    std::exception_ptr shutdownWorker();          // báo dừng + join thread nền; trả về exception của nó (nếu có)

public:
    // maxWorkers = 0: lấy theo số nhân CPU (không biết thì dùng 1). dt <= 0 hoặc không hữu hạn (inf, NaN) ném invalid_argument.
    explicit Simulation(double _dt, unsigned _maxWorkers = 0);
    ~Simulation();   // tự dừng + join thread nền (không bao giờ huỷ std::thread khi chưa join); không ném exception

    void addRoad(std::unique_ptr<Road> r);                    // nullptr ném invalid_argument
    void addIntersection(std::unique_ptr<Intersection> i);    // nullptr ném invalid_argument

    void step();                                              // chạy 1 bước, không in gì
    void run(int steps);                                      // in chi tiết sau mỗi bước; steps < 0 ném invalid_argument
    // Phase 10: chạy đúng simSeconds / dt bước, nên simSeconds phải là bội số của dt (vd. dt = 1 thì 2.5 bị từ chối).
    // Ném invalid_argument nếu: simSeconds < 0, không hữu hạn, không phải bội số của dt, hoặc quá lớn (số bước > INT_MAX);
    // speed <= 0, không hữu hạn, quá lớn (1 tick thật bị làm tròn về 0) hoặc quá nhỏ (1 tick thật > 1 giờ).
    void runRealTime(double simSeconds, double speed = 1.0);

    // Phase 12: chạy nền. speed sai (như runRealTime) -> invalid_argument, kiểm tra TRƯỚC khi tạo thread.
    // Đang chạy, hoặc thread nền đã dừng vì lỗi mà chưa gọi stop() -> logic_error.
    void start(double speed = 1.0);
    void stop();                      // dừng ngay + join thread nền; ném lại exception của thread nền (nếu có); gọi nhiều lần không sao
    bool isRunning() const;

    void display() const;          // in chi tiết từng Road và Intersection
    void displaySummary() const;   // in 1 dòng: thời gian, số xe mỗi Road, màu đèn mỗi nút giao

    double getTime() const;
    int getStepCount() const;
    int getVehicleCount() const;   // tổng số xe trên mọi Road
    unsigned getWorkerCount() const;   // số thread thực sự dùng cho GĐ 1 = min(maxWorkers, số Road)
};

#endif
