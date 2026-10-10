#include "Traffic/GreenState.h"
#include "Traffic/YellowState.h"

std::unique_ptr<TrafficLightState> GreenState::nextState() const {
    return std::make_unique<YellowState>(config);
}
