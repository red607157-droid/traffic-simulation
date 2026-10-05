#include "Traffic/YellowState.h"
#include "Traffic/RedState.h"

std::unique_ptr<TrafficLightState> YellowState::nextState() const {
    return std::make_unique<RedState>(config);
}
