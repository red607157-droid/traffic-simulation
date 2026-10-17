#include <iostream>
#include <cmath>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include "Simulation/Simulation.h"
#include "Vehicle/Car.h"
#include "Vehicle/Bus.h"
#include "Traffic/TrafficLight.h"
using namespace std;
int main(){
  // 1. dt <= 0 bi tu choi
  bool thrown=false; try{ Simulation s(0); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
  thrown=false; try{ Simulation s(-1); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
  // 2. Thoi gian va so buoc tang dung
  Simulation s(0.5); assert(s.getTime()==0 && s.getStepCount()==0);
  for(int i=0;i<5;i++) s.step();
  assert(s.getStepCount()==5 && fabs(s.getTime()-2.5)<1e-9);
  // 3. addVehicle noi xe truoc theo thu tu them; nullptr bi tu choi
  Simulation q(1);
  Vehicle& a=q.addVehicle(make_unique<Car>(1,0,100));
  Vehicle& b=q.addVehicle(make_unique<Bus>(2,0,200,30));
  assert(a.getVehicleAhead()==nullptr && b.getVehicleAhead()==&a && q.getVehicles().size()==2);
  thrown=false; try{ q.addVehicle(nullptr); }catch(const invalid_argument&){ thrown=true; } assert(thrown && q.getVehicles().size()==2);
  // 4. Den duoc update qua simulation (RED=3s -> GREEN)
  Simulation l(1); l.setTrafficLight(make_unique<TrafficLight>(1,3,5,2));
  for(int i=0;i<3;i++) l.step();
  assert(l.getTrafficLight()->getColor()==LightColor::Green);
  // 5. Den do rat dai: xe dung truoc vach 500m, xe sau khong dam xe truoc
  Simulation r(1); r.setTrafficLight(make_unique<TrafficLight>(1,1000,5,2));
  Vehicle& c=r.addVehicle(make_unique<Car>(1,0,100)); c.setPosition(400); c.setTargetSpeed(20);
  Vehicle& d=r.addVehicle(make_unique<Bus>(2,0,200,30)); d.setPosition(370); d.setTargetSpeed(15);
  for(int i=0;i<60;i++){ r.step(); assert(d.getPosition()<c.getPosition()); }
  cout<<"car pos="<<c.getPosition()<<" state="<<c.getState()->getName()<<" | bus pos="<<d.getPosition()<<"\n";
  assert(c.getPosition()<=500+1e-6 && c.getSpeed()==0 && string(c.getState()->getName())=="WaitingAtLight");
  assert(c.getPosition()-d.getPosition()>=d.getSafeDistance()-1e-6);
  // 6. Khong co den: xe chay qua vach (mac dinh den xanh)
  Simulation n(1); Vehicle& e=n.addVehicle(make_unique<Car>(1,0,100)); e.setPosition(450); e.setTargetSpeed(20);
  for(int i=0;i<10;i++) n.step(); assert(e.getPosition()>500);
  // 7. run() voi so buoc am bi tu choi
  thrown=false; try{ n.run(-1); }catch(const invalid_argument&){ thrown=true; } assert(thrown);
  cout<<"Tat ca test simulation dat\n";
}
