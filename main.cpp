#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

#include "Vehicle/Car.h"
#include "Vehicle/Bus.h"
#include "Vehicle/Motorbike.h"
#include "Traffic/TrafficLight.h"
#include "Infrastructure/Lane.h"
#include "Integration/LightMapper.h"

using namespace std;

int main() {
    Lane lane;   // vạch dừng ở 500m (xem Lane.h)

    // Thứ tự trong vector = từ xe ĐẦU hàng tới xe CUỐI hàng (xe đầu được update trước).
    vector<unique_ptr<Vehicle>> vehicles;
    vehicles.push_back(make_unique<Car>(1, 0, 100));
    vehicles.push_back(make_unique<Bus>(2, 0, 200, 30));
    vehicles.push_back(make_unique<Motorbike>(3, 0, 125, 30));

    const double startPos[]    = {400, 370, 350};
    const double targetSpeed[] = {20, 15, 18};
    for (size_t i = 0; i < vehicles.size(); i++) {
        vehicles[i]->setPosition(startPos[i]);
        vehicles[i]->setTargetSpeed(targetSpeed[i]);
        vehicles[i]->setCurrentLane(&lane);
        if (i > 0) vehicles[i]->setVehicleAhead(vehicles[i - 1].get());
    }

    // id=1, RED=12s, GREEN=10s, YELLOW=3s
    TrafficLight light(1, 12, 10, 3);

    const double dt = 1;
    const int steps = 36;
    cout << fixed << setprecision(2);
    for (int step = 0; step < steps; step++) {
        cout << "=========== Step " << step << " ===========\n";
        light.display();
        Vehicle::trafficLight signal = LightMapper(light);   // nối đèn -> xe
        for (auto& v : vehicles) v->setTrafficLight(signal);

        for (auto& v : vehicles) {
            v->update(dt);
            cout << "  #" << v->getId() << " " << setw(14) << left << v->getState()->getName() << right
                 << " pos=" << setw(7) << v->getPosition()
                 << " speed=" << setw(6) << v->getSpeed() << "\n";
        }
        light.update(dt);
    }
    return 0;
}
