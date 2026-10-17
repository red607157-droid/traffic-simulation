#include <memory>

#include "Simulation/Simulation.h"
#include "Vehicle/Car.h"
#include "Vehicle/Bus.h"
#include "Vehicle/Motorbike.h"
#include "Traffic/TrafficLight.h"

using namespace std;

int main() {
    Simulation sim(1.0);   // 1 bước = 1 giây mô phỏng; vạch dừng ở 500m (xem Lane.h)

    // id=1, RED=12s, GREEN=10s, YELLOW=3s
    sim.setTrafficLight(make_unique<TrafficLight>(1, 12, 10, 3));

    // Thêm theo thứ tự từ xe ĐẦU hàng tới xe CUỐI hàng.
    Vehicle& car = sim.addVehicle(make_unique<Car>(1, 0, 100));
    car.setPosition(400);
    car.setTargetSpeed(20);

    Vehicle& bus = sim.addVehicle(make_unique<Bus>(2, 0, 200, 30));
    bus.setPosition(370);
    bus.setTargetSpeed(15);

    Vehicle& bike = sim.addVehicle(make_unique<Motorbike>(3, 0, 125, 30));
    bike.setPosition(350);
    bike.setTargetSpeed(18);

    sim.run(36);
    return 0;
}
