#include <iostream>
#include <cmath>
#include <cassert>
#include "Vehicle/Car.h"
#include "Vehicle/Bus.h"
#include "Vehicle/Motorbike.h"
#include "Traffic/TrafficLight.h"
using namespace std;
int main(){
  // 1. dt=0 khong con chia cho 0 (chay duoi UBSan float-divide-by-zero)
  Car a(1,0,100), b(2,0,100); a.setPosition(100); b.setPosition(95); b.setTargetSpeed(20); b.setVehicleAhead(&a);
  double d=b.getDesiredSpeed(0.0); assert(!std::isnan(d)); b.accelerate();
  // 2. Het xang -> xe phanh dung, fuel khong am, khong chay tiep
  Car c(3,0,5.0); c.setTargetSpeed(20);   // 5 / 0.08 = ~62m
  for(int i=0;i<60;i++){ c.update(1); assert(c.getFuel()>=0); }
  cout<<"car pos="<<c.getPosition()<<" v="<<c.getSpeed()<<" fuel="<<c.getFuel()<<" state="<<c.getState()->getName()<<"\n";
  assert(c.getSpeed()==0 && c.getFuel()==0 && std::string(c.getState()->getName())=="Stopped");
  // 3. Tieu hao dung ty le quang duong
  Bus bus(4,0,200,30); bus.setTargetSpeed(10); double p0=bus.getPosition(); for(int i=0;i<10;i++)bus.update(1);
  double expect=200-0.15*(bus.getPosition()-p0); cout<<"bus fuel="<<bus.getFuel()<<" expected="<<expect<<"\n"; assert(fabs(bus.getFuel()-expect)<1e-9);
  // 4. Xe dung yen khong ton xang
  Motorbike m(5,0,125,30); for(int i=0;i<5;i++)m.update(1); assert(m.getFuel()==30);
  // 5. Nap xang -> chay lai
  c.setFuel(50); for(int i=0;i<5;i++)c.update(1); assert(c.getSpeed()>0); cout<<"refuel ok, v="<<c.getSpeed()<<"\n";
  // 6. setFuel am bi chan ve 0
  c.setFuel(-3); assert(c.getFuel()==0);
  // 7. Den: enum
  TrafficLight l(1,2,2,1); assert(l.getColor()==LightColor::Red); l.update(2); assert(l.getColor()==LightColor::Green); l.update(2); assert(l.getColor()==LightColor::Yellow);
  cout<<"Tat ca test dat\n";
}
