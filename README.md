# Arduino-Smart-Home-V2
This is a detailed version of the Arduino Smart Home System version 2 project.
This project is the Smart Home System v2 and is a detailed version. Schematics and codes have been provided, along with test codes for testing the sensors.

## WORKING PRINCIPLE

### Door Control
* When an RFID card is scanned, the door servo motor activates, and the door opens or closes.

### Curtain Control
* The curtain servo motor is controlled via the IR remote to open or close the curtain.

### Barrier Control
* The infrared barrier sensor detects a vehicle passing.
* When a vehicle approaches, the green light turns on, and the barrier opens.
* 5 seconds after the passage is complete, the yellow light turns on, the barrier closes, and then the red light turns on.

### Security Detection
* Dangers in the environment are detected by the PIR motion sensor, gas sensor, flame sensor, and tilt sensor.
* In case of danger, the buzzer and alarm LEDs are activated.

### Environment Monitoring
* The DHT11 sensor measures temperature and humidity values.
* The measured values are displayed on the LCD screen.

---

## System Capabilities

### Security
* Door control with an RFID card.
* Security enhancement systems with motion and distance detection.
* Alarm activation in case of danger via gas and fire detection.

### Control
* Control of curtains, lamps, power outlets, and barriers via IR remote.
* Vehicle passage control with a traffic light barrier system.

### Environmental Monitoring
* Measuring and displaying temperature and humidity values on the LCD screen.
* Gas/smoke detection and alarming in dangerous situations.

### Automation
* Automatic opening and closing of the barrier during vehicle passage.
* Controlling curtain opening and closing operations with the remote.

---

## Bill of Materials

| Component | Quantity | Function |
| :--- | :--- | :--- |
| Arduino Mega 2560 | 1 | The central processing unit of the system. Controls sensors and modules. |
| RFID-RC522 Module | 1 | Controls the door by reading RFID cards. |
| IR Receiver Module | 1 | Receives IR remote signals to control devices. |
| 16x2 I2C LCD Display | 1 | Displays system status and sensor data. |
| Servo Motor (SG90) | 3 | Moves the door, barrier, and curtain mechanisms. |
| Infrared Barrier Sensor | 1 | Detects vehicle passage to control traffic lights and the barrier. |
| DHT11 Temp/Humidity Sensor | 1 | Measures ambient temperature and humidity. |
| MQ-2 Gas Sensor | 1 | Detects gas leaks. |
| PIR Motion Sensor | 1 | Detects motion in the environment. |
| HC-SR04 Distance Sensor | 1 | Measures distance and enhances barrier safety. |
| Traffic Light LEDs | 3 | Provides information about the barrier status (Red, Yellow, Green). |
| 4-Channel Relay Module | 1 | Controls electrical devices (lamp, outlet, gas valve, fire extinguishing). |
| Buzzer | 1 | Provides audible warnings and alarms. |
| 330 Ohm Resistors | 10+ | Used as current limiters for LEDs and other modules. |

---

## PIN CONNECTIONS

### LCD Display (I2C)
* **SDA:** Arduino Mega pin 20
* **SCL:** Arduino Mega pin 21
* **VCC:** Arduino Mega 5V
* **GND:** Arduino Mega GND

### IR Receiver
* **Data:** Arduino Mega pin 12
* **VCC:** Arduino Mega 5V
* **GND:** Arduino Mega GND

### RFID-RC522 Module
* **SDA (SS):** Arduino Mega pin 53
* **SCK:** Arduino Mega pin 52
* **MOSI:** Arduino Mega pin 51
* **MISO:** Arduino Mega pin 50
* **RST:** Arduino Mega pin 49
* **3.3V:** Arduino Mega 3.3V
* **GND:** Arduino Mega GND

### Servo Motors
* **Door Servo:** Arduino Mega pin 2
* **Barrier Servo:** Arduino Mega pin 3
* **Curtain Servo:** Arduino Mega pin 4
* **VCC:** 5V external power supply
* **GND:** Arduino Mega GND

### Traffic Light LEDs
* **Red LED:** Arduino Mega pin 24 (with 330 ohm resistor)
* **Yellow LED:** Arduino Mega pin 25 (with 330 ohm resistor)
* **Green LED:** Arduino Mega pin 26 (with 330 ohm resistor)

### HC-SR04 Ultrasonic Distance Sensor
* **Trig:** Arduino Mega pin 35
* **Echo:** Arduino Mega pin 36

### Alarm LEDs
* **Earthquake Red LED:** Arduino Mega pin 27 (with 330 ohm resistor)
* **Gas/Fire Red LED:** Arduino Mega pin 28 (with 330 ohm resistor)
* **Motion LED:** Arduino Mega pin 29 (with 330 ohm resistor)
* **Distance Alarm Red LED:** Arduino Mega pin 30 (with 330 ohm resistor)

### Sensors
* **DHT11:** Arduino Mega pin 31
* **Flame Sensor:** Arduino Mega pin 32
* **PIR Motion Sensor:** Arduino Mega pin 33
* **Tilt Sensor:** Arduino Mega pin 34
* **MQ-2 Gas Sensor (Analog):** Arduino Mega pin A0
* **Rain Sensor (Analog):** Arduino Mega pin A1

### Infrared Barrier Sensor
* **Receiver Signal:** Arduino Mega pin 7
* **Receiver VCC:** Arduino Mega 5V
* **Receiver GND:** Arduino Mega GND
* **Transmitter VCC:** Arduino Mega 5V
* **Transmitter GND:** Arduino Mega GND

### Relay Module (4-Channel)
* **Relay 1 (Lamp):** Arduino Mega pin 37
* **Relay 2 (Gas Valve):** Arduino Mega pin 38
* **Relay 3 (Outlet):** Arduino Mega pin 39
* **Relay 4 (Fire Extinguishing):** Arduino Mega pin 40
* **VCC:** Arduino Mega 5V
* **GND:** Arduino Mega GND

### Libraries Used

| Library | Purpose |
| :--- | :--- |
| **MFRC522.h** (RFID Master) | To read data from the RFID-RC522 module. |
| **IRremote.h** (IR Remote Master) | To receive signals from an IR remote control. |
| **LiquidCrystal_I2C.h** | To control the 16x2 I2C LCD screen. |
| **DHT.h** (DHT sensor library) | To read temperature and humidity data from the DHT11 sensor. |
| **Adafruit_Sensor.h** (Adafruit Sensor) | A base library required by the DHT library for sensor operations. |
| **Wire.h** | Required for I2C communication (used by LCD and other sensors). |
| **SPI.h** | Required for SPI communication (used by the RFID module). |

### Buzzer
* **Positive:** Arduino Mega pin 10
* **Negative:** Arduino Mega GND
