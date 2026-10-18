# Urban Traffic Simulation --- C++ Mini Project Roadmap

### Requirements & Roadmap v1 --- C++ / OOP / Design Patterns / Concurrency

*(From project requirements to implementation plan for Claude Code)*

------------------------------------------------------------------------

## 0. Project Goal

Build a console-based **real-time urban traffic simulation** in C++.

The simulation models:

-   Vehicles: Car, Bus, Motorbike
-   Traffic lights
-   Lanes
-   Roads
-   Intersections
-   A simulation engine / clock

The project must demonstrate:

1.  Object-Oriented Programming
2.  Polymorphism and inheritance
3.  State Pattern
4.  Composite Pattern
5.  C++ concurrency / multithreading
6.  Thread safety and synchronization
7.  Clean ownership using smart pointers
8.  A continuously running simulation loop

### Important implementation rule

**Do not rewrite the existing `miniProject.cpp` from scratch.**

Use the existing vehicle hierarchy as the starting point and evolve it
incrementally.

The current project already contains:

``` text
Vehicle
├── Car
├── Bus
└── Motorbike
```

It also already uses:

-   abstract base class
-   virtual functions
-   inheritance
-   polymorphism
-   `std::vector`
-   `std::unique_ptr`
-   a simple simulation loop

The current vehicle implementation and simulation loop are the baseline
for this roadmap.

------------------------------------------------------------------------

# 1. Current Starting Point

The existing code has an abstract `Vehicle` class with:

-   `id`
-   `speed`
-   `move(dt)`
-   `display()`
-   `getMaxSpeed()`

`Car`, `Bus`, and `Motorbike` inherit from `Vehicle` and implement their
own movement/display behavior.

The `main()` function stores different vehicle types in:

``` cpp
vector<unique_ptr<Vehicle>>
```

and updates them repeatedly.

This is the foundation. Preserve the useful structure while refactoring
it when later requirements need it.

------------------------------------------------------------------------

# 2. Functional Requirements

  ID    Requirement
  ----- --------------------------------------------------------------
  F1    Create and manage different vehicle types
  F2    Vehicles have independent state such as speed and position
  F3    Vehicles can change behavior according to traffic conditions
  F4    Traffic lights have RED, GREEN, and YELLOW states
  F5    Vehicles react to traffic-light states
  F6    A lane contains vehicles
  F7    A road contains one or more lanes
  F8    An intersection connects roads and contains traffic lights
  F9    Infrastructure can be represented hierarchically
  F10   Simulation runs continuously through discrete time steps
  F11   Multiple simulation activities can run concurrently
  F12   Shared data is protected from race conditions
  F13   Simulation status can be displayed in the console
  F14   Program can start and stop cleanly

------------------------------------------------------------------------

# 3. Non-Functional Requirements

-   Use modern C++ where practical.
-   Prefer RAII and smart pointers over manual `new` / `delete`.
-   Keep classes small and focused.
-   Avoid global mutable state.
-   Avoid unnecessary inheritance.
-   Use interfaces/abstract classes when polymorphism is genuinely
    useful.
-   Do not introduce design patterns merely to satisfy a label; each
    pattern must solve a concrete design problem.
-   Do not create one OS thread per vehicle.
-   Keep concurrency understandable and bounded.
-   No `System.out`-style debugging; use clear C++ console output.
-   Keep the project buildable after every phase.

------------------------------------------------------------------------

# 4. Target Architecture

Eventually the project should roughly look like:

``` text
TrafficSimulation
│
├── Simulation
│
├── Vehicles
│   ├── Vehicle
│   ├── Car
│   ├── Bus
│   └── Motorbike
│
├── Vehicle States
│   ├── VehicleState
│   ├── DrivingState
│   ├── BrakingState
│   └── StoppedState
│
├── Traffic Lights
│   ├── TrafficLight
│   ├── TrafficLightState
│   ├── RedState
│   ├── GreenState
│   └── YellowState
│
├── Infrastructure
│   ├── RoadComponent
│   ├── Lane
│   ├── Road
│   └── Intersection
│
└── Concurrency
    ├── Simulation tasks
    ├── synchronization
    └── controlled shared state
```

