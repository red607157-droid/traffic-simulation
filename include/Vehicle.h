#ifndef VEHICLE_H
#define VEHICLE_H

class Vehicle {
private:
    int id;
    double speed;

public:
    Vehicle(int _i, double _s) : id(_i), speed(_s) {}
    virtual ~Vehicle() {}

    void setId(int _i) { id = _i; }
    void setSpeed(double _s) { speed = _s; }
    int getId() const { return id; }
    double getSpeed() const { return speed; }

    virtual void move(double dt) = 0;
    virtual void display() const = 0;
    virtual double getMaxSpeed() const = 0;
};

#endif
