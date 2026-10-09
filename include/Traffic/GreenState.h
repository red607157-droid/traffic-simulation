#ifndef GREENSTATE_H
#define GREENSTATE_H

#include "Traffic/TrafficLightState.h"

class GreenState : public TrafficLightState {
public:
    explicit GreenState(const TrafficLightConfig& cfg)
        : TrafficLightState(cfg, cfg.greenDuration) {}

    LightColor getColor() const override { return LightColor::Green; }
    std::string getName() const override { return "GREEN"; }
    std::unique_ptr<TrafficLightState> nextState() const override;
};

#endif
