#pragma once
#include "Vehicle.hpp"

class AirVehicle : private Vehicle {
private:
    int maxAltitude;
public:
    AirVehicle(string vehicleId, string brand, int maxSpeed, int maxAltitude);
    ~AirVehicle();

    void fly(string destination) const;

    string getBrandName() const;
    string showSpec() const;

};