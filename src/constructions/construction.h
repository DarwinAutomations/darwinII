#ifndef CONSTRUCTIONS_CONSTRUCTION_H
#define CONSTRUCTIONS_CONSTRUCTION_H

#include <memory>

#include "steering.h"
#include "motor.h"
#include "distance_sensor.h"

namespace lupus::constructions
{

class IConstruction
{
  public:
    virtual ~IConstruction () = default;

    virtual std::shared_ptr<propulsion::IMotor> getMotor(propulsion::Motor motor) = 0;
    virtual void setPower(propulsion::Motor motor, float power) = 0;
    virtual float getPower(propulsion::Motor motor) = 0;
    virtual float getRps(propulsion::Motor motor) = 0;

    virtual std::shared_ptr<navigation::ISteering> getSteering(navigation::SteeringUnit) = 0;
    virtual void setDirection(navigation::SteeringUnit steering, float direction) = 0;
    virtual float getDirection(navigation::SteeringUnit steering) = 0;

    virtual std::shared_ptr<sensors::IDistanceSensor> getDistanceSensor(sensors::DistanceSensor) = 0;
    virtual float getDistance(sensors::DistanceSensor) = 0;

};

} // namespace lupus::constructions

#endif /* CONSTRUCTIONS_CONSTRUCTION_H */
