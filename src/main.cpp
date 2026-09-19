#include <iostream>
#include <vector>
#include <memory>
#include "Car.h"
#include "Bus.h"
#include "Motorbike.h"
#include "TrafficLight.h"

using namespace std;

int main() {
    int step = 0;
    vector<unique_ptr<Vehicle>> vehicles;
    vehicles.push_back(make_unique<Car>(1, 40, 100));
    vehicles.push_back(make_unique<Bus>(2, 30, 200, 30));
    vehicles.push_back(make_unique<Motorbike>(3, 50, 125, 30));

    // id=1, RED=2s, GREEN=2s, YELLOW=1s — short durations so a full
    // RED->GREEN->YELLOW->RED cycle is visible within a few steps.
    TrafficLight light(1, 2, 2, 1);

    double dt = 1;
    while (step != 6) {
        cout << "===========Step " << step << " ===========\n";
        light.display();
        light.update(dt);
        cout << "\n";
        for (size_t i = 0; i < vehicles.size(); i++) {
            vehicles[i]->display();
            cout << "\n";
            vehicles[i]->move(dt);
        }
        step++;
    }
    return 0;
}
