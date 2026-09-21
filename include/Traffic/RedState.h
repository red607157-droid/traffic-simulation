#ifndef REDSTATE_H
#define REDSTATE_H

#include "TrafficLightState.h"

class RedState : public TrafficLightState {
public:
    explicit RedState(const TrafficLightConfig& cfg)
        : TrafficLightState(cfg, cfg.redDuration) {}

    std::string getName() const override { return "RED"; }
    std::unique_ptr<TrafficLightState> nextState() const override;
};

#endif
