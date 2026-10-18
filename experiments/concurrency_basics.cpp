// Phase 11: thí nghiệm đa luồng nhỏ, KHÔNG dùng code của simulator.
// Giả sử có 2 làn đường, mỗi làn chạy trên 1 thread và đếm số xe đi qua giao lộ.
//
// Build: g++ -std=c++17 -Wall -Wextra -pthread experiments/concurrency_basics.cpp -o concurrency-basics
// (không bật -O2: compiler sẽ gộp vòng lặp lại và race condition ở phần 2 không lộ ra nữa)

#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

const int CARS = 1000000;   // số xe mỗi làn

// ---------- Phần 1: 2 thread, mỗi thread làm việc riêng ----------
int lane1Count = 0;   // chỉ thread A ghi
int lane2Count = 0;   // chỉ thread B ghi

void countLane1() { for (int i = 0; i < CARS; i++) lane1Count++; }
void countLane2() { for (int i = 0; i < CARS; i++) lane2Count++; }

// ---------- Phần 2: 2 thread cùng ghi 1 biến, KHÔNG khoá ----------
int unsafeTotal = 0;

void countUnsafe() {
    for (int i = 0; i < CARS; i++) unsafeTotal++;   // đọc -> cộng 1 -> ghi: 2 thread có thể chen nhau
}

// ---------- Phần 3: có khoá bằng mutex ----------
int safeTotal = 0;
mutex totalMutex;   // bảo vệ safeTotal

void countWithMutex() {
    for (int i = 0; i < CARS; i++) {
        lock_guard<mutex> lock(totalMutex);   // khoá; tự mở khoá khi hết vòng lặp
        safeTotal++;                          // critical section: mỗi lúc chỉ 1 thread vào được
    }
}

// ---------- Phần 4: dùng atomic ----------
atomic<int> atomicTotal(0);   // ++ trên atomic không bị thread khác chen ngang

void countWithAtomic() {
    for (int i = 0; i < CARS; i++) atomicTotal++;
}

int main() {
    cout << "Phan 1: 2 thread lam 2 viec rieng\n";
    thread a(countLane1);   // thread chạy NGAY khi được tạo, song song với main
    thread b(countLane2);
    a.join();               // main đợi a chạy xong. Quên join -> chương trình bị dừng (std::terminate)
    b.join();
    cout << "  lan 1 = " << lane1Count << ", lan 2 = " << lane2Count << "\n";

    cout << "Phan 2: chung 1 bien, khong khoa\n";
    thread c(countUnsafe);
    thread d(countUnsafe);
    c.join();
    d.join();
    cout << "  tong = " << unsafeTotal << " (dung ra phai la " << 2 * CARS << ") -> race condition\n";

    cout << "Phan 3: chung 1 bien, co mutex\n";
    thread e(countWithMutex);
    thread f(countWithMutex);
    e.join();
    f.join();
    cout << "  tong = " << safeTotal << "\n";

    cout << "Phan 4: chung 1 bien, dung atomic\n";
    thread g(countWithAtomic);
    thread h(countWithAtomic);
    g.join();
    h.join();
    cout << "  tong = " << atomicTotal << "\n";
    return 0;
}
