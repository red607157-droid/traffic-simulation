# Composite Pattern trong simulator

## 1. Vấn đề Composite giải quyết

Hạ tầng đường có dạng **cây**: một đường gồm nhiều làn, một làn gồm nhiều xe. Phía ngoài (`Simulation`, `main`, test) muốn làm cùng một việc cho cả cây: "cập nhật", "in ra", "đếm xe". Nếu mỗi loại có một giao diện riêng thì code phía ngoài phải viết:

```cpp
if (là Lane)  { ... cập nhật từng xe ... }
else if (là Road) { for (từng Lane) ... }
else if (là Intersection) { ... }
```

và mỗi khi thêm loại mới (vòng xuyến, đường hầm) lại phải sửa mọi chỗ dùng. Composite cho một giao diện chung để **một phần tử đơn lẻ và một nhóm phần tử được đối xử như nhau**: gọi `component->update(dt)` thì Lane cập nhật xe của nó, Road cập nhật mọi Lane con, mà phía gọi không cần biết đó là loại nào.

## 2. Cấu trúc

```
                    RoadComponent  (giao diện: id, name, update(dt), display(), getVehicleCount())
                   /      |       \
               Lane     Road     Intersection
              (leaf)  (composite)  (thành phần nối + sở hữu đèn)
                         │
                   vector<unique_ptr<Lane>>
```

| Lớp | Vai trò | `update(dt)` | `getVehicleCount()` |
|---|---|---|---|
| `RoadComponent` | Component (giao diện) | thuần ảo | thuần ảo |
| `Lane` | **Leaf**: không có con là RoadComponent; sở hữu xe | gán đèn, cập nhật từng xe từ đầu đoàn | số xe trong làn |
| `Road` | **Composite**: sở hữu các `Lane` cùng chiều dài | chuyển xuống từng Lane | **cộng dồn** các Lane |
| `Intersection` | Thành phần nối làn với làn, sở hữu đèn | đổi đèn, chuyển xe sang làn ra | luôn 0 (xe nằm trong Lane, không ở nút giao) |

Cùng một hàm, tác động khác nhau tuỳ loại, nhưng phía gọi chỉ thấy `RoadComponent`. Ví dụ `Road::display()` gọi `display()` của từng Lane và `Road::getVehicleCount()` cộng số xe của các Lane con mà không cần biết Lane làm gì bên trong.

Cây thực tế trong một kịch bản:

```
Simulation
├── Road "A"
│   └── Lane 1  ── Car, Bus, Motorbike
├── Road "B"
│   └── Lane 2
└── Intersection "I1"      (nối Lane 1 → Lane 2, sở hữu TrafficLight)
```

## 3. Quyết định về sở hữu

- `Road` **sở hữu** `Lane` (`vector<unique_ptr<Lane>>`); Lane không biết ai sở hữu nó. Dùng `unique_ptr` nên con trỏ `Lane*` mà `addLane` trả về vẫn đúng dù vector tăng kích thước (có test, `test_infrastructure.cpp` test 11).
- `Intersection` **không sở hữu Road hay Lane**, chỉ giữ `Lane*` của hướng đi vào và đi ra. Lý do: Road đã sở hữu Lane, để Intersection cũng sở hữu sẽ thành sở hữu kép. Vì vậy cây "sở hữu" là `Simulation → Road → Lane → Vehicle`, còn Intersection là phần tử **nối** các nhánh của cây (và sở hữu đèn của các hướng).

## 4. Hạn chế, nói rõ để không hiểu nhầm

1. **Composite ở đây là dạng hẹp.** `Road` chỉ chứa `Lane` (không chứa `Road` hay `Intersection` khác), nên cây chỉ sâu hai tầng. Roadmap có phác thảo `City → Intersection → Road → Lane`; nhóm không làm `City` vì chưa có yêu cầu nào cần nó (Rule 4: không thêm trừu tượng thừa). Nếu cần, mở rộng bằng cách cho một lớp `City` giữ `vector<unique_ptr<RoadComponent>>` và chuyển `update` xuống; các lớp hiện có không phải sửa.
2. **`Simulation` không đối xử Road và Intersection hoàn toàn giống nhau.** Ở Phase 2 `Simulation` giữ một danh sách `RoadComponent` và gọi `update()` chung. Từ Phase 12 nó giữ `vector<Road>` và `vector<Intersection>` riêng, vì hai loại cần **cách chạy khác nhau**: Road chạy song song, Intersection phải chạy 1 thread sau khi mọi Road xong (xem [concurrency.md](concurrency.md)). Composite **vẫn được dùng** ở chỗ nó giải quyết đúng vấn đề: `Road` đối xử mọi `Lane` con qua một giao diện, và `Road`, `Lane`, `Intersection` cùng một giao diện `RoadComponent`; chỉ tầng ngoài cùng (`Simulation`) cần biết loại cụ thể để xếp lịch chạy. Đây là sự đánh đổi có chủ ý giữa tính đồng nhất của pattern và yêu cầu đúng / an toàn của đa luồng.
3. `Lane` đứng ở lá nên các hàm quản lý con (thêm / xoá con) chỉ có ở `Road` (`addLane`) chứ không nằm trong `RoadComponent` (cách "an toàn", thay vì cách "đồng nhất" nơi Lane phải ném lỗi khi bị thêm con).

## 5. Thêm một loại hạ tầng mới

Ví dụ một `Roundabout`:
1. Kế thừa `RoadComponent`, cài `update`, `display`, `getVehicleCount`.
2. Nếu chứa nhiều Lane thì dùng `vector<unique_ptr<Lane>>` như `Road` (hoặc dùng chính một `Road`).
3. Đưa vào `Simulation`: nếu nó chạy song song được thì thêm vào nhóm GĐ 1, nếu cần ghi xuyên Road thì vào GĐ 2. Phải đọc các quy tắc đa luồng trong concurrency.md trước.

## 6. Kiểm thử

- `tests/test_infrastructure.cpp`:
  - test 8–11: `Road::addLane` (id trùng bị từ chối, chiều dài làn = chiều dài Road), `Road::update` lan toả xuống Lane, `getVehicleCount` cộng dồn nhiều Lane, con trỏ `Lane*` ổn định;
  - test 12–16: Intersection nối làn vào và làn ra, chuyển xe, làn ra đầy thì xe ở lại.
- `tests/test_simulation.cpp` test 4, 6: đếm xe qua `Simulation`, xe chuyển Road qua nút giao mà tổng số xe không đổi.
