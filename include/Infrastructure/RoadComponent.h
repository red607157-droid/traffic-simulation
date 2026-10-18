#ifndef ROADCOMPONENT_H
#define ROADCOMPONENT_H

#include <string>

// Component của Composite Pattern: Lane, Road, Intersection đều có chung giao diện này
// để bên ngoài (Simulation) gọi update()/display() mà không cần biết loại cụ thể.
class RoadComponent {
private:
    int id;
    std::string name;

public:
    RoadComponent(int _id, const std::string& _name);
    virtual ~RoadComponent() = default;

    int getId() const { return id; }
    const std::string& getName() const { return name; }

    // dt <= 0 bị bỏ qua (giống Vehicle::update).
    virtual void update(double dt) = 0;
    virtual void display() const = 0;
    // Tổng số xe đang nằm trong thành phần này (composite = cộng dồn các con).
    virtual int getVehicleCount() const = 0;
};

#endif
