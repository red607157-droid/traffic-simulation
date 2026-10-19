#undef NDEBUG   // luôn bật assert, kể cả khi build với -DNDEBUG (nếu không, test sẽ "đạt" mà không kiểm tra gì)
#include <iostream>
#include <cmath>
#include <cassert>
#include <chrono>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include "Simulation/Simulation.h"
#include "Infrastructure/Road.h"
#include "Infrastructure/Intersection.h"
#include "Vehicle/Car.h"
#include "Vehicle/Bus.h"
#include "Vehicle/Motorbike.h"
using namespace std;

bool near(double a, double b){ return fabs(a-b)<1e-6; }

// Road "hong": update() luon nem exception -> dung de thu cach Simulation xu ly loi trong cac thread.
class ThrowingRoad : public Road {
public:
  ThrowingRoad(int _id) : Road(_id, "Hong", 100.0) {}
  void update(double) override { throw runtime_error("loi trong Road::update"); }
};

// Kich ban nhieu Road de thu da luong: 4 Road; A -> B qua nut giao I1, C -> D qua nut giao I2.
// cars: con tro (KHONG so huu) toi tung xe de so sanh ket qua.
void buildCity(Simulation& sim, vector<Vehicle*>& cars){
  auto rA=make_unique<Road>(1,"A",300.0); Lane* la=rA->addLane(1);
  auto rB=make_unique<Road>(2,"B",300.0); Lane* lb=rB->addLane(2);
  auto rC=make_unique<Road>(3,"C",250.0); Lane* lc=rC->addLane(3);
  auto rD=make_unique<Road>(4,"D",400.0); Lane* ld=rD->addLane(4);
  const double pos[]={250,220,190, 200,170,140};
  const double target[]={20,15,18, 16,22,14};
  for(int i=0;i<6;i++){
    unique_ptr<Vehicle> v;
    if(i%3==0) v=make_unique<Car>(i+1,0,100);
    else if(i%3==1) v=make_unique<Bus>(i+1,0,200,30);
    else v=make_unique<Motorbike>(i+1,0,125,30);
    v->setTargetSpeed(target[i]);
    cars.push_back(v.get());
    (i<3 ? la : lc)->addVehicle(move(v),pos[i]);
  }
  auto i1=make_unique<Intersection>(1,"I1"); i1->addApproach(la,lb,8,6,2);
  auto i2=make_unique<Intersection>(2,"I2"); i2->addApproach(lc,ld,5,7,2,/*phaseOffset=*/3);
  sim.addRoad(move(rA)); sim.addRoad(move(rB)); sim.addRoad(move(rC)); sim.addRoad(move(rD));
  sim.addIntersection(move(i1)); sim.addIntersection(move(i2));
}

