#pragma once
#include "Vehicle.hpp"

class LandVehicle : public Vehicle {
protected:
  int numWheels;

public:
  LandVehicle(string vehicleId, string brand, int maxSpeed, int numWheels);
  ~LandVehicle();

  void drive(string destination) const;
};