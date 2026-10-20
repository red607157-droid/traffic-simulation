#include <iostream>
#include <cassert>
#include <cmath>
#include <limits>
#include "Vehicle/Car.h"
#include "Infrastructure/Road.h"
#include "Infrastructure/Lane.h"
#include "Infrastructure/Intersection.h"
using namespace std;

int main() {
    // 1. Lane validation: chieu dai <= 0 hoac speedLimit < 0 phai nem exception
    bool threw = false;
    try { Lane bad(1, "bad", -5.0, 0.0); } catch (const invalid_argument&) { threw = true; }
    assert(threw);
    threw = false;
    try { Lane bad(1, "bad", 10.0, -1.0); } catch (const invalid_argument&) { threw = true; }
    assert(threw);

    // 2. addVehicle: dat position (khong am) va currentLane
    {
        Lane lane(1, "L1", 100.0, 0.0);
        auto c = make_unique<Car>(1, 0.0, 100.0);
        lane.addVehicle(move(c), -5.0);               // am -> phai clamp ve 0
        assert(lane.getVehicleAt(0)->getPosition() == 0.0);
        assert(lane.getVehicleCount() == 1);
    }

    // 3. Thu tu xe (refreshOrder) + lien ket vehicleAhead, bat ke thu tu them vao
    {
        Lane lane(2, "L2", 100.0, 0.0);
        lane.addVehicle(make_unique<Car>(1, 0.0, 100.0), 10.0);
        lane.addVehicle(make_unique<Car>(2, 0.0, 100.0), 30.0);   // se thanh xe dau doan
        lane.addVehicle(make_unique<Car>(3, 0.0, 100.0), 5.0);    // se thanh xe cuoi doan

        assert(lane.getFrontVehicle()->getPosition() == 30.0);
        assert(lane.getVehicleAt(0)->getPosition() == 30.0);
        assert(lane.getVehicleAt(1)->getPosition() == 10.0);
        assert(lane.getVehicleAt(2)->getPosition() == 5.0);
        assert(lane.getRearVehicle()->getPosition() == 5.0);

        assert(lane.getVehicleAt(0)->getVehicleAhead() == nullptr);
        assert(lane.getVehicleAt(1)->getVehicleAhead() == lane.getVehicleAt(0));
        assert(lane.getVehicleAt(2)->getVehicleAhead() == lane.getVehicleAt(1));
    }

    // 4. hasExitedVehicle / takeExitedVehicle: xe di het lan, lay ra dung xe, don dep con tro
    {
        Lane lane(3, "L3", 10.0, 0.0);
        auto c = make_unique<Car>(42, 0.0, 100.0);
        c->setTargetSpeed(20.0);
        lane.addVehicle(move(c), 0.0);

        int tick = 0;
        while (!lane.hasExitedVehicle() && tick < 50) { lane.update(1.0); tick++; }
        assert(lane.hasExitedVehicle());

        auto exited = lane.takeExitedVehicle();
        assert(exited != nullptr);
        assert(exited->getId() == 42);
        assert(lane.getVehicleCount() == 0);
        assert(exited->getDistanceToNextLight() == Vehicle::NO_LIGHT);  // currentLane da duoc clear

        assert(lane.takeExitedVehicle() == nullptr);   // khong con xe nao de lay nua
    }

    // 5. getDistanceToNextLight: khong co den -> NO_LIGHT; co den -> khoang cach dung, >vach -> NO_LIGHT
    {
        Lane lane(4, "L4", 50.0, 0.0);
        assert(lane.getDistanceToNextLight(10.0) == Vehicle::NO_LIGHT);   // chua gan den

        TrafficLight light(4, 5, 5, 2);
        lane.setTrafficLight(&light);
        assert(lane.hasTrafficLight());
        assert(fabs(lane.getDistanceToNextLight(10.0) - 40.0) < 1e-9);
        assert(lane.getDistanceToNextLight(60.0) == Vehicle::NO_LIGHT);   // da qua vach
    }

    // 6. hasRoomAtEntry: lan rong -> true; xe cuoi doan qua gan dau lan -> false; du xa -> true
    {
        Lane lane(5, "L5", 100.0, 0.0);
        assert(lane.hasRoomAtEntry(5.0));
        lane.addVehicle(make_unique<Car>(1, 0.0, 100.0), 2.0);
        assert(!lane.hasRoomAtEntry(5.0));
        lane.getRearVehicle()->setPosition(10.0);
        assert(lane.hasRoomAtEntry(5.0));
    }

    // 7. HOI QUY: gioi han toc do cua lan khong duoc xoa vinh vien targetSpeed cua xe
    //    (bug da sua trong Lane::update - xem CHANGES.md)
    {
        Lane slow(6, "slow", 25.0, 5.0);
        Lane open(7, "open", 300.0, 0.0);

        auto cp = make_unique<Car>(1, 0.0, 100.0);
        cp->setTargetSpeed(25.0);
        Car* car = cp.get();
        slow.addVehicle(move(cp), 0.0);

        int tick = 0;
        while (!slow.hasExitedVehicle() && tick < 50) { slow.update(1.0); tick++; }
        assert(fabs(car->getTargetSpeed() - 25.0) < 1e-9);   // van con 25, khong bi ghi de con 5

        double overshoot = car->getPosition() - slow.getLength();
        open.addVehicle(slow.takeExitedVehicle(), overshoot);
        for (int i = 0; i < 5; i++) open.update(1.0);

        assert(fabs(car->getTargetSpeed() - 25.0) < 1e-9);
        assert(car->getSpeed() > 5.5);   // thuc su tang toc lai, khong bi ket o muc cu
    }

    // 8. Road::addLane: id trung bi tu choi, chieu dai lan = chieu dai Road
    {
        Road road(1, "R1", 200.0);
        Lane* l1 = road.addLane(1, 10.0);
        assert(l1 != nullptr);
        assert(fabs(l1->getLength() - 200.0) < 1e-9);
        assert(road.addLane(1, 15.0) == nullptr);   // id 1 da ton tai
        assert(road.addLane(2, 15.0) != nullptr);
        assert(road.getLaneCount() == 2);
    }

    // 9. Road::update lan toa het cac Lane con
    {
        Road road(2, "R2", 50.0);
        Lane* lane = road.addLane(1, 0.0);
        auto c = make_unique<Car>(1, 0.0, 100.0);
        c->setTargetSpeed(10.0);
        lane->addVehicle(move(c), 0.0);
        road.update(1.0);
        assert(lane->getVehicleAt(0)->getPosition() > 0.0);   // thuc su da di chuyen
    }

    // 10. Road::getVehicleCount cong don tren nhieu Lane
    {
        Road road(3, "R3", 50.0);
        Lane* a = road.addLane(1, 0.0);
        Lane* b = road.addLane(2, 0.0);
        a->addVehicle(make_unique<Car>(1, 0.0, 100.0), 0.0);
        a->addVehicle(make_unique<Car>(2, 0.0, 100.0), 10.0);
        b->addVehicle(make_unique<Car>(3, 0.0, 100.0), 0.0);
        assert(road.getVehicleCount() == 3);
    }

    // 11. On dinh con tro: them nhieu Lane (buoc vector phai realloc) khong lam hong
    //     cac Lane* da tra ve truoc do (vi dung vector<unique_ptr<Lane>>)
    {
        Road road(4, "R4", 50.0);
        Lane* first = road.addLane(1, 0.0);
        for (int i = 2; i <= 30; i++) road.addLane(i, 0.0);   // ep vector realloc nhieu lan
        assert(road.getLane(0) == first);        // van la cung dia chi
        assert(first->getId() == 1);              // va van con nguyen ven
    }

    // 12. Intersection::addApproach: cac dieu kien bi tu choi / nem exception
    {
        Road road(5, "R5", 50.0);
        Lane* in1 = road.addLane(1, 0.0);
        Lane* out1 = road.addLane(2, 0.0);

        Intersection inter(1, "I1");
        assert(inter.addApproach(nullptr, out1, 5, 5, 2) == nullptr);     // incoming null
        assert(inter.addApproach(in1, in1, 5, 5, 2) == nullptr);          // incoming == outgoing

        threw = false;
        try { inter.addApproach(in1, out1, 0, 5, 2); }                    // thoi luong <=0
        catch (const invalid_argument&) { threw = true; }
        assert(threw);
        assert(!in1->hasTrafficLight());   // that bai do exception -> khong duoc gan den (an toan)

        threw = false;
        try { inter.addApproach(in1, out1, 5, 5, 2, -1.0); }              // phaseOffset < 0
        catch (const invalid_argument&) { threw = true; }
        assert(threw);

        threw = false;
        try { inter.addApproach(in1, out1, 5, 5, 2, numeric_limits<double>::infinity()); }   // phaseOffset vo cuc (truoc day treo)
        catch (const invalid_argument&) { threw = true; }
        assert(threw);
        assert(!in1->hasTrafficLight());

        TrafficLight* light = inter.addApproach(in1, out1, 10, 5, 2);
        assert(light != nullptr);
        assert(in1->hasTrafficLight());
        assert(inter.addApproach(in1, out1, 5, 5, 2) == nullptr);         // in1 da co den roi

        assert(inter.getApproachCount() == 1);
        assert(inter.getLight(0) == light);
        assert(inter.getLight(99) == nullptr);   // sai index
    }

    // 13. phaseOffset thuc su tua dong ho cua den di truoc
    {
        Road road(6, "R6", 50.0);
        Lane* in1 = road.addLane(1, 0.0);
        Lane* out1 = road.addLane(2, 0.0);
        Intersection inter(2, "I2");
        TrafficLight* light = inter.addApproach(in1, out1, 10, 5, 2, 3.0);
        assert(light->getColor() == LightColor::Red);
        assert(fabs(light->getTimeRemaining() - 7.0) < 1e-9);   // 10 - 3 = 7
    }

    // 14. Intersection::update chuyen xe dung tu lan vao sang lan ra, giu nguyen overshoot
    {
        Road road(7, "R7", 20.0);
        Lane* in1 = road.addLane(1, 0.0);
        Lane* out1 = road.addLane(2, 0.0);
        Intersection inter(3, "I3");
        inter.addApproach(in1, out1, 5, 5, 2);   // den do luc dau -> se chan xe o vach

        auto c = make_unique<Car>(1, 0.0, 100.0);
        c->setTargetSpeed(20.0);
        in1->addVehicle(move(c), 18.0);   // dat gan vach san de nhanh vuot qua

        int tick = 0;
        while (in1->getVehicleCount() > 0 && tick < 50) {
            inter.update(1.0);
            in1->update(1.0);
            tick++;
        }
        assert(in1->getVehicleCount() == 0);    // xe da roi khoi lan vao
        assert(out1->getVehicleCount() == 1);   // va xuat hien trong lan ra
        assert(out1->getVehicleAt(0)->getPosition() >= 0.0);
    }

    // 15. Intersection::update voi outgoing == nullptr: xe roi khoi he thong (bi huy), khong con trong he thong
    {
        Road road(8, "R8", 10.0);
        Lane* in1 = road.addLane(1, 0.0);
        Intersection inter(4, "I4");
        inter.addApproach(in1, nullptr, 5, 5, 2);

        auto c = make_unique<Car>(1, 0.0, 100.0);
        c->setTargetSpeed(20.0);
        in1->addVehicle(move(c), 9.0);

        int tick = 0;
        while (in1->getVehicleCount() > 0 && tick < 50) {
            inter.update(1.0);
            in1->update(1.0);
            tick++;
        }
        assert(in1->getVehicleCount() == 0);   // xe da bi huy, khong con trong he thong nua
    }

    // 16. Intersection::update: lan ra day -> xe o lai lan vao, khong bi mat
    {
        Road road(9, "R9", 10.0);
        Lane* in1 = road.addLane(1, 0.0);
        Lane* out1 = road.addLane(2, 0.0);
        Intersection inter(5, "I5", /*minEntryGap=*/5.0);
        inter.addApproach(in1, out1, 5, 5, 2);

        out1->addVehicle(make_unique<Car>(99, 0.0, 100.0), 0.0);   // chan het cho o dau lan ra

        auto c = make_unique<Car>(1, 0.0, 100.0);
        c->setTargetSpeed(20.0);
        in1->addVehicle(move(c), 9.0);

        for (int i = 0; i < 10; i++) { inter.update(1.0); in1->update(1.0); }

        assert(in1->getVehicleCount() == 1);    // xe van con trong lan vao, khong bi mat
        assert(out1->getVehicleCount() == 1);   // lan ra khong bi them xe moi vao
    }

    cout << "Tat ca test infrastructure dat\n";
    return 0;
}