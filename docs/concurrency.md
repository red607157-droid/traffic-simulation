# Đa luồng trong simulator (Phase 12–13)

Tài liệu này ghi lại **mô hình đa luồng** của simulator, **ai sở hữu / ai đọc / ai ghi** từng dữ liệu dùng chung, và **các quy tắc mọi người phải giữ** khi sửa code. Code liên quan: `include/Simulation/Simulation.h`, `src/Simulation/Simulation.cpp`.

## 1. Mô hình

Không tạo 1 thread cho mỗi xe / lane / đèn. Mỗi bước (`dt` giây mô phỏng) chia 2 giai đoạn:

```
Mỗi bước:
  GĐ 1 - XE (song song)     : các Road được chia cho tối đa N thread (N = min(maxWorkers, số Road)).
                              Thread w update Road w, w+N, w+2N, ...  -> mỗi Road thuộc đúng 1 thread.
  join                      : đợi mọi thread của GĐ 1 xong.
  GĐ 2 - NÚT GIAO (1 thread): update mọi Intersection: đổi đèn + chuyển xe từ làn vào sang làn ra.
  time += dt, stepCount++
```

Ngoài ra có thể chạy nền: `start(speed)` chạy vòng lặp thời gian thực trên **1 thread nền**; `main` chờ người dùng rồi gọi `stop()`.

Tổng số thread tối đa: `main` + 1 thread nền + N thread của GĐ 1 (N có giới hạn).

**Ý tưởng chính:** thay vì dùng mutex để bảo vệ dữ liệu dùng chung, thiết kế **tránh để 2 thread cùng chạm vào một dữ liệu trong cùng một lúc**:
- Trong GĐ 1, mỗi thread chỉ ghi vào các Road của mình; dữ liệu dùng chung duy nhất là đèn, và đèn **chỉ được đọc**.
- Mọi thao tác ghi "xuyên Road" (đổi đèn, chuyển xe giữa các lane) dồn vào GĐ 2, lúc đó chỉ có 1 thread chạy.
- `join()` ở cuối GĐ 1 đảm bảo GĐ 2 thấy đầy đủ kết quả của GĐ 1 (và ngược lại ở bước sau).

Hệ quả: kết quả **giống hệt** khi chạy 1 thread. Có test kiểm tra điều này (`tests/test_simulation.cpp`, test 14 và 15).

## 2. Dữ liệu dùng chung: ai sở hữu, ai đọc, ai ghi

| Dữ liệu | Ai sở hữu | Ai ghi (khi nào) | Ai đọc (khi nào) | Có thể bị 2 thread truy cập cùng lúc? | Cách đồng bộ |
|---|---|---|---|---|---|
| Xe trong một Lane (`vector<unique_ptr<Vehicle>>`, vị trí, tốc độ, state) | `Lane` | thread phụ trách Road chứa Lane đó (GĐ 1); thread mô phỏng khi chuyển xe (GĐ 2) | cùng các thread đó | Không | Tách theo giai đoạn + `join` |
| Danh sách Lane trong Road | `Road` | chỉ lúc dựng kịch bản (trước khi chạy) | thread phụ trách Road (GĐ 1) | Không (không đổi khi đang chạy) | Không cần |
| `TrafficLight` (state đèn) | `Intersection` | thread mô phỏng (GĐ 2) | thread của Road chứa làn có đèn (GĐ 1, qua `Lane::getLightSignal`) | Không (ghi và đọc ở 2 giai đoạn khác nhau) | Tách theo giai đoạn + `join` |
| Danh sách Road / Intersection trong `Simulation` | `Simulation` | `addRoad` / `addIntersection` | các thread GĐ 1 (chỉ đọc vector) | Không | `addRoad`/`addIntersection` bị chặn (`logic_error`) khi đang chạy nền |
| `time`, `stepCount` | `Simulation` | thread đang chạy mô phỏng | `main` có thể đọc bất kỳ lúc nào (`getTime`, `getStepCount`) | **Có** | `std::atomic` |
| Cờ `running` | `Simulation` | `main` (`stop()`), `start()` | thread nền | **Có** | `std::atomic<bool>` |
| `std::cout` | dùng chung | thread nền in mỗi bước; `main` in trước `start()` và sau `stop()` | – | Không, nếu giữ quy tắc in bên dưới | Chỉ 1 thread in tại một thời điểm |

