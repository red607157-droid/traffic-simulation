#ifndef VEHICLE_H
#define VEHICLE_H

#include "Vehicle/States/Vehicle_State.h"
#include <memory>
#include <string>

class Lane;

class Vehicle {
public:
    enum trafficLight { redLight, greenLight, yellowLight, unknownLight };

    static constexpr double NO_LIGHT = 9999.0;  // Lane trả về giá trị này = "không có đèn phía trước"
    static constexpr double EPS      = 1e-9;    // sai số khi so sánh số thực (khoảng cách)
    static constexpr double SPEED_EPS = 0.01;   // sai số tốc độ (m/s): |v - D| nhỏ hơn mức này coi như bằng nhau / đã dừng

private:
    int id;
    double speed;
    double position;
    double targetSpeed;
    double safeDistance;
    double fuel;                  // nhiên liệu còn lại (đơn vị tuỳ ý, giảm theo quãng đường đi)
    Vehicle* vehicleAhead;        // KHÔNG sở hữu: xe phía trước phải sống lâu hơn xe này
    trafficLight currentTrafficLight;
    Lane* currentLane;            // KHÔNG sở hữu
    std::unique_ptr<VehicleState> currentState;
    int dispatchDepth;            // chống đệ quy vô hạn khi chuyển state liên tiếp trong 1 tick

protected:
    // In các thông tin chung (state, vị trí, tốc độ, nhiên liệu, tốc độ tối đa). Lớp con in tên loại xe trước rồi gọi hàm này.
    void displayCommon() const;

public:
    Vehicle(int _i, double _s, double _fuel, double _safeDistance);

    // --- State ---
    void setState(std::unique_ptr<VehicleState> newState);
    VehicleState* getState() const;
    // Đổi state rồi chạy ngay update của state mới trong CÙNG tick (không mất 1 tick chuyển động).
    // Lưu ý: state gọi hàm này phải `return` ngay sau đó và không được đụng vào member của chính nó
    // (vì object state hiện tại đã bị huỷ).
    void transitionTo(std::unique_ptr<VehicleState> newState, double dt);
    // Xe đã dừng hẳn: vào WaitingAtLight nếu đang bị ĐÈN chặn, ngược lại vào Stopped.
    // runUpdateNow = true -> chạy update của state mới ngay trong tick này (như transitionTo).
    void enterHaltState(double dt, bool runUpdateNow);

    // --- Lệnh điều khiển ---
    void accelerate();
    void brake();      // = ra lệnh dừng (targetSpeed = 0 rồi phanh)
    void stop();
    void update(double dt);   // dt <= 0 bị bỏ qua

    // --- Getter / setter ---
    int getId() const;
    double getSpeed() const;
    double getPosition() const;
    double getTargetSpeed() const;
    trafficLight getTrafficLight() const;
    double getSafeDistance() const;
    Vehicle* getVehicleAhead() const;
    double getDistanceToNextLight() const;
    double getFuel() const;
    void setVehicleAhead(Vehicle* v);
    void setSpeed(double _s);
    void setPosition(double _p);
    void setTargetSpeed(double _ts);
    void setTrafficLight(trafficLight _tl);
    void setCurrentLane(Lane* l);
    void setFuel(double _f);

    // --- Vật lý / an toàn (dùng chung cho mọi state) ---
    // Tốc độ lớn nhất mà xe được phép chạy ở tick tới: min(target, max, giới hạn xe trước, giới hạn đèn).
    // Hết nhiên liệu -> 0 (xe phải phanh dừng).
    double getDesiredSpeed(double dt) const;
    double getLeaderLimit(double dt) const;   // giới hạn do xe phía trước
    double getLightLimit(double dt) const;    // giới hạn do đèn đỏ/vàng
    bool   isBlockedByLight(double dt) const; // đang bị chặn bởi đèn (chứ không phải bởi xe trước)
    // Ghi tốc độ mới, di chuyển xe dt giây, trừ nhiên liệu, và không cho xe đâm xuyên/chồng lên xe trước.
    void advance(double newSpeed, double dt);

    virtual double getMaxSpeed() const = 0;
    virtual double getAcceleration() const = 0;
    virtual double getDeceleration() const = 0;
    virtual double getFuelPerMeter() const = 0;   // nhiên liệu tiêu hao trên mỗi mét đi được
    virtual void display() const = 0;

    virtual ~Vehicle() = default;
};

#endif
