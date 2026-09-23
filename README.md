
# ESP32-C3 Self-Balancing Robot

A two-wheel self-balancing robot developed from the ground up using an **ESP32-C3**, **FreeRTOS**, an **ICM-42670 6-axis IMU**, a **TB6612FNG dual H-bridge motor driver**, and **DC gear motors with quadrature encoders**.

The project focuses on real-time embedded control, sensor fusion, feedback control, motor actuation, encoder-based motion sensing, and low-level hardware interfacing using **C and ESP-IDF**.

Rather than relying on a prebuilt robotics framework, the control system and hardware interfaces are implemented directly to better understand the complete path from raw sensor measurements to physical actuator response.

---

## Project Overview

A two-wheel balancing robot is an inherently unstable system similar to an inverted pendulum. Without continuous feedback, the robot naturally falls away from its equilibrium point.

The controller continuously estimates the robot's pitch angle and angular velocity, determines how far the system is from its desired balance point, and commands the motors to move the wheels underneath the robot's center of mass.

At a high level, the feedback loop is:

**IMU Measurements → Sensor Fusion → State Estimation → PID Control → PWM Motor Command → Robot Motion → New IMU Measurements**

This loop runs continuously in real time on the ESP32-C3.

---

## Hardware

* **ESP32-C3 Development Board**
* **ICM-42670 6-Axis IMU**

  * 3-axis accelerometer
  * 3-axis gyroscope
* **TB6612FNG Dual H-Bridge Motor Driver**
* **DC Gear Motors**
* **Quadrature Encoders**
* External motor power supply / battery
* Custom mechanical chassis

---

## Software & Technologies

* **C**
* **ESP-IDF**
* **FreeRTOS**
* Embedded systems programming
* Real-time control
* PID feedback control
* Complementary sensor fusion
* I²C communication
* PWM motor control
* GPIO
* Hardware timers
* Interrupt-driven encoder measurement
* Closed-loop control
* Embedded debugging and telemetry

---

## System Architecture

The robot is divided into several functional layers:

```text
                    ┌─────────────────────┐
                    │     ICM-42670       │
                    │ Accelerometer/Gyro  │
                    └──────────┬──────────┘
                               │
                              I²C
                               │
                               ▼
                    ┌─────────────────────┐
                    │      ESP32-C3       │
                    │                     │
                    │  Sensor Processing  │
                    │         ↓           │
                    │ Complementary Filter│
                    │         ↓           │
                    │   PID Controller    │
                    │         ↓           │
                    │    PWM Generation   │
                    └──────┬────────┬─────┘
                           │        │
                         PWM      GPIO
                           │        │
                           ▼        ▼
                    ┌─────────────────────┐
                    │      TB6612FNG      │
                    │ Dual Motor Driver   │
                    └──────────┬──────────┘
                               │
                       ┌───────┴───────┐
                       ▼               ▼
                    Motor 1         Motor 2
                       │               │
                    Encoder         Encoder
                       │               │
                       └───────┬───────┘
                               │
                               ▼
                            ESP32-C3
```

---

## Sensor Acquisition

The ICM-42670 provides raw accelerometer and gyroscope measurements.

Raw accelerometer readings are converted into acceleration values, while gyroscope measurements are converted into angular velocity.

The accelerometer provides a long-term reference for the robot's orientation relative to gravity. The gyroscope provides fast measurements of rotational motion but accumulates error when integrated over time.

Neither sensor is ideal by itself for balancing, so the measurements are combined through sensor fusion.

---

## Complementary Filter

Pitch estimation is performed using a **complementary filter**.

The accelerometer provides an absolute pitch reference:

```text
pitch_accel = atan2(-Ax, Az)
```

The gyroscope predicts the change in orientation:

```text
pitch_gyro = previous_angle + gyro_rate × dt
```

The two estimates are then combined:

```text
filtered_angle =
    α × gyro_prediction
    + (1 - α) × accelerometer_angle
```

The gyroscope dominates short-term motion estimation while the accelerometer continuously corrects long-term drift.

This provides a computationally lightweight orientation estimate suitable for a real-time embedded control loop.

---

## Gyroscope Calibration

Small gyroscope offsets can create significant orientation drift after integration.

To reduce this error, the system performs gyroscope bias calibration and subtracts the measured bias from subsequent angular velocity measurements.

```text
corrected_gyro = measured_gyro - gyro_bias
```

This improves the stability of the pitch estimate and reduces accumulated orientation error.

---

