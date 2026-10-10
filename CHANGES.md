# Các thay đổi so với bản gốc

## Build
- Makefile: tìm đệ quy `src/**/*.cpp` + `main.cpp`, build ra `build/`, có `all`, theo dõi header (`-MMD`).
- Thêm lớp `Bus` (Bus.h / Bus.cpp). Giả định: `Bus(id, speed, fuel, capacity)`, max 20 m/s, a=2, decel=4, safe=12m.
- Xoá khai báo trùng `getDistanceToNextLight`, thêm `WaitingAtLight::getName`, sửa `#include`, `move` -> `update`.

## Logic
- `Vehicle` khởi tạo đủ member; `update(dt<=0)` bị bỏ qua.
- Một nơi duy nhất quyết định tốc độ cho phép: `Vehicle::getDesiredSpeed(dt)` = min(target, maxSpeed, giới hạn xe trước, giới hạn đèn).
  Mọi state dùng nó -> không còn va chạm, vượt vạch, vượt maxSpeed, đèn chỉ áp dụng cho xe đầu.
- Đổi state không mất tick chuyển động (`Vehicle::transitionTo`, có chặn đệ quy).
- `Stopped` / `WaitingAtLight` tự chạy lại khi được phép đi.
- Đèn vàng: nếu phanh không kịp thì đi tiếp. `unknownLight` được coi như đèn đỏ.
- `TrafficLight::update` mang phần thời gian dư sang state kế tiếp; thời gian <= 0 ném `invalid_argument`.
- `Lane` (file của Duy): hàm `const`, dung sai 1e-6 tại vạch dừng. Vẫn là bản tạm (vạch cứng 500m).
- `main.cpp`: nối đèn -> xe (`LightMapper`), nối xe trước/sau, gán làn, 36 bước.

## Giới hạn đã biết
- Đèn vàng quá ngắn so với tốc độ (< v/2a + thời gian phản ứng) vẫn có "vùng lưỡng nan": xe không kịp dừng lẫn kịp qua.
- Xe chưa có chiều dài (gap = khoảng cách giữa hai điểm).
- Khi xếp hàng đang chạy chậm dần, gap có thể tạm thấp hơn safeDistance (~80%); lúc dừng hẳn luôn >= safeDistance.

# Đợt sửa lỗi tiếp theo (review)
- `Vehicle::getLeaderLimit`: sửa chia cho 0 khi `dt = 0` (undefined behavior, bị `accelerate()` kích hoạt).
- `fuel` chuyển từ 3 lớp con lên `Vehicle` (getFuel/setFuel dùng chung). Thêm `getFuelPerMeter()` (mỗi loại xe một mức tiêu hao); `advance()` trừ nhiên liệu theo quãng đường thực đi. Hết nhiên liệu -> `getDesiredSpeed` = 0 -> xe phanh dừng. Nạp lại bằng `setFuel`.
- Mọi member của `Vehicle` chuyển sang `private`; `safeDistance` và `fuel` truyền qua constructor của `Vehicle`.
- Thêm `Vehicle::enterHaltState(dt, runUpdateNow)` thay cho đoạn "WaitingAtLight hay Stopped" bị lặp 4 lần trong các state.
- `display()`: phần chung gom vào `Vehicle::displayCommon()`; lớp con chỉ in tên loại xe + thông tin riêng.
- `TrafficLight::getColor()` trả `LightColor` (enum); `LightMapper` dùng `switch` thay cho so sánh chuỗi "RED"/"GREEN"/"YELLOW".
- Xoá code không dùng: `getDynamicSafeDistance`, `getDistanceToVehicleAhead`.
- Thêm `tests/test_vehicle.cpp` + `make test`. Chuẩn hoá line ending về LF.
- Giữ nguyên: `std::cout` trong các state (chỉ là thông báo lệnh), `Lane` tạm của Duy.
