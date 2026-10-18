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
    // id=1, ten "lane-1", dai 500m (vach dung o 500, xem Lane.h), khong gioi han toc do rieng.
    Lane lane(1, "lane-1", 500.0);

    // addVehicle giao xe cho Lane theo thu tu bat ky; Lane tu refreshOrder() de
    // xac dinh xe nao la DAU hang (vi tri lon nhat) va noi vehicleAhead cho tung xe.
    const double startPos[]    = {400, 370, 350};
    const double targetSpeed[] = {20, 15, 18};

    auto car  = make_unique<Car>(1, 0, 100);
    auto bus  = make_unique<Bus>(2, 0, 200, 30);
    auto bike = make_unique<Motorbike>(3, 0, 125, 30);

    // Dat targetSpeed TRUOC khi move() giao quyen so huu cho Lane.
    car->setTargetSpeed(targetSpeed[0]);
    bus->setTargetSpeed(targetSpeed[1]);
    bike->setTargetSpeed(targetSpeed[2]);

    lane.addVehicle(move(car),  startPos[0]);
    lane.addVehicle(move(bus),  startPos[1]);
    lane.addVehicle(move(bike), startPos[2]);

    // id=1, RED=12s, GREEN=10s, YELLOW=3s
    TrafficLight light(1, 12, 10, 3);
    lane.setTrafficLight(&light);   // thay cho goi LightMapper() + setTrafficLight() tay moi step

    const double dt = 1;
    const int steps = 36;
    cout << fixed << setprecision(2);
    for (int step = 0; step < steps; step++) {
        cout << "=========== Step " << step << " ===========\n";
        light.display();

        lane.update(dt);   // Lane tu goi v->update(dt) cho tung xe, dung thu tu DAU->CUOI

        for (size_t i = 0; i < static_cast<size_t>(lane.getVehicleCount()); i++) {
            Vehicle* v = lane.getVehicleAt(i);
            cout << "  #" << v->getId() << " " << setw(14) << left << v->getState()->getName() << right
                 << " pos=" << setw(7) << v->getPosition()
                 << " speed=" << setw(6) << v->getSpeed() << "\n";
        }
        light.update(dt);
    }
    return 0;
}