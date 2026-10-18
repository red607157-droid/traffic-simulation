#undef NDEBUG   // luôn bật assert, kể cả khi build với -DNDEBUG (nếu không, test sẽ "đạt" mà không kiểm tra gì)
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
#include "Vehicle/Motorbike.h"
using namespace std;

bool near(double a, double b){ return fabs(a-b)<1e-6; }

// Road A (50m) -> nut giao (den xanh ngay tu dau) -> Road B. Tra ve buoc ma xe sang toi Road B.
int stepReachingRoadB(bool roadsFirst){
  auto roadA=make_unique<Road>(1,"A",50.0); Lane* la=roadA->addLane(1);
  auto roadB=make_unique<Road>(2,"B",500.0); Lane* lb=roadB->addLane(2); Road* rb=roadB.get();
  auto car=make_unique<Car>(1,0,100); car->setTargetSpeed(10);
  la->addVehicle(move(car),40);
  auto inter=make_unique<Intersection>(1,"I");
  inter->addApproach(la,lb,5,100,3,/*phaseOffset=*/5);
  Simulation sim(1);
  if(roadsFirst){ sim.addComponent(move(roadA)); sim.addComponent(move(roadB)); sim.addComponent(move(inter)); }
  else          { sim.addComponent(move(inter)); sim.addComponent(move(roadA)); sim.addComponent(move(roadB)); }
  for(int st=1; st<=20; st++){ sim.step(); if(rb->getVehicleCount()==1) return st; }
  return -1;
}

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
  // 8. Goi run() 2 lan: chay TIEP, khong reset (time va stepCount cong don)
  {
    Simulation t(1);
    t.run(2); t.run(3);
    assert(t.getStepCount()==5 && near(t.getTime(),5));
  }
  // 9. Thu tu update: Road truoc -> xe sang Road B o buoc 3; Intersection truoc -> cham hon dung 1 buoc
  assert(stepReachingRoadB(true)==3);
  assert(stepReachingRoadB(false)==4);
  // 10. Hoi quy: kich ban giong main.cpp, khoa lai vi tri / toc do / mau den o cac buoc co dinh
  {
    auto roadA=make_unique<Road>(1,"A",500.0); Lane* la=roadA->addLane(1); Road* ra=roadA.get();
    auto roadB=make_unique<Road>(2,"B",500.0); Lane* lb=roadB->addLane(2); Road* rb=roadB.get();
    auto car=make_unique<Car>(1,0,100); auto bus=make_unique<Bus>(2,0,200,30); auto bike=make_unique<Motorbike>(3,0,125,30);
    car->setTargetSpeed(20); bus->setTargetSpeed(15); bike->setTargetSpeed(18);
    Vehicle* c=car.get(); Vehicle* b=bus.get(); Vehicle* m=bike.get();
    la->addVehicle(move(car),400); la->addVehicle(move(bus),370); la->addVehicle(move(bike),350);
    auto inter=make_unique<Intersection>(1,"I1"); TrafficLight* light=inter->addApproach(la,lb,12,10,3);
    Simulation sim(1);
    sim.addComponent(move(roadA)); sim.addComponent(move(roadB)); sim.addComponent(move(inter));
    for(int st=1; st<=36; st++){
      sim.step();
      if(st==11) assert(light->getColor()==LightColor::Red);
      if(st==12){
        assert(light->getColor()==LightColor::Green);
        assert(ra->getVehicleCount()==3 && rb->getVehicleCount()==0);
        assert(fabs(c->getPosition()-499.998714)<1e-5 && fabs(c->getSpeed()-0.113420)<1e-5);   // xe dau dung sat vach 500m
        assert(fabs(b->getPosition()-483.565670)<1e-5 && fabs(m->getPosition()-478.565670)<1e-5);
      }
      if(st==22) assert(light->getColor()==LightColor::Yellow);
      if(st==25) assert(light->getColor()==LightColor::Red);
    }
    assert(ra->getVehicleCount()==0 && rb->getVehicleCount()==3);
    assert(fabs(c->getPosition()-423.679232)<1e-5 && near(c->getSpeed(),20));
    assert(fabs(b->getPosition()-314.425896)<1e-5 && near(b->getSpeed(),15));
    assert(fabs(m->getPosition()-309.425896)<1e-5 && near(m->getSpeed(),15));
  }
  cout<<"Tat ca test simulation dat\n";
}