Trong lúc chạy nền, `main` **chỉ** được gọi `stop()`, `isRunning()`, `getTime()`, `getStepCount()`. Các hàm khác (`step`, `run`, `display`, `getVehicleCount`, `addRoad`...) sẽ đọc/ghi xe và đèn mà thread nền đang dùng, nên chúng ném `logic_error` thay vì để xảy ra data race.

## 3. Quy tắc khi sửa code (cho cả nhóm)

Mô hình ở mục 1 chỉ đúng khi các quy tắc sau được giữ. **Code hiện tại đã giữ đủ** (đã rà soát ở Phase 13). Nếu cần phá một quy tắc, hãy báo người làm Phase 12–13 trước, vì có thể phải thêm mutex hoặc đổi mô hình.

**Vehicle và các state (`Vehicle::update`, `*_State::update`):**
1. Chỉ được đọc **xe phía trước trong cùng lane** (`vehicleAhead`) và **lane của chính nó** (`currentLane`). Không đọc xe ở lane hay road khác. (Tính năng chuyển làn / vượt xe sẽ phá quy tắc này.)
2. Không in (`cout`) trong đường đi của `update()`. Hiện tại các dòng in chỉ nằm trong `accelerate/brake/stop` (lệnh gọi tay), không nằm trong `update()`.
3. Không dùng biến `static` / biến toàn cục có thể thay đổi.

**Lane / Road:**
4. `Lane::update` và `Road::update` chỉ được động vào xe của **chính lane / road đó**.
5. `Lane` chỉ **đọc** đèn (`const TrafficLight*`), không được gọi `update()` của đèn.
6. Một Lane chỉ thuộc **một** Road.

**Intersection / TrafficLight:**
7. Chỉ `Intersection::update` được đổi đèn và chuyển xe giữa các lane (chạy ở GĐ 2).
8. Không dựng thêm Road / Intersection khi simulation đang chạy nền.

**Chung:**
9. Các hàm `update()` không được ném exception. Exception thoát ra khỏi một thread GĐ 1 sẽ làm chương trình dừng (`std::terminate`). Hiện tại chỉ constructor mới ném exception.

## 4. Kiểm tra

- **Test (chạy được trên mọi máy):** `tests/test_simulation.cpp`
  - test 13: số thread GĐ 1 không vượt `maxWorkers` và số Road;
  - test 14: 4 thread cho kết quả giống hệt 1 thread ở mọi bước (lặp 20 lần);
  - test 15: `start()`/`stop()`, các hàm bị chặn khi đang chạy nền, kết quả chạy nền giống chạy tuần tự;
  - test 16: huỷ `Simulation` khi đang chạy nền không làm chương trình dừng đột ngột.
- **ThreadSanitizer (chỉ Linux/Mac):** `make test-simulation-tsan`. TSan báo chính xác dòng nào có data race. Trên Windows/MinGW không có TSan; cần chạy trên Linux, Mac hoặc WSL.

## 5. Giới hạn đã biết

- GĐ 1 tạo thread mới ở **mỗi bước**. Đơn giản và số thread luôn có giới hạn, nhưng tốn thêm thời gian tạo thread. Với vài Road thì không đáng kể; nếu sau này có rất nhiều bước / Road, có thể đổi sang thread pool cố định.
- Song song theo **Road**: kịch bản chỉ có 1–2 Road thì gần như không nhanh hơn. Mục tiêu ở đây là đúng và an toàn, không phải tốc độ.
- `stop()` có thể phải chờ tới hết tick hiện tại (tối đa `dt / speed` giây thật) mới dừng.
