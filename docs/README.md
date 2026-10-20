# Urban Traffic Simulation

Mô phỏng giao thông đô thị chạy trên console, viết bằng C++17. Xe (ô tô, xe buýt, xe máy) chạy trên các làn, gặp đèn giao thông ở nút giao, tự giữ khoảng cách an toàn với xe trước, dừng khi đèn đỏ và chạy lại khi đèn xanh. Mô phỏng chạy theo bước thời gian cố định, có thể chạy theo thời gian thật trên thread nền.

Dự án minh hoạ: lập trình hướng đối tượng, kế thừa và đa hình, **State Pattern**, **Composite Pattern**, đa luồng có kiểm soát, quản lý sở hữu bằng smart pointer.

## Mục lục
- [Tổng quan kiến trúc](#tổng-quan-kiến-trúc)
- [OOP](#oop)
- [State Pattern](#state-pattern)
- [Composite Pattern](#composite-pattern)
- [Đa luồng](#đa-luồng)
- [Build](#build)
- [Chạy](#chạy)
- [Ví dụ output](#ví-dụ-output)
- [Kiểm thử](#kiểm-thử)
- [Cấu trúc thư mục](#cấu-trúc-thư-mục)
- [Giới hạn đã biết](#giới-hạn-đã-biết)

## Tổng quan kiến trúc

```
Simulation ─┬─ Road ── Lane ── Vehicle (Car | Bus | Motorbike) ── VehicleState
            └─ Intersection ── TrafficLight ── TrafficLightState (Red | Green | Yellow)
```

| Lớp | Trách nhiệm |
|---|---|
| `Vehicle`, `Car`, `Bus`, `Motorbike` | Dữ liệu và vật lý của xe (vị trí, tốc độ, nhiên liệu); mỗi loại có thông số riêng |
| `VehicleState` và 5 lớp con | Hành vi của xe theo trạng thái: `Stopped`, `Accelerating`, `Cruising`, `Braking`, `WaitingAtLight` |
| `TrafficLight`, `TrafficLightState` và `RedState`/`GreenState`/`YellowState` | Đèn đỏ → xanh → vàng → đỏ với thời lượng cấu hình được |
| `RoadComponent`, `Lane`, `Road`, `Intersection` | Hạ tầng: làn chứa xe, đường chứa làn, nút giao nối làn và sở hữu đèn |
| `LightMapper` | Đổi màu đèn thành tín hiệu mà xe hiểu |
| `Simulation` | Thời gian mô phỏng, điều phối các bước, chạy theo thời gian thật, chạy nền |

Mỗi bước mô phỏng có 2 giai đoạn: (1) các Road cập nhật xe **song song**, (2) các Intersection đổi đèn và chuyển xe sang làn ra trên **một thread**. Mô tả đầy đủ, bảng sở hữu và luồng một bước: [docs/architecture.md](docs/architecture.md).

## OOP

- **Kế thừa:** `Car`, `Bus`, `Motorbike` kế thừa `Vehicle`; `Lane`, `Road`, `Intersection` kế thừa `RoadComponent`; các state kế thừa `TrafficLightState` / `VehicleState`. Mỗi quan hệ là "is-a"; quan hệ "has-a" dùng thành phần (`Road` có `Lane`, `Lane` có `Vehicle`).
- **Trừu tượng:** `Vehicle` là lớp trừu tượng (`getMaxSpeed`, `getAcceleration`, `getDeceleration`, `getFuelPerMeter`, `display` là hàm ảo thuần) nên không tạo được "một chiếc xe chung chung". `RoadComponent`, `VehicleState`, `TrafficLightState` cũng là giao diện trừu tượng.
- **Đa hình:** `Lane` giữ `vector<unique_ptr<Vehicle>>` chứa lẫn ô tô, xe buýt, xe máy và chỉ gọi `vehicle->update(dt)`; hàm nào chạy tuỳ loại xe và state hiện tại. Tương tự `component->update(dt)` cho hạ tầng.
- **Đóng gói:** mọi thành viên của `Vehicle`, `Lane`, `Road`... là `private`, truy cập qua getter `const` / setter có kiểm tra (ví dụ `setFuel` không cho âm, constructor của `Lane`, `Road`, `TrafficLight` ném `invalid_argument` khi tham số sai).
- **Smart pointer:** không có `new` / `delete` thủ công. Mỗi đối tượng có đúng một chủ (`unique_ptr`); con trỏ thô chỉ dùng để *tham chiếu*, không sở hữu (`vehicleAhead`, `currentLane`, `Lane::light`).

| Loại xe | Tốc độ tối đa | Tăng tốc | Phanh | Nhiên liệu / m | Khoảng cách an toàn |
|---|---|---|---|---|---|
| Car | 25 m/s | 3 m/s² | 5 m/s² | 0.08 | 10 m |
| Bus | 20 m/s | 2 m/s² | 4 m/s² | 0.15 | 12 m |
| Motorbike | 20 m/s | 6 m/s² | 8 m/s² | 0.04 | 5 m |

## State Pattern

Dùng hai lần:

```
TrafficLight → TrafficLightState → RedState | GreenState | YellowState     (chuyển theo thời gian)
Vehicle      → VehicleState      → Stopped | Accelerating | Cruising | Braking | WaitingAtLight   (chuyển theo điều kiện)
```

Lý do: hành vi thật sự khác nhau theo trạng thái, có nhiều đường chuyển, và cần thêm trạng thái mới mà không sửa một loạt `switch`. Mỗi state tự biết state kế tiếp; `TrafficLight` và `Vehicle` chỉ giữ state hiện tại và uỷ quyền. Mọi state của xe dùng chung một hàm `Vehicle::getDesiredSpeed()` (gộp tốc độ mục tiêu, tốc độ tối đa, xe trước, đèn) nên không state nào tự tính va chạm. Chi tiết, sơ đồ chuyển state, cách thêm state: [docs/state-pattern.md](docs/state-pattern.md).

## Composite Pattern

```
RoadComponent  (update, display, getVehicleCount)
 ├── Lane          (leaf: sở hữu xe)
 ├── Road          (composite: sở hữu các Lane)
 └── Intersection  (nối làn vào → làn ra, sở hữu đèn)
```

Một `Lane` đơn lẻ và một `Road` gồm nhiều làn được dùng qua cùng một giao diện: `road.update(dt)` cập nhật mọi làn con, `road.getVehicleCount()` cộng số xe các làn. `Simulation` giữ Road và Intersection ở hai danh sách riêng vì hai loại chạy khác nhau trong đa luồng (có giải thích đầy đủ ở tài liệu). Chi tiết và hạn chế: [docs/composite-pattern.md](docs/composite-pattern.md).

## Đa luồng

- **Cái gì chạy song song:** giai đoạn cập nhật xe: các Road được chia cho tối đa `maxWorkers` thread (mặc định = số nhân CPU, không bao giờ nhiều hơn số Road). Ngoài ra `start()` chạy cả vòng mô phỏng trên 1 thread nền.
- **Dữ liệu dùng chung:** thời gian mô phỏng, số bước, cờ `running` (đều `std::atomic`); đèn (chỉ đọc ở giai đoạn xe).
- **Đồng bộ:** thiết kế **tránh** để hai thread cùng ghi một dữ liệu: mỗi Road thuộc đúng một thread; mọi thao tác ghi xuyên Road (đổi đèn, chuyển xe giữa các làn) dồn vào giai đoạn 2 chạy một thread, sau `join()`. Mutex chỉ dùng cho `condition_variable` để `stop()` đánh thức thread nền ngay. Kết quả **giống hệt** khi chạy 1 thread (có test so sánh từng bước).
- **Vì sao không một thread mỗi xe:** hàng nghìn xe sẽ tạo hàng nghìn thread (tốn bộ nhớ, tốn thời gian chuyển ngữ cảnh) và mọi xe cùng làn vẫn phụ thuộc nhau (xe sau đọc vị trí xe trước), nên song song hoá theo xe không nhanh hơn mà còn cần khoá. Tách theo Road giữ số thread có giới hạn và không cần khoá.
- **ThreadSanitizer:** `make test-simulation-tsan` không có cảnh báo nào (kết quả đầy đủ trong tài liệu).

Bảng "ai sở hữu / ai đọc / ai ghi" cho từng dữ liệu, quy tắc khi sửa code, xử lý exception và dừng thread: [docs/concurrency.md](docs/concurrency.md).

## Build

Yêu cầu: `g++` hỗ trợ C++17 (đã build và test với g++ 13, Ubuntu 24.04) và `make`. Không dùng thư viện ngoài.

```bash
make            # build ./traffic-sim
make clean      # xoá file build
```

Windows (PowerShell, MinGW-w64 bản **posix thread** vì dùng `std::thread`):

```powershell
g++ -std=c++17 -Wall -Wextra -pthread -Iinclude (Get-ChildItem src -Recurse -Filter *.cpp).FullName main.cpp -o traffic-sim.exe
```

## Chạy

```bash
./traffic-sim        # hoặc: make run      (1 làn, 3 xe, 1 đèn, 36 bước)
make demo            # demo của Simulation: run(), runRealTime(), start()/stop()
make concurrency     # thí nghiệm đa luồng độc lập (Phase 11), không dùng simulator
```

`make demo` mất khoảng 5 giây thật vì có đoạn chạy theo thời gian thật.

## Ví dụ output

`./traffic-sim`, hai bước đầu (đèn hiển thị trước khi cập nhật, vị trí là sau khi cập nhật):

```
=========== Step 0 ===========
TrafficLight no.1 -> RED (time left: 12.00s)
  #1 Accelerating   pos= 403.00 speed=  3.00
  #2 Accelerating   pos= 372.00 speed=  2.00
  #3 Accelerating   pos= 356.00 speed=  6.00
=========== Step 1 ===========
TrafficLight no.1 -> RED (time left: 11.00s)
  #1 Accelerating   pos= 409.00 speed=  6.00
  #2 Accelerating   pos= 376.00 speed=  4.00
```

`make demo`, đoạn `runRealTime(30, 10.0)`: mô phỏng 30 giây ở tốc độ gấp 10 lần thời gian thật. Đèn đỏ 12 giây, xanh 10 giây, vàng 3 giây; các xe bắt đầu sang Road B ngay sau khi đèn xanh:

```
Real-time simulation started (dt=1s, x10)
t=3s | A: 3 xe | B: 0 xe | I1: RED
t=12s | A: 3 xe | B: 0 xe | I1: GREEN
t=13s | A: 2 xe | B: 1 xe | I1: GREEN
t=15s | A: 0 xe | B: 3 xe | I1: GREEN
t=22s | A: 0 xe | B: 3 xe | I1: YELLOW
t=25s | A: 0 xe | B: 3 xe | I1: RED
```

(Cột `A` / `B` là số xe trên Road A / Road B; xe qua vạch dừng ở cuối làn A thì chuyển sang làn B.)

## Kiểm thử

```bash
make test                    # chạy cả 4 bộ test, build với AddressSanitizer + UBSan
make test-trafficlight       # chạy riêng từng bộ: test-vehicle, test-trafficlight,
                             #   test-infrastructure, test-simulation
make test-simulation-tsan    # ThreadSanitizer cho phần đa luồng (Linux / Mac / WSL)
```

| Bộ test | Kiểm tra gì |
|---|---|
| `tests/test_vehicle.cpp` | `dt = 0` không chia cho 0, hết xăng thì phanh dừng, nhiên liệu tỉ lệ quãng đường, nạp xăng chạy lại |
| `tests/test_trafficlight.cpp` | Thứ tự đổi màu, ranh giới, thời gian dư, `dt` lớn / vô cực, từng state riêng, `LightMapper`, `Lane` đọc đèn |
| `tests/test_infrastructure.cpp` | Lane, Road, Intersection: thứ tự xe, thêm / lấy xe, chuyển xe qua nút giao, làn ra đầy |
| `tests/test_simulation.cpp` | Đèn đỏ xe dừng, đèn xanh xe đi, thời gian thực, song song = tuần tự, `start()` / `stop()`, exception trong thread, huỷ khi đang chạy nền |

Test dùng `assert` (đã `#undef NDEBUG` nên luôn bật) và là test xác định (không phụ thuộc may rủi); riêng kiểm tra thời gian thật chỉ có cận rất rộng.

## Cấu trúc thư mục

```
include/ src/        Vehicle/  Vehicle/States/  Traffic/  Infrastructure/  Integration/  Simulation/
main.cpp             demo nhỏ (1 Lane, không dùng Simulation)
demo/                demo của Simulation
tests/               4 bộ test
experiments/         thí nghiệm đa luồng độc lập
docs/                architecture.md, state-pattern.md, composite-pattern.md, concurrency.md
CHANGES.md           nhật ký thay đổi và giới hạn đã biết
```

## Giới hạn đã biết

- Đèn vàng quá ngắn so với tốc độ vẫn có "vùng lưỡng nan": xe không kịp dừng lẫn không kịp qua vạch.
- Xe chưa có chiều dài; chỉ có làn thẳng, không chuyển làn hay rẽ; Intersection không kiểm tra xung đột giữa các hướng.
- Giai đoạn xe tạo lại thread mỗi bước, và kịch bản chỉ có 1–2 Road gần như không nhanh hơn khi dùng nhiều thread.

Danh sách đầy đủ ở cuối [docs/architecture.md](docs/architecture.md) và [CHANGES.md](CHANGES.md).
