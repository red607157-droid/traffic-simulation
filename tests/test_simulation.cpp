#include <iostream>
#include <cmath>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include "Simulation/Simulation.h"
#include "Infrastructure/Road.h"
#include "Infrastructure/Intersection.h"
#include "Vehicle/Car.h"
#include "Vehicle/Bus.h"
using namespace std;
int main(){
  // 1. dt <= 0 bi tu choi
  bool thrown=false; try{ Simulation s(0); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
  thrown=false; try{ Simulation s(-1); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
  // 2. Thoi gian va so buoc tang dung (khong co thanh phan nao van chay duoc)
  Simulation s(0.5); assert(s.getTime()==0 && s.getStepCount()==0);
  for(int i=0;i<5;i++) s.step();
  assert(s.getStepCount()==5 && fabs(s.getTime()-2.5)<1e-9);
  // 3. addComponent(nullptr) bi tu choi
  thrown=false; try{ s.addComponent(nullptr); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
  // 4. step() update cac thanh phan: xe tren Road chay toi, dem xe qua Simulation
  {
    Simulation q(1);
    auto road=make_unique<Road>(1,"R",1000.0); Lane* lane=road->addLane(1);
    auto car=make_unique<Car>(1,0,100); car->setTargetSpeed(20); Car* c=car.get();
    lane->addVehicle(move(car),0);
    q.addComponent(move(road));
    assert(q.getVehicleCount()==1);
    for(int i=0;i<5;i++) q.step();
    assert(c->getPosition()>0);
  }
  // 5. Den do rat dai: xe dung o vach cuoi lan (500m), xe sau khong dam xe truoc
  {
    Simulation r(1);
    auto road=make_unique<Road>(1,"A",500.0); Lane* lane=road->addLane(1);
    auto car=make_unique<Car>(1,0,100); car->setTargetSpeed(20); Car* c=car.get();
    auto bus=make_unique<Bus>(2,0,200,30); bus->setTargetSpeed(15); Bus* b=bus.get();
    lane->addVehicle(move(car),400); lane->addVehicle(move(bus),370);
    auto inter=make_unique<Intersection>(1,"I"); inter->addApproach(lane,nullptr,1000,5,2);
    r.addComponent(move(road)); r.addComponent(move(inter));
    for(int i=0;i<60;i++){ r.step(); assert(b->getPosition()<c->getPosition()); }
    cout<<"car pos="<<c->getPosition()<<" state="<<c->getState()->getName()<<" | bus pos="<<b->getPosition()<<"\n";
    assert(c->getPosition()<=500+1e-6 && c->getSpeed()==0 && string(c->getState()->getName())=="WaitingAtLight");
    assert(c->getPosition()-b->getPosition()>=b->getSafeDistance()-1e-6);
    assert(r.getVehicleCount()==2);
  }
  // 6. Den xanh: xe qua nut giao sang Road B, tong so xe khong doi
  {
    Simulation g(1);
    auto roadA=make_unique<Road>(1,"A",50.0); Lane* la=roadA->addLane(1); Road* ra=roadA.get();
    auto roadB=make_unique<Road>(2,"B",500.0); Lane* lb=roadB->addLane(2); Road* rb=roadB.get();
    auto car=make_unique<Car>(1,0,100); car->setTargetSpeed(10);
    la->addVehicle(move(car),40);
    auto inter=make_unique<Intersection>(1,"I");
    inter->addApproach(la,lb,5,100,3,/*phaseOffset=*/5);   // tua qua het den do -> bat dau la den xanh
    g.addComponent(move(roadA)); g.addComponent(move(roadB)); g.addComponent(move(inter));
    for(int i=0;i<5;i++) g.step();
    assert(ra->getVehicleCount()==0 && rb->getVehicleCount()==1 && g.getVehicleCount()==1);
  }
  // 7. run() voi so buoc am bi tu choi
  thrown=false; try{ s.run(-1); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
  cout<<"Tat ca test simulation dat\n";
}
