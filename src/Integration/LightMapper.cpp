#include "Integration/LightMapper.h"

Vehicle::trafficLight LightMapper(const TrafficLight& tfl) {
    switch (tfl.getColor()) {
        case LightColor::Red:    return Vehicle::redLight;
        case LightColor::Green:  return Vehicle::greenLight;
        case LightColor::Yellow: return Vehicle::yellowLight;
    }
    return Vehicle::unknownLight;   // an toàn: coi như đèn đỏ (xem Vehicle::getLightLimit)
}