## PID Feedback Controller

The robot uses feedback control to maintain its unstable upright equilibrium.

The control error is defined relative to the desired balance angle:

```text
error = measured_angle - target_angle
```

The controller consists of proportional, integral, and derivative components.

### Proportional Control

```text
P = Kp × error
```

The proportional term determines how aggressively the robot responds to angular displacement.

A larger tilt produces a larger corrective motor command.

### Integral Control

```text
integral = integral + error × dt
I = Ki × integral
```

The integral term accumulates persistent error over time and can compensate for steady-state effects such as mechanical imbalance, drivetrain asymmetry, motor deadband, and small balance-point offsets.

Integral behavior requires careful management to prevent **integral windup** when the robot moves outside its recoverable balancing region.

### Derivative Control

```text
D = Kd × angular_velocity
```

Gyroscope angular velocity is used directly as the derivative measurement.

This provides damping based on how quickly the chassis is rotating and avoids calculating a noisy numerical derivative of the measured angle.

### Controller Output

The resulting motor command is calculated from:

```text
motor_command = P + I + D
```

The command is then converted into motor direction and PWM duty cycle.

---

## Motor Control

Motor actuation is handled through the **TB6612FNG dual H-bridge driver**.

The ESP32-C3 controls:

* Motor direction
* PWM duty cycle
* Driver standby state

PWM is generated using the ESP32's hardware peripherals.

The control output is signed:

```text
Positive command → Forward rotation
Negative command → Reverse rotation
Command magnitude → PWM duty cycle
```

The software also constrains motor commands to the available PWM range before sending them to the motor driver.

---

## Encoder Feedback

The DC gear motors include quadrature encoders for wheel-motion measurement.

Encoder pulses are counted over known time intervals and converted into wheel rotational velocity.

Conceptually:

```text
RPM = encoder_counts / counts_per_revolution × 60 / elapsed_time
```

Encoder feedback provides an additional measurement independent of the IMU.

While the IMU describes chassis orientation and angular velocity, the encoders provide information about actual wheel motion.

This creates the foundation for future cascaded control architectures where wheel velocity or position can be controlled independently of the fast inner balancing loop.

---

## Real-Time Execution

The firmware is built using **ESP-IDF and FreeRTOS**.

Timing is critical because both sensor integration and PID calculations depend on the actual elapsed time between control iterations.

The ESP32 hardware timer is used to determine the control-loop time step:

```text
dt = current_time - previous_time
```

Using measured `dt` rather than assuming a fixed loop frequency improves numerical consistency when execution timing varies.

---

## Control-System Development

Controller development is performed incrementally rather than tuning all parameters simultaneously.

The development process includes:

* Verifying motor direction independently of the controller
* Characterizing motor startup deadband
* Validating accelerometer orientation
* Calibrating gyroscope bias
* Verifying gyroscope sign conventions
* Testing complementary-filter behavior
* Establishing the physical balance point
* Testing proportional control
* Introducing derivative damping
* Characterizing underdamped and overdamped responses
* Introducing integral correction
* Monitoring controller terms through serial telemetry
* Evaluating encoder feedback for future velocity and position control

This approach allows mechanical, electrical, sensor, and software problems to be isolated before modifying controller gains.

---

## Current Capabilities

The current system implements the core components required for closed-loop balancing:

* Real-time IMU acquisition
* Accelerometer-based pitch calculation
* Gyroscope bias correction
* Gyroscope angular-rate measurement
* Complementary-filter sensor fusion
* Dynamic `dt` measurement
* PID feedback control
* Bidirectional motor control
* Hardware PWM generation
* Motor command saturation
* Encoder pulse acquisition
* Wheel RPM calculation
* Serial telemetry for controller analysis
* Experimental controller tuning

The robot is capable of actively correcting small disturbances and recovering toward its equilibrium point.

---

## Engineering Challenges

### Motor Deadband

The DC motors require a minimum command before static friction is overcome and rotation begins.

This introduces a nonlinear region into the actuator response:

```text
Controller Output
       │
       │             /
       │            /
───────┼───────────/──────
       │     Deadband
───────┼───────────\──────
       │            \
       │             \
```

Characterizing this behavior is important because a mathematically valid PID command may still be too small to produce physical movement.

### Sensor Noise

Accelerometers respond not only to gravity but also to chassis acceleration and vibration. Direct accelerometer angle measurements therefore become noisy while the motors are moving.

The complementary filter reduces this effect by relying primarily on gyroscope measurements for short-term motion.

