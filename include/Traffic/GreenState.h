#ifndef GREENSTATE_H
#define GREENSTATE_H

#include "TrafficLightState.h"

class GreenState : public TrafficLightState {
public:
    explicit GreenState(const TrafficLightConfig& cfg)
        : TrafficLightState(cfg, cfg.greenDuration) {}

    std::string getName() const override { return "GREEN"; }
    std::unique_ptr<TrafficLightState> nextState() const override;
};

#endif
