#include "Integration/LightMapper.h"
#include "Vehicle/Vehicle.h"
#include<string>


Vehicle::trafficLight LightMapper(const TrafficLight &tfl){
    std::string state = tfl.getStateName();
    if(state == "RED")           {return Vehicle::redLight;}
    else if(state == "GREEN")    { return Vehicle::greenLight;}
    else if(state == "YELLOW")   { return Vehicle::yellowLight;}
    else                         { return Vehicle::unknownLight;}
}