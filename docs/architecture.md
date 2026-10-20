# Kiến trúc của simulator

Tài liệu này mô tả **các lớp chính, ai sở hữu ai, và một bước mô phỏng chạy như thế nào**. Hai pattern được giải thích riêng: [state-pattern.md](state-pattern.md), [composite-pattern.md](composite-pattern.md). Đa luồng: [concurrency.md](concurrency.md).

## 1. Bức tranh chung

```
                         Simulation                    (nhịp thời gian, điều phối, chạy nền)
                        /          \
               vector<Road>      vector<Intersection>
                    |                   |      \
              vector<Lane>        Approach[]     TrafficLight  ──► TrafficLightState (Red/Green/Yellow)
                    |            (lane vào, lane ra, đèn)
              vector<Vehicle>
                    |
          Car / Bus / Motorbike  ──► VehicleState (Stopped/Accelerating/Cruising/Braking/WaitingAtLight)
```

| Nhóm | Lớp | Một câu trách nhiệm |
|---|---|---|
| Xe | `Vehicle`, `Car`, `Bus`, `Motorbike` | Dữ liệu và vật lý của một xe; mỗi loại xe chỉ khác thông số |
| Trạng thái xe | `VehicleState` + 5 lớp con | Xe làm gì ở mỗi tick tuỳ trạng thái hiện tại |
| Đèn | `TrafficLight` | Giữ trạng thái đèn hiện tại và chuyển tiếp thời gian cho nó |
| Trạng thái đèn | `TrafficLightState`, `RedState`, `GreenState`, `YellowState` | Đếm ngược thời lượng của một màu và biết màu kế tiếp |
| Hạ tầng | `RoadComponent`, `Lane`, `Road`, `Intersection` | Giao diện chung; làn chứa xe; đường chứa làn; nút giao nối làn và sở hữu đèn |
| Cầu nối | `LightMapper` | Đổi màu đèn (`LightColor`) thành tín hiệu xe hiểu (`Vehicle::trafficLight`) |
| Điều phối | `Simulation` | Thời gian mô phỏng, chia việc cho thread, chạy nền, in trạng thái |

## 2. Cấu trúc thư mục

```
include/  src/
  Vehicle/            Vehicle, Car, Bus, Motorbike
  Vehicle/States/     Vehicle_State + Stopped, Accelerating, Cruising, Braking, WaitingAtLight
  Traffic/            TrafficLight, TrafficLightState, RedState, GreenState, YellowState
  Infrastructure/     RoadComponent, Lane, Road, Intersection
  Integration/        LightMapper
  Simulation/         Simulation
main.cpp              demo nhỏ: 1 Lane + 1 đèn, không dùng Simulation
demo/                 demo của Simulation (run, runRealTime, start/stop)
tests/                test_vehicle, test_trafficlight, test_infrastructure, test_simulation
experiments/          thí nghiệm đa luồng độc lập (Phase 11)
docs/                 tài liệu này
```

Cấu trúc thư mục khác bản gợi ý trong roadmap (`vehicle/`, `vehicle_state/`...) vì nhóm giữ tên theo từng nhóm lớp; roadmap cho phép điều này (mục 23: "nhất quán quan trọng hơn số thư mục").

## 3. Các lớp chính

### Vehicle (lớp cơ sở trừu tượng)
- **Trách nhiệm:** dữ liệu một xe (vị trí, tốc độ, nhiên liệu) và phần vật lý dùng chung cho mọi loại xe.
- **Trường quan trọng (đều `private`):** `speed`, `position`, `targetSpeed`, `safeDistance`, `fuel`, `vehicleAhead` (con trỏ không sở hữu), `currentLane` (không sở hữu), `currentState` (`unique_ptr<VehicleState>`).
- **Hàm quan trọng:**
  - `getDesiredSpeed(dt)`: tốc độ lớn nhất được phép ở tick tới = `min(targetSpeed, maxSpeed, giới hạn xe trước, giới hạn đèn)`; hết nhiên liệu thì bằng 0. **Mọi state dùng chung hàm này** (xem mục 6).
  - `advance(newSpeed, dt)`: ghi tốc độ, di chuyển, trừ nhiên liệu theo quãng đường thực đi, không cho chồng lên xe trước.
  - `transitionTo`, `enterHaltState`: đổi state (xem state-pattern.md).
