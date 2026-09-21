#include "GreenState.h"
#include "YellowState.h"

std::unique_ptr<TrafficLightState> GreenState::nextState() const {
    return std::make_unique<YellowState>(config);
}
