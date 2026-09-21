#ifndef TRAFFICLIGHTSTATE_H
#define TRAFFICLIGHTSTATE_H

#include <memory>
#include <string>

// Shared durations so any state can build the next one without
// TrafficLight having to know which duration belongs to which color.
struct TrafficLightConfig {
    double redDuration;
    double greenDuration;
    double yellowDuration;
};

class TrafficLightState {
protected:
    TrafficLightConfig config;
    double timer; // time remaining in this state

public:
    TrafficLightState(const TrafficLightConfig& cfg, double duration)
        : config(cfg), timer(duration) {}
    virtual ~TrafficLightState() = default;

    // Shared behavior: every state just counts down the same way.
    void update(double dt) {
        if (dt < 0) { return; }
        timer -= dt;
    }
    bool isFinished() const { return timer <= 0; }
    double getTimeRemaining() const { return timer; }

    // State-specific behavior, delegated to each subclass.
    virtual std::string getName() const = 0;
    virtual std::unique_ptr<TrafficLightState> nextState() const = 0;
};

#endif
