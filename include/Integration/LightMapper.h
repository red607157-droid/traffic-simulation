#ifndef LIGHTMAPPER_H
#define LIGHTMAPPER_H

#include "Vehicle/Vehicle.h"
#include "Traffic/TrafficLight.h"

// Chuyển màu của TrafficLight (LightColor) sang tín hiệu mà Vehicle hiểu (Vehicle::trafficLight).
Vehicle::trafficLight LightMapper(const TrafficLight& tfl);

#endif
