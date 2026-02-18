# home_security_iot_system
### Collaborators: Theodora Ghiorghita, Silvana Banica, Michael Kamara, Jaser Al-Hamiri

# 1.Introduction 

 

This project focuses on the development of a basic IoT-based home alarm system using an ESP32 microcontroller. The main purpose of the system is to improve home saftey by detecting door activity such as an unauthorized entry and movement inside the house when the system is armed.  

The project demonstrates how hardware components, sensor inputs and network communication can work together in a simple IoT security application.	 

 

# 2. Setup – Sensors, Actuators, Modules, Controllers 

  For this project, the ESP32 microcontroller was selected to integrate the necessary sensors and actuators, while a Raspberry Pi unit was employed for receiving and managing security alarms. Communication between the two devices is achieved through the MQTT protocol by connecting them to the same Wi-Fi network and using a dedicated broker, where the ESP32 functions as the publisher and the Raspberry Pi as the subscriber. 
  To ensure the functionality of the system, two sensors were implemented. The Magnetic Reed Switch KY-025 is used to detect door opening events, and the Tracking Sensor KY-033 is responsible for identifying motion inside the house. 

  For activating and deactivating the security system, the following switch module was chosen: 

**Button KY-004:** This module provides the arming and disarming feature that allows the owner to enable the security system when leaving the house and to disable it upon returning in order to stop the alarm. The button replaces the role of a physical keypad that would normally be utilized in a real-life installation. 

 

 

  Based on the data transmitted by the sensors to the ESP32, different actions are executed by the actuators: 

**Passive Buzzer KY-012:** When motion or door opening is detected, the alarm state is enabled. In this situation, the buzzer is activated and emits an audible warning signal. 

**RGB LED KY-016:** In the disarmed state, the RGB LED remains off and does not display any color. During the arming phase, the LED blinks green to inform the owner that there is still time to exit the house without triggering the alarm. Once the system becomes fully armed, the LED shows a static green light. If motion or door opening is detected, the alarm is turned on and the RGB LED displays a blinking red light for one minute, providing the owner with an interval to press the button and disarm the system. If the alarm continues for more than one minute, the LED switches to a static red light. 

  The state of the security system is displayed at all times using an **I2C 16×2 LCD module** , which provides real-time information regarding both the system status and the alarm status.  

 

# 3. System functionality  

  The system operates in two main modes: DISARMED and ARMED, controlled by a push button. 

**DISARMED State**

  -The system starts in the DISARMED state. 

  -The RGB LED is OFF and the buzzer is OFF. 

  -The LCD displays: “SYSTEM: DISARMED” and “ALARM: OFF”. 

  -Sensor monitoring is disabled while the system is disarmed. 

**ARMING State**

  -The user presses the button to arm the system. 

  -A 10-second arming delay starts to allow the owner to leave the house safely. 

During the delay: 

  -The green LED blinks to indicate arming in progress. 

  -The LCD displays: “SYSTEM: ARMING” and “ALARM: OFF”. 

**ARMED State**

  -After the delay, the system becomes ARMED. 

  -The green LED remains ON. 

  -The LCD displays: “SYSTEM: ARMED” and “ALARM: OFF” . 

  -The system continuously monitors: Door access using a reed switch & Movement using a motion sensor 

**Alarm Activation**

  -If movement or door activity is detected while armed the alarm is triggered. 

  -The buzzer is activated and remains ON during the entire alarm period. 
  
  -Short alarm (first 60 seconds): The red LED is blinking. The LCD displays: “PRESS BUTTON TO DISARM”. The user can stop the alarm by pressing the button. 

  -Long alarm (after 60 seconds): The red LED stays on continuously.An SMS alert is sent to the owner using Twilio. The alarm continues until the system is disarmed. 

**Disarming the System**

  -Pressing the button stops the alarm and returns the system to DISARMED. 

  -All LEDs turn OFF and the buzzer stops. 
  -Sensor monitoring is disabled. 

  -The LCD returns to: “SYSTEM:DISARMED” and “ALARM: OFF”. 

  -In addition to local alerts, the system uses WiFi and MQTT to send system status and alarm messages, allowing events to be monitored and logged remotely. 

 

 

# 5. Case Scenarios  

**Case No. 1:** The owner entered the house and the Reed Switch went on- he has 1 minute to press the button to disalarm it for the light to become green, for the buzzer to stop and for the ESP32 to not send a notification to the owner through the Raspberry Pi and Twilio.  

**Case No. 2:** A thief lockpicks the door and enters. The Reed Switch went on and to disalarm it he has to press the button- but he can’t do that because he isn’t the owner ( Let’s pretend the button has a PIN ). The LED and the buzzer keep showing red and buzzing for some time until it reaches the 1 minute mark. Once it did, the Raspberry Pi will see that the alarm has been going for way too long and notifies the owner about a break-in through SMS. 

**Case No. 3:** A thief finds an open window, but the owner sets the system to ON—he is not home. The motion detector starts reading that there is movement around the house so the alarm goes on. If by mistake one of the owners set the system to ON, with another one remaining in the house,  they can easily disalarm it, except for the thief. Then again, a SMS will be sent to the owners of the house. 

 
