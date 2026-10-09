#ifndef REDSTATE_H
#define REDSTATE_H

#include "Traffic/TrafficLightState.h"

class RedState : public TrafficLightState {
public:
    explicit RedState(const TrafficLightConfig& cfg)
        : TrafficLightState(cfg, cfg.redDuration) {}

    LightColor getColor() const override { return LightColor::Red; }
    std::string getName() const override { return "RED"; }
    std::unique_ptr<TrafficLightState> nextState() const override;
};

#endif