Do not create every class immediately. Build them phase by phase.

------------------------------------------------------------------------

# 5. Design Principles

## 5.1 Single Responsibility

A class should have one main responsibility.

Example:

``` text
Vehicle
→ vehicle data and vehicle behavior

TrafficLight
→ traffic-light state

Lane
→ vehicles belonging to a lane

Simulation
→ simulation timing and coordination
```

Do not put the entire simulation inside `main()`.

------------------------------------------------------------------------

## 5.2 Encapsulation

Keep internal state private.

Prefer:

``` cpp
class Vehicle {
private:
    double speed;

public:
    double getSpeed() const;
    void setSpeed(double speed);
};
```

rather than exposing mutable fields publicly.

------------------------------------------------------------------------

## 5.3 Composition Before Inheritance

Use inheritance when objects genuinely have an "is-a" relationship.

Examples:

``` text
Car is a Vehicle
Bus is a Vehicle
Motorbike is a Vehicle
```

Use composition for "has-a" relationships:

``` text
Road has Lanes
Lane has Vehicles
Intersection has Roads
```

------------------------------------------------------------------------

# 6. Phase 0 --- Understand the Existing Code

### Goal

Understand the existing `miniProject.cpp` before modifying architecture.

### Topics

-   Class vs object
-   Constructor
-   `private` / `public`
-   Inheritance
-   Abstract class
-   Pure virtual function
-   `virtual`
-   `override`
-   Polymorphism
-   `std::vector`
-   `std::unique_ptr`
-   `std::make_unique`

### Tasks

1.  Explain the `Vehicle` class.
2.  Explain why `Vehicle` is abstract.
3.  Explain why `Car`, `Bus`, and `Motorbike` can be stored together.
4.  Explain what happens when:

``` cpp
vehicles[i]->move(dt);
```

is executed. 5. Explain why `unique_ptr` is being used. 6. Explain the
current simulation loop.

### Acceptance Criteria

The developer can explain the complete execution flow without relying on
generated explanations.

**Claude Code instruction:** Do not modify code in this phase unless a
tiny experiment is required to demonstrate a concept.

------------------------------------------------------------------------

# 7. Phase 1 --- Refactor the Vehicle Foundation

### Goal

Make the existing vehicle hierarchy clean enough to become the
foundation of the larger system.

### Tasks

-   Improve constructors if necessary.
-   Use member initializer lists.
-   Make getters `const`.
-   Use `override` consistently.
-   Review ownership.
-   Remove duplicated logic where appropriate.
-   Decide whether fuel behavior belongs in the base class or individual
    vehicle classes.
-   Introduce position only when needed for the simulation.

### Example target concept

``` text
Vehicle
├── identity
├── movement information
└── common vehicle behavior

Car
└── car-specific behavior

Bus
└── passenger-related behavior

Motorbike
└── engine-related behavior
```

### Acceptance Criteria

-   Existing Car/Bus/Motorbike behavior still works.
-   No manual memory management.
-   Code compiles with warnings enabled.
-   Developer can explain every inheritance relationship.

------------------------------------------------------------------------

# 8. Phase 2 --- Create the Simulation Model

### Goal

Turn the current simple loop into a basic simulation engine.

Introduce a `Simulation` class.

Possible responsibility:

``` text
Simulation
├── owns/manages vehicles
├── controls simulation steps
└── updates simulation state
```

### Tasks

-   Move simulation logic out of `main()`.
-   Create a clear simulation step.
-   Introduce simulation time.
-   Define what one `dt` means.
-   Allow the simulation to run for a configurable number of steps.
-   Display useful simulation information.

### Acceptance Criteria

The program can run:

``` text
Simulation started
Step 0
Step 1
Step 2
...
Simulation finished
```

without having the simulation logic embedded entirely in `main()`.

