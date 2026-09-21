#include "RedState.h"
#include "GreenState.h"

std::unique_ptr<TrafficLightState> RedState::nextState() const {
    return std::make_unique<GreenState>(config);
}