### Gyroscope Drift

Even a small gyroscope bias becomes significant after integration.

Calibration and accelerometer correction are therefore necessary to prevent long-term orientation drift.

### Center of Mass

Battery placement, wiring, chassis geometry, and component positioning change the physical equilibrium point of the robot.

The controller must therefore be tuned for the assembled mechanical system rather than assuming that geometric zero is necessarily the true dynamic balance point.

### Real-Time Timing

Control performance depends heavily on consistent timing.

Sensor acquisition, filtering, PID calculations, encoder processing, motor updates, and debug telemetry must coexist without introducing excessive control-loop latency.

---

## Planned Improvements

Future development will focus on improving robustness and extending the robot from basic angle stabilization toward complete motion control.

Planned work includes:

* PID anti-windup protection
* Integral-term saturation and reset logic
* Improved motor deadband compensation
* Encoder-based wheel velocity feedback
* Position estimation using encoder measurements
* Cascaded control architecture
* Outer velocity/position controller
* Dynamic pitch-setpoint generation
* Improved control-loop scheduling with FreeRTOS
* Telemetry rate limiting
* Controller saturation handling
* Sensor calibration improvements
* Battery-voltage compensation
* Automated data logging and controller analysis

A future cascaded architecture can use a fast inner loop to stabilize chassis angle while a slower outer loop controls wheel velocity or position.

```text
Desired Position / Velocity
          │
          ▼
┌──────────────────────────┐
│ Outer Motion Controller  │
└────────────┬─────────────┘
             │
      Desired Pitch
             │
             ▼
┌──────────────────────────┐
│ Inner Balance Controller │
│       PID / PD           │
└────────────┬─────────────┘
             │
        Motor Command
             │
             ▼
┌──────────────────────────┐
│   TB6612 + DC Motors     │
└──────────────────────────┘
```

This architecture separates the fast balancing problem from the slower problem of controlling where the robot moves.

---

## What This Project Demonstrates

This project provides hands-on experience with:

**Embedded Firmware Engineering** — Development of low-level C firmware on the ESP32-C3 using ESP-IDF and FreeRTOS.

**Real-Time Systems** — Timing-sensitive acquisition, estimation, feedback control, and actuator updates.

**Control Systems** — Practical implementation and experimental tuning of PID feedback for an unstable nonlinear electromechanical system.

**Sensor Fusion** — Combining accelerometer and gyroscope measurements using a complementary filter for real-time attitude estimation.

**Motor Control** — Bidirectional PWM control of brushed DC motors through a dual H-bridge driver.

**Feedback & State Measurement** — Integration of IMU and encoder measurements for chassis and drivetrain state estimation.

**Hardware/Software Integration** — Debugging interactions between sensors, microcontrollers, motor drivers, power electronics, mechanical dynamics, and embedded firmware.

**Experimental Engineering** — Characterizing motor deadband, sensor bias, equilibrium offsets, controller damping, and real-world nonidealities through measured data rather than relying solely on theoretical models.

---

## Repository Structure

A typical organization for the project is:

```text
self-balancing-robot/
│
├── main/
│   ├── main.c
│   ├── motor_control.c
│   ├── motor_control.h
│   ├── imu.c
│   ├── imu.h
│   ├── encoder.c
│   └── encoder.h
│
├── CMakeLists.txt
├── sdkconfig
└── README.md
```

The exact structure may change as the firmware is separated into dedicated drivers, control modules, and FreeRTOS tasks.

---

## Development Environment

The firmware is developed with the Espressif IoT Development Framework (**ESP-IDF**) and targets the **ESP32-C3** RISC-V microcontroller.

Typical development workflow:

```bash
idf.py set-target esp32c3
idf.py build
idf.py flash
idf.py monitor
```

---

## Project Status

**Active Development**

The robot has progressed from individual hardware validation to closed-loop balancing experiments. Current work is focused on improving PID behavior, preventing integral windup, refining sensor calibration, and preparing encoder feedback for higher-level velocity and position control.

---

## Key Engineering Areas

`Embedded C` · `ESP32-C3` · `ESP-IDF` · `FreeRTOS` · `Real-Time Systems` · `PID Control` · `Feedback Control` · `Control Systems` · `Sensor Fusion` · `IMU` · `I²C` · `PWM` · `GPIO` · `Quadrature Encoders` · `Motor Control` · `State Estimation` · `Robotics` · `Embedded Systems` · `RISC-V` · `Hardware/Software Integration`
