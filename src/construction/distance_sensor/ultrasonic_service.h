#ifndef ULTRASONIC_SERVICE_H
#define ULTRASONIC_SERVICE_H

#include <thread>
#include <mutex>
#include <map>
#include <tuple>
#include <memory>

#include "gpio_driver.h"
#include "distance_sensor.h"

namespace lupus::construction::distanceSensor
{

class UltrasonicService
{
  private:
    bool isRunning;
    std::chrono::high_resolution_clock::time_point lastMassTrigger;
    std::shared_ptr<drivers::gpio::GpioDriver> gpio;
    std::thread measuringThread;
    std::mutex registryMutex;
    std::map<int, std::shared_ptr<IDistanceSensor>> registry;

    void measuringLoop(int frequency);
    void onEchoChange(
      int id, int pin, int level,
      std::chrono::high_resolution_clock::time_point echo_time);
    void setupPins(DistanceSensorConfiguration config);

  public:
    UltrasonicService (std::shared_ptr<drivers::gpio::GpioDriver> gpio, int frequency);
    virtual ~UltrasonicService ();
    int registerSensor(std::shared_ptr<IDistanceSensor>);
    void deregisterSensor(int id);
};

}

#endif /* ULTRASONIC_SERVICE_H */