- **Hàm ảo thuần:** `getMaxSpeed`, `getAcceleration`, `getDeceleration`, `getFuelPerMeter`, `display`.

| Loại | Tốc độ tối đa | Tăng tốc | Phanh | Nhiên liệu / m | Khoảng cách an toàn |
|---|---|---|---|---|---|
| `Car` | 25 m/s | 3 | 5 | 0.08 | 10 m |
| `Bus` (+ `capacity`) | 20 m/s | 2 | 4 | 0.15 | 12 m |
| `Motorbike` (+ `engineCC`) | 20 m/s | 6 | 8 | 0.04 | 5 m |

Nhiên liệu nằm ở lớp cơ sở (không lặp ở 3 lớp con); mỗi loại xe chỉ khai báo mức tiêu hao riêng qua `getFuelPerMeter()` (quyết định của Phase 1).

### TrafficLight và TrafficLightState
`TrafficLight` chỉ giữ `unique_ptr<TrafficLightState>` và uỷ quyền. Mỗi state giữ bản sao `TrafficLightConfig` (3 thời lượng) nên **tự tạo được state kế tiếp** mà `TrafficLight` không cần biết màu nào dài bao lâu. Chi tiết: state-pattern.md.

### Lane
- **Trách nhiệm:** chứa xe của một làn thẳng, vị trí đo từ 0 (đầu làn) đến `length` (cuối làn = vạch dừng nếu có đèn).
- **Sở hữu** `vector<unique_ptr<Vehicle>>`, luôn sắp theo vị trí giảm dần (phần tử 0 = xe đầu đoàn). `refreshOrder()` sắp lại và nối `vehicleAhead` cho từng xe.
- **Hàm quan trọng:** `addVehicle`, `takeExitedVehicle` (chuyển quyền sở hữu xe đã qua cuối làn), `getDistanceToNextLight(position)` (Vehicle hỏi qua đây), `getLightSignal()`, `hasRoomAtEntry(gap)`, `update(dt)`.
- **`update(dt)`:** duyệt từ xe đầu về xe cuối; gán tín hiệu đèn; hạ `targetSpeed` xuống `speedLimit` của làn *trong lúc update xe* rồi trả lại giá trị cũ (làn không xoá vĩnh viễn ý định của xe); gọi `vehicle->update(dt)`.

### Road
Composite sở hữu nhiều `Lane` có cùng chiều dài. `addLane` từ chối id trùng. `update`, `display`, `getVehicleCount` chuyển xuống từng Lane / cộng dồn.

### Intersection
- **Trách nhiệm:** sở hữu các đèn và chuyển xe từ làn vào sang làn ra.
- Mỗi `Approach` = `{làn vào, làn ra (hoặc nullptr = rời hệ thống), unique_ptr<TrafficLight>}`. `addApproach` tạo đèn, gắn vào làn vào (`Lane::setTrafficLight`) và có thể tua đèn đi trước `phaseOffset` giây để các hướng không cùng xanh/đỏ.
- **`update(dt)`:** cập nhật đèn, rồi với mỗi hướng: nếu xe đầu đoàn đã qua cuối làn vào và làn ra còn chỗ (`hasRoomAtEntry`) thì chuyển xe sang làn ra, mang theo quãng đã đi quá vạch (`overshoot`). Làn ra đầy thì xe giữ lại ở cuối làn vào, không bị mất.

