#ifndef YELLOWSTATE_H
#define YELLOWSTATE_H

#include "TrafficLightState.h"

class YellowState : public TrafficLightState {
public:
    explicit YellowState(const TrafficLightConfig& cfg)
        : TrafficLightState(cfg, cfg.yellowDuration) {}

    std::string getName() const override { return "YELLOW"; }
    std::unique_ptr<TrafficLightState> nextState() const override;
};

#endif