------------------------------------------------------------------------

# 9. Phase 3 --- TrafficLight Without State Pattern

### Goal

First understand the traffic-light problem before applying a design
pattern.

Create:

``` text
TrafficLight
```

with:

``` text
RED
GREEN
YELLOW
```

### Tasks

-   Define a traffic-light state representation.
-   Add transition behavior.
-   Add configurable duration for each state.
-   Display the current state.
-   Connect the light conceptually to the simulation clock.

### Important

Do NOT implement State Pattern yet.

Initially use a simple implementation so the problem is visible.

### Acceptance Criteria

A traffic light can transition:

``` text
RED → GREEN → YELLOW → RED
```

and the simulation displays the transitions.

------------------------------------------------------------------------

# 10. Phase 4 --- State Pattern for TrafficLight

### Goal

Replace state-dependent conditional logic with the **State Pattern**.

### Required structure

``` text
TrafficLight
      │
      ↓
TrafficLightState
      │
 ┌────┼─────┐
 ↓    ↓     ↓
Red  Green  Yellow
```

### Concepts to learn

-   Interface / abstract state
-   Delegation
-   State transition
-   Composition
-   Runtime behavior changes

### Tasks

Create:

``` text
TrafficLightState
RedState
GreenState
YellowState
```

The `TrafficLight` should hold its current state and delegate
state-specific behavior to it.

### Acceptance Criteria

-   No giant `if/else` or `switch` controlling all state-specific
    behavior.
-   State transitions work correctly.
-   Adding another state does not require rewriting a giant conditional.
-   Developer can explain why State Pattern is appropriate.

------------------------------------------------------------------------

# 11. Phase 5 --- Vehicle State Model

### Goal

Give vehicles behavior that changes according to their current state.

Possible states:

``` text
Driving
Braking
Stopped
```

### Example behavior

``` text
GREEN
  ↓
Driving

RED detected
  ↓
Braking
  ↓
Stopped

GREEN detected
  ↓
Driving
```

### Required structure

``` text
Vehicle
   │
   ↓
VehicleState
   │
 ┌─┼──────────┐
 ↓ ↓          ↓
Driving     Braking    Stopped
```

### Important

Do not create unnecessary vehicle states. Start with only the states
needed for the simulation.

### Acceptance Criteria

A vehicle can change state based on traffic conditions.

------------------------------------------------------------------------

# 12. Phase 6 --- Introduce Lane

### Goal

Model a lane as an object containing vehicles.

Conceptually:

``` text
Lane 1
────────────────────────
🚗 → 🚗 → 🚌 → 🏍️
────────────────────────
```

### Tasks

Create:

``` text
Lane
```

It should support operations such as:

-   add vehicle
-   remove vehicle
-   inspect vehicles
-   update vehicles

### Ownership decision

Explicitly decide whether the lane:

-   owns vehicles, or
-   references vehicles owned by another component.

Document the decision.

Avoid accidental double ownership.

### Acceptance Criteria

A lane can contain multiple vehicles and update them correctly.

------------------------------------------------------------------------

# 13. Phase 7 --- Introduce Road

### Goal

Create roads containing lanes.

Conceptually:

``` text
Road A
├── Lane 1
├── Lane 2
└── Lane 3
```

### Tasks

Create:

``` text
Road
```

Support:

-   adding lanes
-   accessing lanes
-   updating lanes
-   identifying the road

### Acceptance Criteria

A road can contain multiple lanes and update its infrastructure.

------------------------------------------------------------------------

# 14. Phase 8 --- Composite Pattern for Infrastructure

### Goal

Introduce the **Composite Pattern** only after the road/lane hierarchy
is understood.

### Target structure

``` text
RoadComponent
       │
  ┌────┴────┐
  ↓         ↓
 Lane      Road
```

Both individual and composite infrastructure objects should implement
the common abstraction.

Possible hierarchy:

``` text
City
└── Intersection
    ├── Road
    │   ├── Lane
    │   └── Lane
    └── Road
        ├── Lane
        └── Lane
```