// Road A (50m) -> nut giao (den xanh ngay tu dau) -> Road B. Tra ve buoc ma xe sang toi Road B.
int stepReachingRoadB(bool roadsFirst){
  auto roadA=make_unique<Road>(1,"A",50.0); Lane* la=roadA->addLane(1);
  auto roadB=make_unique<Road>(2,"B",500.0); Lane* lb=roadB->addLane(2); Road* rb=roadB.get();
  auto car=make_unique<Car>(1,0,100); car->setTargetSpeed(10);
  la->addVehicle(move(car),40);
  auto inter=make_unique<Intersection>(1,"I");
  inter->addApproach(la,lb,5,100,3,/*phaseOffset=*/5);
  Simulation sim(1);
  if(roadsFirst){ sim.addRoad(move(roadA)); sim.addRoad(move(roadB)); sim.addIntersection(move(inter)); }
  else          { sim.addIntersection(move(inter)); sim.addRoad(move(roadA)); sim.addRoad(move(roadB)); }
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
  // 3. addRoad(nullptr) / addIntersection(nullptr) bi tu choi
  thrown=false; try{ s.addRoad(nullptr); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
  thrown=false; try{ s.addIntersection(nullptr); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
  // 4. step() update cac thanh phan: xe tren Road chay toi, dem xe qua Simulation
  {
    Simulation q(1);
    auto road=make_unique<Road>(1,"R",1000.0); Lane* lane=road->addLane(1);
    auto car=make_unique<Car>(1,0,100); car->setTargetSpeed(20); Car* c=car.get();
    lane->addVehicle(move(car),0);
    q.addRoad(move(road));
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
    r.addRoad(move(road)); r.addIntersection(move(inter));
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
    g.addRoad(move(roadA)); g.addRoad(move(roadB)); g.addIntersection(move(inter));
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
  // 9. Thu tu update: Simulation tu chay Road truoc, Intersection sau -> them theo thu tu nao cung ra buoc 3
  assert(stepReachingRoadB(true)==3);
  assert(stepReachingRoadB(false)==3);
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
    sim.addRoad(move(roadA)); sim.addRoad(move(roadB)); sim.addIntersection(move(inter));
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
  // 11. Phase 10 - runRealTime: tham so sai bi tu choi
  {
    Simulation t(1);
    thrown=false; try{ t.runRealTime(-1); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
    thrown=false; try{ t.runRealTime(5, 0); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
    thrown=false; try{ t.runRealTime(5, -2); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
    // khong phai boi so cua dt: dt=1 ma xin 2.5s -> bao loi, khong lang le cat con 2 buoc
    thrown=false; try{ t.runRealTime(2.5); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
    // vo cuc / NaN / qua lon
    const double INF=numeric_limits<double>::infinity(), NaN=numeric_limits<double>::quiet_NaN();
    thrown=false; try{ t.runRealTime(INF); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
    thrown=false; try{ t.runRealTime(NaN); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
    thrown=false; try{ t.runRealTime(1e12); }catch(const invalid_argument&){ thrown=true; } assert(thrown);   // so buoc > INT_MAX
    thrown=false; try{ t.runRealTime(5, INF); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
    thrown=false; try{ t.runRealTime(5, NaN); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
    thrown=false; try{ t.runRealTime(5, 1e300); }catch(const invalid_argument&){ thrown=true; } assert(thrown);   // tick lam tron ve 0
    thrown=false; try{ t.runRealTime(5, 1e-300); }catch(const invalid_argument&){ thrown=true; } assert(thrown);  // tick > 1 gio
    thrown=false; try{ Simulation bad(INF); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
    thrown=false; try{ Simulation bad(NaN); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
    assert(t.getStepCount()==0);   // cac lan goi sai khong chay buoc nao
    t.runRealTime(0);              // 0 giay: hop le, khong chay buoc nao
    assert(t.getStepCount()==0);
    Simulation f(0.1);
    f.runRealTime(0.3, 1000);      // 0.3 / 0.1 = 2.9999999999999996 tren may -> van tinh la 3 buoc
    assert(f.getStepCount()==3);
  }
  // 12. Phase 10 - runRealTime: thoi gian MO PHONG dung, thoi gian THAT xap xi simSeconds / speed, va chay TIEP
  {
    Simulation t(0.5);
    auto start=chrono::steady_clock::now();
    t.runRealTime(5, 50);   // 5s mo phong, x50 -> ~0.1s that, 10 buoc
    double real=chrono::duration<double>(chrono::steady_clock::now()-start).count();
    assert(t.getStepCount()==10 && near(t.getTime(),5));
    // Thoi gian MO PHONG da kiem tra chinh xac o tren. Thoi gian THAT phu thuoc may nen chi kiem tra rong:
    // khong chay NHANH hon thoi gian that (can duoi), can tren rat rong (30s) chi de bat truong hop bi treo.
    assert(real>=0.09 && real<30.0);
    t.runRealTime(2, 50);
    assert(t.getStepCount()==14 && near(t.getTime(),7));
  }
  // 13. Phase 12 - so thread co gioi han: min(maxWorkers, so Road), khong phai 1 thread / xe
  {
    vector<Vehicle*> cars;
    Simulation a(1,4); buildCity(a,cars); assert(a.getWorkerCount()==4);
    Simulation b(1,3); buildCity(b,cars); assert(b.getWorkerCount()==3);
    Simulation c(1,8); buildCity(c,cars); assert(c.getWorkerCount()==4);   // chi co 4 Road
    Simulation d(1);   buildCity(d,cars); assert(d.getWorkerCount()>=1 && d.getWorkerCount()<=4);
    Simulation e(1,4); assert(e.getWorkerCount()==0);                       // chua co Road nao
  }
  // 14. Phase 12 - song song = tuan tu: 4 thread cho ket qua GIONG HET 1 thread o MOI buoc (lap 20 lan)
  for(int rep=0; rep<20; rep++){
    vector<Vehicle*> seqCars, parCars;
    Simulation seq(1,1); buildCity(seq,seqCars);
    Simulation par(1,4); buildCity(par,parCars);
    assert(par.getWorkerCount()==4);
    for(int st=1; st<=80; st++){
      seq.step(); par.step();
      for(size_t i=0;i<seqCars.size();i++){
        assert(seqCars[i]->getPosition()==parCars[i]->getPosition());
        assert(seqCars[i]->getSpeed()==parCars[i]->getSpeed());
      }
    }
    assert(seq.getVehicleCount()==par.getVehicleCount());
  }
  // 15. Phase 12 - start()/stop(): chay nen, cac ham khac bi chan khi dang chay, ket qua van dung
  {
    vector<Vehicle*> bgCars, refCars;
    Simulation bg(1,4); buildCity(bg,bgCars);
    thrown=false; try{ bg.start(0); }catch(const invalid_argument&){ thrown=true; } assert(thrown && !bg.isRunning());
    // speed sai bi tu choi TRUOC khi tao thread nen (khong de exception xay ra trong thread)
    thrown=false; try{ bg.start(numeric_limits<double>::infinity()); }catch(const invalid_argument&){ thrown=true; } assert(thrown && !bg.isRunning());
    thrown=false; try{ bg.start(1e300); }catch(const invalid_argument&){ thrown=true; } assert(thrown && !bg.isRunning());   // tick lam tron ve 0
    thrown=false; try{ bg.start(numeric_limits<double>::quiet_NaN()); }catch(const invalid_argument&){ thrown=true; } assert(thrown && !bg.isRunning());
    thrown=false; try{ bg.start(1e-300); }catch(const invalid_argument&){ thrown=true; } assert(thrown && !bg.isRunning());  // tick > 1 gio
    assert(bg.getStepCount()==0);   // khong co thread nen nao da chay buoc nao
    bg.start(1000);   // 1 buoc = 1ms that
    assert(bg.isRunning());
    thrown=false; try{ bg.start(1000); }catch(const logic_error&){ thrown=true; } assert(thrown);
    thrown=false; try{ bg.step(); }catch(const logic_error&){ thrown=true; } assert(thrown);
    thrown=false; try{ bg.addRoad(make_unique<Road>(9,"X",10.0)); }catch(const logic_error&){ thrown=true; } assert(thrown);
    thrown=false; try{ bg.getVehicleCount(); }catch(const logic_error&){ thrown=true; } assert(thrown);
    this_thread::sleep_for(chrono::milliseconds(50));
    assert(bg.getTime()>=0);   // getTime() doc duoc trong luc chay (atomic)
    bg.stop();
    assert(!bg.isRunning());
    bg.stop();   // goi lai khong sao
    int n=bg.getStepCount();
    assert(n>0 && near(bg.getTime(),n));
    // chay tuan tu dung so buoc do -> phai ra cung ket qua
    Simulation ref(1,1); buildCity(ref,refCars);
    for(int i=0;i<n;i++) ref.step();
    for(size_t i=0;i<bgCars.size();i++) assert(bgCars[i]->getPosition()==refCars[i]->getPosition());
    // start lai: chay TIEP
    bg.start(1000); this_thread::sleep_for(chrono::milliseconds(20)); bg.stop();
    assert(bg.getStepCount()>n);
  }
  // 16. Phase 12 - huy Simulation khi dang chay nen: destructor tu stop() + join, khong bi std::terminate
  {
    vector<Vehicle*> cars;
    Simulation d(1,2); buildCity(d,cars);
    d.start(1000);
    this_thread::sleep_for(chrono::milliseconds(10));
  }
  // 17. Exception trong thread cua GD 1 khong lam chuong trinh dung (std::terminate): step() nem lai o thread chinh
  for(unsigned workers : {1u, 4u}){
    Simulation s2(1, workers);
    s2.addRoad(make_unique<Road>(1,"A",100.0));
    s2.addRoad(make_unique<ThrowingRoad>(2));
    s2.addRoad(make_unique<Road>(3,"C",100.0));
    thrown=false; try{ s2.step(); }catch(const runtime_error&){ thrown=true; } assert(thrown);
    assert(s2.getStepCount()==0);   // buoc loi khong duoc tinh
  }
  // 18. Exception trong thread NEN: thread tu dung, start() bi chan toi khi goi stop(), stop() nem lai loi do
  {
    Simulation s3(1,2);
    s3.addRoad(make_unique<Road>(1,"A",100.0));
    s3.addRoad(make_unique<ThrowingRoad>(2));
    s3.start(1000);
    for(int i=0; i<2000 && s3.isRunning(); i++) this_thread::sleep_for(chrono::milliseconds(1));
    assert(!s3.isRunning());                                   // tu dung vi loi
    thrown=false; try{ s3.start(1000); }catch(const logic_error&){ thrown=true; } assert(thrown);   // phai lay loi truoc
    thrown=false; try{ s3.stop(); }catch(const runtime_error&){ thrown=true; } assert(thrown);      // loi duoc nem lai
    s3.stop();                                                 // goi lai: khong con loi
  }
  // 19. stop() dung NGAY, khong cho het tick: speed=0.001 -> 1 tick = 1000 giay that
  {
    vector<Vehicle*> cars;
    Simulation s4(1,2); buildCity(s4,cars);
    s4.start(0.001);
    this_thread::sleep_for(chrono::milliseconds(20));
    auto t0=chrono::steady_clock::now();
    s4.stop();
    double waited=chrono::duration<double>(chrono::steady_clock::now()-t0).count();
    assert(waited<1.0);                 // truoc day phai cho toi 1000 giay
    assert(s4.getStepCount()==0);       // chua toi tick dau tien
  }
  // 20. Huy Simulation khi thread nen da dung vi loi ma chua stop(): destructor khong nem exception
  {
    Simulation s5(1,2);
    s5.addRoad(make_unique<ThrowingRoad>(1));
    s5.start(1000);
    this_thread::sleep_for(chrono::milliseconds(30));
  }
  cout<<"Tat ca test simulation dat\n";
}