### Simulation
Giữ `dt`, thời gian mô phỏng (`atomic<double>`), số bước (`atomic<int>`), danh sách Road và Intersection. `step()` chạy một bước; `run(n)` in chi tiết; `runRealTime(s, speed)` chạy theo thời gian thật; `start(speed)`/`stop()` chạy nền trên 1 thread. Thời gian **mô phỏng** (tăng đúng `dt` mỗi bước) tách khỏi thời gian **thật** (`dt / speed` giây mỗi bước, đo bằng `steady_clock`).

## 4. Ai sở hữu ai

| Đối tượng | Chủ sở hữu | Ai chỉ giữ con trỏ (không sở hữu) |
|---|---|---|
| `Road` | `Simulation` (`unique_ptr`) | `Intersection::Approach` (qua `Lane*`) |
| `Intersection` | `Simulation` (`unique_ptr`) | – |
| `Lane` | `Road` (`unique_ptr`) | `Intersection::Approach`, `Vehicle::currentLane` |
| `Vehicle` | `Lane` (`unique_ptr`) | `Vehicle::vehicleAhead` (xe liền trước, cùng làn) |
| `TrafficLight` | `Intersection` (`unique_ptr` trong `Approach`) | `Lane::light` (`const TrafficLight*`) |
| `VehicleState` | `Vehicle` (`unique_ptr`) | – |
| `TrafficLightState` | `TrafficLight` (`unique_ptr`) | – |

Quy tắc vòng đời: không có sở hữu kép; chỉ có đúng một chủ cho mỗi đối tượng. Xe đổi làn qua `takeExitedVehicle()` → `addVehicle()` là **chuyển** `unique_ptr` (không sao chép). Trong `Simulation`, `roads` khai báo trước `intersections`, nên khi huỷ thì Intersection (giữ `Lane*`) bị huỷ trước Road. Lane giữ `const TrafficLight*` vào đèn do Intersection sở hữu, nên Lane không được dùng sau khi Intersection bị huỷ (Simulation huỷ cả hai cùng lúc nên không xảy ra).

## 5. Một bước mô phỏng

```
Simulation::step()  (dt giây mô phỏng)
 ├─ GĐ 1  updateRoads()          các Road chạy SONG SONG, tối đa maxWorkers thread (mỗi Road đúng 1 thread)
 │    Road::update
 │     └─ Lane::update
 │         ├─ refreshOrder()                     sắp xe, nối vehicleAhead
 │         ├─ signal = getLightSignal()          đọc đèn (CHỈ ĐỌC): LightMapper(light)
 │         └─ với từng xe, đầu đoàn trước:
 │              setTrafficLight(signal); kẹp targetSpeed theo speedLimit
 │              Vehicle::update(dt) ──► currentState->update(vehicle, dt)
 │                                         getDesiredSpeed → quyết định tăng / giữ / giảm tốc → advance()
 ├─ join                         đợi mọi thread GĐ 1
 ├─ GĐ 2  Intersection::update   1 THREAD: đổi đèn, chuyển xe qua vạch sang làn ra
 └─ time += dt; stepCount++
```

Hai giai đoạn tách nhau để không cần mutex: GĐ 1 chỉ ghi vào dữ liệu của từng Road, GĐ 2 là nơi duy nhất ghi xuyên Road (đổi đèn, chuyển xe). Xem concurrency.md.

## 6. Xe quyết định tốc độ thế nào

`Vehicle::getDesiredSpeed(dt)` là **nơi duy nhất** tính tốc độ cho phép; mọi state chỉ so sánh tốc độ hiện tại với nó:

1. `targetSpeed` (ý định của xe, đã bị Lane kẹp theo `speedLimit`) và `getMaxSpeed()` của loại xe.
2. **Giới hạn xe trước** (`getLeaderLimit`): tốc độ lớn nhất sao cho đi `v·dt` rồi phanh với gia tốc của mình vẫn dừng được cách xe trước ít nhất `safeDistance`. Quãng phanh của xe trước được cộng vào khoảng trống. Sau tick, xe cũng không được sát hơn `safeDistance`.
3. **Giới hạn đèn** (`getLightLimit`): đèn xanh không giới hạn; đèn đỏ hoặc tín hiệu không xác định (`unknownLight`, coi như đỏ) coi vạch dừng là vật cản đứng yên; đèn vàng thì dừng nếu phanh kịp trước vạch, nếu không kịp thì đi tiếp (an toàn hơn dừng giữa nút giao).
4. Hết nhiên liệu → 0, xe phải phanh dừng. Nạp lại bằng `setFuel`.

