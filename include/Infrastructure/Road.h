#ifndef ROAD_H
#define ROAD_H

#include <memory>
#include <string>
#include <vector>
#include "Infrastructure/RoadComponent.h"
#include "Infrastructure/Lane.h"

// Road = Composite: SỞ HỮU nhiều Lane (các làn song song) cùng chiều dài.
class Road : public RoadComponent {
private:
    double length;
    std::vector<std::unique_ptr<Lane>> lanes;

public:
    Road(int _id, const std::string& _name, double _length);

    // Tạo và thêm một làn mới có chiều dài = length của Road.
    // Trả về con trỏ KHÔNG sở hữu (sống đến khi Road bị huỷ).
    Lane* addLane(int laneId, double speedLimit = 0.0);

    double getLength() const;
    std::size_t getLaneCount() const;
    Lane* getLane(std::size_t index) const;   // nullptr nếu index sai

    void update(double dt) override;          // update lần lượt từng Lane
    void display() const override;
    int getVehicleCount() const override;     // cộng dồn xe của các Lane
};

#endif
