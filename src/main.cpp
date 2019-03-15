#include <iostream>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <thread>
#include <cstdlib>

#include "gpio_driver.h"
#include "lupus.h"
#include "lupus_factory.h"
#include "lupus_controller.h"
#include "propulsion_unit.h"
#include "distance_sensor.h"
#include "ultrasonic_sensor.h"
#include "ultrasonic_service.h"
#include "profile.h"
#include "granny_profile.h"
#include "propulsion_service.h"
#include "lupus_configuration.h"
#include "controller.h"
#include "controller.h"

using namespace lupus;

void input_loop(
  bool& isActive,
  std::shared_ptr<construction::LupusController> controller);

void output_loop(
  bool& isActive,
  std::shared_ptr<construction::LupusController> controller,
  std::shared_ptr<construction::Lupus> lupus);

int main()
{
  const int i2cAddress = 0x40;
  const int frequency = 60;
  const char* configurationFile = "data/default.cfg";

  auto configuration =
    construction::LupusConfigurationRepository::fromFile(configurationFile);

  auto gpioDriver = std::make_shared<drivers::gpio::GpioDriver>(
    (char*)NULL,
    (char*)NULL);
  auto pwmDriver = std::make_shared<drivers::pwm::PwmDriver>(i2cAddress);
  pwmDriver->setPwmFrequency(frequency);

  auto ultrasonicService = std::make_shared<construction::distanceSensor::UltrasonicService>(
      gpioDriver, 1);

  auto lupus = construction::LupusFactory::create(
      pwmDriver,
      gpioDriver,
      ultrasonicService,
      *configuration);

  lupus->setPower(construction::motor::Motor::FrontLeft, 0);
  lupus->setPower(construction::motor::Motor::FrontRight, 0);
  lupus->setPower(construction::motor::Motor::BackLeft, 0);
  lupus->setPower(construction::motor::Motor::BackRight, 0);
  lupus->setDirection(construction::steeringUnit::SteeringUnitPosition::Left, 0);
  lupus->setDirection(construction::steeringUnit::SteeringUnitPosition::Right, 0);

  auto profile = std::make_shared<application::profiles::GrannyProfile>();
  auto propulsionService =
    std::make_shared<construction::motor::PropulsionService>(lupus, profile);

  auto controller =
    std::make_shared<construction::LupusController>(
      lupus,
      propulsionService);

  // start input and output thread
  bool isActive = true;

  auto input_thread = std::thread(
    input_loop,
    std::ref(isActive),
    controller);

  auto output_thread = std::thread(
    output_loop,
    std::ref(isActive),
    controller,
    lupus);

  // wait for user input and exit
  while(getchar() != 'q');
  isActive = false;
  input_thread.join();
  output_thread.join();

  return 0;
}

void input_loop(
  bool& isActive,
  std::shared_ptr<construction::LupusController> controller)
{
  js_event* event = new js_event();
  int fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
  float magic_value = 32767.0f;

  while(isActive)
  {
    int bytes = read(fd, event, sizeof(js_event));
    if (bytes > 0)
    {
      event->type &= ~JS_EVENT_INIT;

      if (event->type & JS_EVENT_AXIS)
      {
        switch (event->number)
        {
          case 0:
            controller->setDirection(event->value / magic_value);
            break;
          case 5:
            controller->setAcceleration(event->value / (magic_value*2) + 0.5f);
            break;
        }
      }
      if (event->type & JS_EVENT_BUTTON)
      {
        switch (event->number) {
          case 4:
            if(event->value == 1)
      	      controller->setAcceleration(-0.5);
            else
	      controller->setAcceleration(0);
	    break;
          case 5:
            if(event->value == 1)
	      controller->decelerate();
	    else
	      controller->setAcceleration(0);
        }
      }
    }
  }
}

void output_loop(
  bool& isActive,
  std::shared_ptr<construction::LupusController> controller,
  std::shared_ptr<construction::Lupus> lupus)
{
  while (isActive)
  {
    printf("%5.1f %5i %5i %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f\n",
      controller->getDirection(),
    	lupus->getSteeringUnit(construction::steeringUnit::SteeringUnitPosition::Left)->getValue(),
    	lupus->getSteeringUnit(construction::steeringUnit::SteeringUnitPosition::Right)->getValue(),
      lupus->getPower(construction::motor::Motor::FrontLeft) * 100,
      lupus->getMotor(construction::motor::Motor::FrontLeft)->getRawPower(),
      lupus->getRps(construction::motor::Motor::FrontLeft),
      lupus->getPower(construction::motor::Motor::FrontRight) * 100,
      lupus->getMotor(construction::motor::Motor::FrontRight)->getRawPower(),
      lupus->getRps(construction::motor::Motor::FrontRight),
      lupus->getPower(construction::motor::Motor::BackLeft) * 100,
      lupus->getMotor(construction::motor::Motor::BackLeft)->getRawPower(),
      lupus->getRps(construction::motor::Motor::BackLeft),
      lupus->getPower(construction::motor::Motor::BackRight) * 100,
      lupus->getMotor(construction::motor::Motor::BackRight)->getRawPower(),
      lupus->getRps(construction::motor::Motor::BackRight),
      lupus->getDistance(construction::distanceSensor::DistanceSensor::FrontLeft),
      lupus->getDistance(construction::distanceSensor::DistanceSensor::FrontCenterLeft),
      lupus->getDistance(construction::distanceSensor::DistanceSensor::FrontCenterRight),
      lupus->getDistance(construction::distanceSensor::DistanceSensor::FrontRight),
      lupus->getDistance(construction::distanceSensor::DistanceSensor::BackLeft),
      lupus->getDistance(construction::distanceSensor::DistanceSensor::BackCenterLeft),
      lupus->getDistance(construction::distanceSensor::DistanceSensor::BackCenterRight),
      lupus->getDistance(construction::distanceSensor::DistanceSensor::BackRight));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}