Hệ quả: không state nào tự tính va chạm hay đèn, nên thêm một ràng buộc mới (ví dụ giới hạn tốc độ khu vực) chỉ cần sửa một hàm.

## 7. Các quyết định thiết kế chính

| Quyết định | Lý do |
|---|---|
| `getDesiredSpeed` dùng chung cho mọi state | Một nơi duy nhất đảm bảo không va chạm, không vượt vạch, không vượt `maxSpeed` |
| Lane sở hữu xe, Road sở hữu Lane, Intersection sở hữu đèn | Một chủ cho mỗi đối tượng, không sở hữu kép, huỷ tự động (RAII) |
| Intersection nối làn với làn, không chứa Road | Tránh sở hữu kép; Road đã sở hữu Lane |
| Đổi state không mất tick chuyển động (`transitionTo`) | Xe không "đứng khựng" một tick mỗi lần đổi state |
| Tách thời gian mô phỏng khỏi thời gian thật | Kết quả không phụ thuộc máy nhanh hay chậm; test xác định được |
| Song song theo Road, không theo xe | Số thread bị chặn, không dùng mutex, kết quả giống hệt chạy 1 thread |
| Hàm `update()` không in ra màn hình | In từ nhiều thread sẽ lẫn dòng; chỉ `display()` và các lệnh gọi tay mới in |

## 8. Khác với kế hoạch ban đầu trong roadmap

| Roadmap | Hiện tại | Lý do |
|---|---|---|
| Vehicle state: `Driving`, `Braking`, `Stopped` | 5 state: `Stopped`, `Accelerating`, `Cruising`, `Braking`, `WaitingAtLight` | Tách "đang đi" thành tăng tốc / giữ tốc để mỗi state có một quy tắc rõ; `WaitingAtLight` phân biệt dừng vì đèn với dừng vì lý do khác (hết xăng, xe trước chặn) |
| `City` chứa `Intersection` chứa `Road` | Không có `City`; `Simulation` giữ `Road` và `Intersection` | Roadmap nói không cần tạo mọi lớp; `Simulation` đóng vai gốc của cây (xem composite-pattern.md) |
| Simulation gọi chung `RoadComponent::update()` (Phase 2) | Từ Phase 12 giữ 2 danh sách riêng | Road chạy song song, Intersection phải chạy 1 thread sau đó |
| `main.cpp` chạy cả mô phỏng | `main.cpp` là demo nhỏ 1 Lane; `demo/simulation_demo.cpp` dùng `Simulation` | Giữ nguyên demo gốc của nhóm; `Simulation` có demo riêng |

## 9. Giới hạn đã biết

- **Vùng lưỡng nan của đèn vàng:** nếu đèn vàng ngắn hơn `v/2a` cộng thời gian phản ứng thì có tốc độ mà xe không kịp dừng lẫn không kịp qua vạch.
- Xe chưa có chiều dài (khoảng cách là khoảng giữa hai điểm).
- Khi cả hàng đang chậm dần, khoảng cách có thể tạm thấp hơn `safeDistance` (khoảng 80%); lúc dừng hẳn luôn ≥ `safeDistance`.
- Chỉ có làn thẳng, không chuyển làn, không rẽ. Một Intersection không kiểm tra xung đột giữa các hướng (các hướng xanh cùng lúc vẫn được phép; chỉ dùng `phaseOffset` để lệch pha).
- Mỗi bước tạo lại thread cho GĐ 1; với ít Road thì đủ nhanh, nhiều Road nên đổi sang thread pool.
- Song song theo Road: kịch bản chỉ 1–2 Road gần như không nhanh hơn.