### Core idea

An individual object and a group of objects can be treated through the
same interface.

Example:

``` cpp
component->update();
```

could work for:

-   a Lane
-   a Road
-   an Intersection

### Acceptance Criteria

-   Infrastructure forms a tree.
-   Composite objects can contain child components.
-   The same operation can be applied to an individual component or a
    group.
-   Developer can explain the problem Composite solves.

------------------------------------------------------------------------

# 15. Phase 9 --- Intersection and Traffic Interaction

### Goal

Make the infrastructure meaningful.

Create:

``` text
Intersection
```

that connects roads and traffic lights.

Example:

``` text
          Road A
            ↓
            🚗
            │
            │
        ────┼────
            🚦
        ────┼────
            │
            ↓
          Road B
```

### Tasks

-   Connect lanes/roads to intersections.
-   Associate traffic lights with relevant traffic directions.
-   Vehicles approaching a red light should stop.
-   Vehicles approaching a green light can continue.

### Acceptance Criteria

A simple intersection scenario works correctly.

------------------------------------------------------------------------

# 16. Phase 10 --- Real-Time Simulation

### Goal

Make the simulation continuously progress through time.

### Concepts

-   simulation time
-   real time vs simulated time
-   fixed time step
-   update loop
-   timing

Possible conceptual loop:

``` text
while (running) {

    updateTrafficLights();

    updateVehicles();

    updateInfrastructure();

    display();

    waitUntilNextTick();
}
```

### Important

Keep simulation time separate from wall-clock time where possible.

### Acceptance Criteria

The simulation visibly progresses without manually invoking each step.

------------------------------------------------------------------------

# 17. Phase 11 --- Concurrency Basics

### Goal

Understand C++ concurrency before parallelizing the entire project.

### Topics

-   `std::thread`
-   thread lifetime
-   `join()`
-   `std::mutex`
-   `std::lock_guard`
-   race condition
-   critical section
-   thread safety
-   `std::atomic`

### First exercise

Create a very small independent concurrency experiment.

Example concept:

``` text
Thread A → task A
Thread B → task B
```

Do not immediately modify the traffic simulator.

### Acceptance Criteria

Developer can explain:

-   what a thread is
-   what a race condition is
-   why a mutex is needed
-   why joining threads matters

------------------------------------------------------------------------

# 18. Phase 12 --- Add Controlled Concurrency

### Goal

Introduce concurrency into the simulation without creating one thread
per vehicle.

### Recommended conceptual model

``` text
Simulation
    │
 ┌──┼──────────────┐
 ↓  ↓              ↓
Vehicle update   Traffic-light update
task             task
```

The exact architecture should be chosen based on the existing
implementation.

### Rules

-   Do NOT create one OS thread for every vehicle.
-   Keep the number of worker threads bounded.
-   Identify shared mutable state before adding locks.
-   Protect only the necessary critical sections.
-   Avoid holding locks while doing expensive work.
-   Prefer clear synchronization over premature optimization.

### Acceptance Criteria

Multiple simulation activities execute concurrently and the program
remains correct.

------------------------------------------------------------------------

# 19. Phase 13 --- Thread Safety Review

### Goal

Deliberately test for concurrency problems.

Review shared objects:

``` text
Lane
Road
Vehicle collection
TrafficLight
Simulation state
```

### Questions to answer

-   Who owns this data?
-   Who reads it?
-   Who writes it?
-   Can two threads access it simultaneously?
-   Does it need a mutex?
-   Can it use an atomic instead?
-   Can the design avoid shared mutable state?

### Acceptance Criteria

Every shared mutable object has an explicit synchronization/ownership
decision.

------------------------------------------------------------------------

# 20. Phase 14 --- Final Integration

### Goal

Combine all concepts.

Target flow:

``` text
Start
  ↓
Create city/infrastructure
  ↓
Create roads
  ↓
Create lanes
  ↓
Create traffic lights
  ↓
Create vehicles
  ↓
Place vehicles into lanes
  ↓
Start simulation
  ↓
Traffic lights change
  ↓
Vehicles react
  ↓
Road infrastructure updates
  ↓
Concurrent tasks execute safely
  ↓
Repeat
  ↓
Stop
```

------------------------------------------------------------------------

# 21. Phase 15 --- Testing

### Unit-level tests

Test:

-   Vehicle speed limits
-   Vehicle movement
-   Fuel behavior
-   Traffic-light transitions
-   Vehicle state transitions
-   Lane add/remove behavior
-   Composite update behavior

### Integration tests

Test:

``` text
Red light → vehicle stops
Green light → vehicle moves
Road → lane → vehicle update
Intersection → traffic-light interaction
```

### Concurrency tests

Test:

-   simultaneous access to shared lane data
-   simulation start/stop
-   thread cleanup
-   race-prone operations

Use assertions and deterministic tests wherever possible.

------------------------------------------------------------------------

# 22. Phase 16 --- Documentation

Create a `README.md` containing:

## Project Overview

What the simulation does.

## Architecture

Explain the major classes.

## OOP

Explain:

-   inheritance
-   abstraction
-   polymorphism
-   encapsulation

## State Pattern

Explain:

``` text
TrafficLight → TrafficLightState
Vehicle → VehicleState
```

and why it was used.

## Composite Pattern

Explain the infrastructure tree.

## Concurrency

Explain:

-   what runs concurrently
-   what data is shared
-   how it is synchronized
-   why the chosen design avoids one-thread-per-vehicle

## Build Instructions

Provide exact commands.

## Run Instructions

Provide exact commands.

## Example Output

Show a small simulation example.

------------------------------------------------------------------------

# 23. Final Project Structure

The exact structure can evolve, but aim for something similar to:

``` text
traffic-simulation/
│
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── architecture.md
│   ├── state-pattern.md
│   ├── composite-pattern.md
│   └── concurrency.md
│
├── include/
│   ├── vehicle/
│   │   ├── Vehicle.h
│   │   ├── Car.h
│   │   ├── Bus.h
│   │   └── Motorbike.h
│   │
│   ├── vehicle_state/
│   │   ├── VehicleState.h
│   │   ├── DrivingState.h
│   │   ├── BrakingState.h
│   │   └── StoppedState.h
│   │
│   ├── traffic_light/
│   │   ├── TrafficLight.h
│   │   └── TrafficLightState.h
│   │
│   ├── infrastructure/
│   │   ├── RoadComponent.h
│   │   ├── Lane.h
│   │   ├── Road.h
│   │   └── Intersection.h
│   │
│   └── simulation/
│       └── Simulation.h
│
├── src/
│   ├── vehicle/
│   ├── vehicle_state/
│   ├── traffic_light/
│   ├── infrastructure/
│   └── simulation/
│
└── tests/
```

Do not force this exact directory structure if the project remains
small. Consistency and clarity are more important than folder count.

------------------------------------------------------------------------

# 24. Git Workflow

Use Git from the beginning.

Suggested branches:

``` text
main
└── feature/vehicle-refactor
└── feature/simulation-engine
└── feature/traffic-light
└── feature/state-pattern
└── feature/lane-road
└── feature/composite
└── feature/concurrency
└── feature/testing
```

Suggested commits:

``` text
feat: add traffic light model
feat: implement traffic light state pattern
feat: add lane and road infrastructure
refactor: extract simulation engine
feat: add vehicle states
feat: add concurrent simulation updates
test: add traffic light transition tests
docs: document architecture and design patterns
```

------------------------------------------------------------------------

# 25. Definition of Done

The project is complete when:

-   [ ] Car, Bus, and Motorbike exist.
-   [ ] Vehicle polymorphism works.
-   [ ] Smart-pointer ownership is understood and intentional.
-   [ ] Simulation logic is separated from `main()`.
-   [ ] TrafficLight exists.
-   [ ] TrafficLight supports RED/GREEN/YELLOW.
-   [ ] TrafficLight uses State Pattern.
-   [ ] Vehicles use State Pattern for behavior changes.
-   [ ] Lane exists.
-   [ ] Road exists.
-   [ ] Intersection exists.
-   [ ] Infrastructure uses Composite Pattern appropriately.
-   [ ] Simulation runs continuously.
-   [ ] Concurrency is implemented.
-   [ ] No one-thread-per-vehicle design.
-   [ ] Shared mutable state has a documented synchronization strategy.
-   [ ] Program shuts down threads cleanly.
-   [ ] Tests cover important behavior.
-   [ ] README explains the architecture.
-   [ ] Design patterns are used because they solve actual problems, not
    just for decoration.

------------------------------------------------------------------------

# 26. Claude Code Working Rules

Claude Code must follow these rules throughout the project.

### Rule 1 --- Work phase by phase

Do not implement the entire roadmap in one request.

Only work on the current phase.

### Rule 2 --- Explain before major changes

Before making a significant architectural change, explain:

1.  What problem exists?
2.  What concept solves it?
3.  What files/classes will change?
4.  Why is the proposed design appropriate?

### Rule 3 --- Preserve understanding

Do not hide architectural decisions behind generated code.

For every major class, explain:

-   responsibility
-   important fields
-   important methods
-   relationships with other classes

### Rule 4 --- Do not over-engineer

Avoid adding:

-   unnecessary design patterns
-   unnecessary abstractions
-   unnecessary templates
-   unnecessary frameworks
-   unnecessary third-party libraries

### Rule 5 --- Keep the project compiling

After each meaningful change:

1.  Build the project.
2.  Run relevant tests.
3.  Fix compilation errors.
4.  Report what changed.

### Rule 6 --- Don't silently rewrite working code

If an existing implementation works but needs improvement, explain the
reason before replacing it.

### Rule 7 --- Concurrency comes late

Do not introduce threads until the single-threaded simulation is
correct.

### Rule 8 --- No thread-per-object design

Do not create one OS thread for every vehicle, lane, or traffic light.

### Rule 9 --- Tests are part of implementation

When adding important behavior, add or update tests.

### Rule 10 --- Keep a learning log

After every phase, provide:

``` text
What was implemented:
-

Concepts learned:
-

Important design decisions:
-

Potential problems:
-

Next phase:
-
```

------------------------------------------------------------------------

# 27. How to Use This Plan With Claude Code

At the beginning of a session, tell Claude Code:

> Read `TRAFFIC-SIMULATION-ROADMAP.md`.
>
> We are currently working on Phase X.
>
> Follow the working rules in the roadmap. Do not implement future
> phases. First inspect the current code and explain what needs to
> change. Then make the smallest reasonable implementation for this
> phase. Build and test after the changes. At the end, provide the phase
> learning log.

When you want to move to the next phase:

> Phase X is complete. Review the acceptance criteria from
> `TRAFFIC-SIMULATION-ROADMAP.md`, inspect the current code, and tell me
> whether we are actually ready for Phase X+1. Do not start implementing
> the next phase until I confirm.

------------------------------------------------------------------------

# 28. Final Learning Sequence

The project should teach the concepts in this order:

``` text
Existing miniProject.cpp
        ↓
      OOP
        ↓
 Polymorphism
        ↓
Smart pointers / ownership
        ↓
 Simulation Engine
        ↓
  Traffic Light
        ↓
   State Pattern
        ↓
  Vehicle States
        ↓
      Lane
        ↓
      Road
        ↓
 Composite Pattern
        ↓
   Intersection
        ↓
Real-time simulation
        ↓
Concurrency basics
        ↓
 Thread safety
        ↓
Concurrent simulation
        ↓
      Testing
        ↓
Documentation
        ↓
      FINAL
```

The objective is not merely to produce a working traffic simulator.

The objective is for the developer to be able to explain **why each
architectural decision exists and how the C++ language features, OOP,
State Pattern, Composite Pattern, and concurrency work together.**
