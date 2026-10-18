#ifndef LANE_H
#define LANE_H

#include <memory>
#include <string>
#include <vector>
#include "Infrastructure/RoadComponent.h"
#include "Vehicle/Vehicle.h"
#include "Traffic/TrafficLight.h"

// Leaf của Composite (RoadComponent): một làn đường thẳng, vị trí đo từ 0 (đầu làn) tới length (cuối làn / vạch dừng).
// Lane SỞ HỮU các xe trên nó, và là nơi Vehicle hỏi về "xe phía trước" và "đèn phía trước".
class Lane : public RoadComponent {
private:
    double length;
    double speedLimit;                                  // m/s, 0 = không giới hạn thêm
    const TrafficLight* light;                          // KHÔNG sở hữu (Intersection sở hữu), nullptr = không có đèn
    std::vector<std::unique_ptr<Vehicle>> vehicles;     // luôn sắp xếp theo position GIẢM DẦN: phần tử 0 = xe đầu đoàn

    // Sắp xếp lại vehicles và nối vehicleAhead cho từng xe (xe đầu đoàn -> nullptr).
    void refreshOrder();

public:
    Lane(int _id, const std::string& _name, double _length, double _speedLimit = 0.0);

    // --- Đèn giao thông ---
    // Đèn nằm ở cuối làn (vạch dừng tại position == length).
    void setTrafficLight(const TrafficLight* _light);
    bool hasTrafficLight() const;

    // --- Quản lý xe ---
    // Nhận quyền sở hữu xe, đặt position = entryPosition, gán currentLane cho xe. xe nullptr bị bỏ qua.
    void addVehicle(std::unique_ptr<Vehicle> v, double entryPosition = 0.0);
    // Xe đầu đoàn đã đi quá cuối làn (position > length) chưa?
    bool hasExitedVehicle() const;
    // Lấy ra (chuyển quyền sở hữu) xe đầu đoàn nếu nó đã đi quá cuối làn; nullptr nếu chưa có xe nào ra.
    // Lấy từng xe một để Intersection kiểm tra làn ra còn chỗ TRƯỚC khi lấy (xe không bị mất).
    // Sau khi lấy, xe còn lại được nối lại vehicleAhead ngay để không bị con trỏ treo.
    std::unique_ptr<Vehicle> takeExitedVehicle();

    // --- Truy vấn cho Vehicle (Vehicle::getDistanceToNextLight / setTrafficLight gọi qua đây) ---
    // Khoảng cách từ vị trí `position` tới đèn; Vehicle::NO_LIGHT nếu không có đèn hoặc đã qua vạch dừng.
    double getDistanceToNextLight(double position) const;
    // Tín hiệu đèn dưới dạng Vehicle::trafficLight (dùng LightMapper); unknownLight nếu không có đèn.
    Vehicle::trafficLight getLightSignal() const;

    // --- Getter ---
    double getLength() const;
    double getSpeedLimit() const;
    // Xe ở vị trí index (0 = đầu đoàn); nullptr nếu index sai. Con trỏ KHÔNG sở hữu.
    Vehicle* getVehicleAt(std::size_t index) const;
    // Xe đầu đoàn / xe cuối đoàn; nullptr nếu làn rỗng.
    Vehicle* getFrontVehicle() const;
    Vehicle* getRearVehicle() const;
    // Còn đủ chỗ ở đầu làn để nhận xe mới không (khoảng trống >= minGap).
    bool hasRoomAtEntry(double minGap) const;

    // --- RoadComponent ---
    // Thứ tự: refreshOrder -> với từng xe (đầu đoàn trước): gán tín hiệu đèn, hạ targetSpeed xuống speedLimit,
    // vehicle->update(dt) -> refreshOrder lại.
    // Xe đã vượt cuối làn mà chưa được Intersection nhận (làn ra đầy) bị 'đóng băng' (speed = 0) cho tới khi được chuyển đi.
    void update(double dt) override;
    void display() const override;
    int getVehicleCount() const override;
};

#endif
