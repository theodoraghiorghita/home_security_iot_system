// ---- LIBRARIES ------ 

#include <Wire.h> 

#include <LiquidCrystal_I2C.h> 

#include <WiFi.h> 

#include <PubSubClient.h> 

#include "time.h" 

// ---- DISPLAY DECLARATION ---- 

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27- I2C address, 16- characters per row, 2- number of rows 

 

// SYSTEM STATES  

#define DISARMED 0 

#define ARMED    1 

 

/// VARIABLES FOR SYSTEM AND ALARM 

int systemState = DISARMED; // starting as disarmed 

bool alarmActive = false; // alarm OFF 

bool alarmFinalLogged = false; // alarm shorter than a minute  

String alarmReason = ""; // motion / door opening detected 

unsigned long alarmStartTime = 0; // starting from 0 to count until given time in the code 

 

// ARMING VARIABLES 

bool armingInProgress = false; // it is not arming- if it is true, it s arming( not armed- in the progress of arming) 

unsigned long armingStartTime = 0; // counter of the seconds until 10 / armed 

const unsigned long ARM_DELAY = 10000; // 10 seconds to get out of the house 

 

// SENSORS VARIABLES 

const int linetracking = 2; // KY-033 - D2  

int currentLineValue; // current data 

int previousLineValue; // previous data used to compare to the current  

 

const int doorSensor = 5; // KY-025 - D5 - magnetic reed switch 

int currentDoorValue; // same thing 

int previousDoorValue; // same thing 

 

// OUTPUTS - RGB LED & PASSIVE BUZZER  

const int redPin   = 14; // D14 

const int greenPin = 12; // D12 

const int bluePin  = 13; // D13 

 

const int buzzerPin = 25; // D25 

 

// BUTTON  

const int buttonPin = 27; // D27 

bool lastButtonState = HIGH;  

 

// LCD VARIABLES  

int lastSystemState = -1; 

bool lastAlarmActive = false; 

bool lastArmingState = false; 

unsigned long lastAlarmDisplayState = 0; 

 

/// WIFI  

const char* ssid = "MDU_Guest"; 

const char* pass = "Mario202601"; 

 

// MQTT 

const char* mqtt_broker = "10.132.172.132"; // raspberry pi address 

const int mqtt_port = 1883; 

const char* topic = "Project"; 

WiFiClient espClient; 

PubSubClient client(espClient); 

 

// DATE & TIME 

const char* ntpServer = "pool.ntp.org"; // NTP server 

const long  gmtOffset_sec = 3600;       // time zone for sweden 

const int   daylightOffset_sec = 3600;  // time zone for sweden 

 

// FUNCTIONS 

void lcdDisplay() { 

    unsigned long elapsed = millis() - alarmStartTime; // millis counts how many seconds since the alarm started 

    bool shortAlarm = alarmActive && elapsed < 60000;  // <1 min 

    bool longAlarm  = alarmActive && elapsed >= 60000; // >=1 min 

 

    // only update LCD if something changed 

    if (systemState == lastSystemState && alarmActive == lastAlarmActive && armingInProgress == lastArmingState && shortAlarm == lastAlarmDisplayState) { 

        return; 

    } 

    // attribute new data for the variables- if the function skipped the if statement it means that somehting changed about the system/ alarm 

    lastSystemState = systemState; 

    lastAlarmActive = alarmActive; 

    lastArmingState = armingInProgress; 

    lastAlarmDisplayState = shortAlarm; 

 

    lcd.clear(); // clear screen once 

 

    if (shortAlarm) { 

        lcd.setCursor(0, 0); 

 

        lcd.print("PRESS TO DISARM"); 

    }  

    else { 

        // normal display (no alarm OR long alarm) 

        lcd.setCursor(0, 0); 

        if (armingInProgress) { 

            lcd.print("SYSTEM: ARMING   "); 

        }  

        else if (systemState == ARMED) { 

            lcd.print("SYSTEM: ARMED    "); 

        }  

        else { 

            lcd.print("SYSTEM: DISARMED "); 

        } 

 

        lcd.setCursor(0, 1); 

        if (longAlarm) { 

            lcd.print("ALARM: ON        "); 

        } else { 

            lcd.print("ALARM: OFF       "); 

        } 

    } 

} 

 

 

 

 

 

void triggerAlarm(const char* reason) {  

    alarmActive = true; // tells the system that an alarm is on 

    alarmStartTime = millis(); //used to save the current time since the ESP started to know when a minute has passed 

    alarmReason = reason; // saves the alarm reason as a text (for ex: Door opened) 

 

    Serial.print("ALARM TRIGGERED: "); 

    Serial.println(reason); 

 

    digitalWrite(redPin, HIGH); // turn the red led on 

    digitalWrite(greenPin, LOW); // turn the green led off 

    digitalWrite(bluePin, LOW); // turn the ble led off 

    //digitalWrite(buzzerPin, HIGH); 

    lcdDisplay(); //update the dislay  

} 

 

void setArmedState() { 

    systemState = ARMED; //system is armed 

    alarmActive = false; //ensures the alarm s off while arming 

    alarmFinalLogged = false;  

    armingInProgress = true; // starts the countdown (until it reaches 10 secs.) 

    armingStartTime = millis(); //saves time when the arming started 

 

    digitalWrite(redPin, LOW); 

    digitalWrite(greenPin, HIGH); 

    digitalWrite(buzzerPin, LOW); 

 

    Serial.println("SYSTEM: ARMING"); 

    lcdDisplay(); 

} 

 

void setDisarmedState() { 

    systemState = DISARMED;  

    alarmActive = false; //stops alarm in case it was active 

    armingInProgress = false; // stops cuntdown if the alarm is running 

    alarmFinalLogged = false; // resest alarm 

 

    digitalWrite(redPin, LOW); 

    digitalWrite(greenPin, LOW); 

    digitalWrite(buzzerPin, LOW); 

    client.publish(topic, (String("[") + getTimestamp() + "] SYSTEM: DISARMED").c_str()); //gets the current date and time and builds a message that is sent through MQTT  

    Serial.println("SYSTEM: DISARMED"); 

    lcdDisplay(); //update screen 

} 

static void WiFi_setup() { //connects ESP32 to wifi 

  Serial.print("Connecting to WiFi ..."); 

  WiFi.begin(ssid, pass); //starts connecting with ssid and pass. 

 

  while (WiFi.status() != WL_CONNECTED) { //loop starts when wifi is connected 

    delay(500); //waiting time  

    Serial.print("."); 

  } 

 

  Serial.println(); 

  Serial.print("Connected! IP: "); 

  Serial.println(WiFi.localIP()); //prints ESP32 IP 

} 

 

 

void callback(char* topic, byte* payload, unsigned int length) { //this func. is called when the ESP32 received a message  

 

  Serial.print("Message arrived to Raspberry Pi: "); 

  for (int i = 0; i < length; i++) { //loop through message bytes 

    Serial.print((char)payload[i]); //converts bytes into chars.  

  } 

  Serial.println(); 

  Serial.println("-----------------------"); 

} 

 

 

static void MQTT_setup() { //connects ESP32 to MQTT broker  

  client.setServer(mqtt_broker, mqtt_port); //sets broker IP and port 

  client.setCallback(callback); //calls callback function 

 

  while (!client.connected()) { //loop until MQTT is connected  

    String client_id = "esp32-client-"; //creates client id 

    client_id += WiFi.macAddress(); //appends mac address and ... 

    Serial.printf("The client %s connects to the MQTT broker\n", client_id.c_str());  

 

    if (client.connect(client_id.c_str())) { //tries to connect to mqtt 

      Serial.println("Connected to the MQTT broker"); 

    } else { 

      Serial.print("Failed with state: "); 

      Serial.println(client.state()); 

      delay(2000); //wait before retrying in case the connection failed  

    } 

  } 

 

  client.publish(topic, "Connected to MQTT server from ESP32"); //sends mqtt message on connect  

  client.subscribe(topic); //subscribe to the topic (Project) to get the messages sent there 

} 

String getTimestamp() { //return date and time (string) 

    struct tm timeinfo; 

    if(!getLocalTime(&timeinfo)){ 

        return "00:00:00 01/01/1970"; // fallback time if it failed 

    } 

    char buf[30]; 

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo); //format time into text 

    return String(buf); //converts char array to string and returns timestamp 

} 

 

 

// SETUP 

void setup() { //runs once ESP32 boots  

    Serial.begin(115200); //starts serial monitor 

    WiFi_setup(); //connects to wifi 

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //configures time and date 

     

    Serial.print("Waiting for NTP time"); 

    struct tm timeinfo; 

    while(!getLocalTime(&timeinfo)){ //waits until the time is synced 

        Serial.print("."); 

        delay(500); 

    } 

    Serial.println(); 

    Serial.println("Time synced!"); 

    MQTT_setup(); //connects to mqtt 

     

    pinMode(linetracking, INPUT_PULLUP); //motion sensor usually reads HIGH (1) 

    pinMode(doorSensor, INPUT_PULLUP); //door sensor usually reads HIGH(1) 

    pinMode(buttonPin, INPUT_PULLUP);  //button sensor (normally not pressed) reads HIGH (1) 

 

    previousLineValue = digitalRead(linetracking); //save initial motion sensor state 

    previousDoorValue = digitalRead(doorSensor); //save initial door state 

 

    pinMode(redPin, OUTPUT);  

    pinMode(greenPin, OUTPUT); 

    pinMode(bluePin, OUTPUT); 

    pinMode(buzzerPin, OUTPUT); 

 

    Wire.begin(21, 22); //tells which pins are used for I2C (pin 21 = SDA- SERIAL DATA, pin 22 = SCL - SERIAL CLOCK) 

    lcd.init(); 

    lcd.backlight(); 

 

    setDisarmedState(); 

} 

 

 

void loop() { 

    client.loop(); // keeping the client on 

    // BUTTON HANDLING 

    bool buttonState = digitalRead(buttonPin); // reads button state 

    if (buttonState == LOW && lastButtonState == HIGH) { //detects if the btton is pressed by comparing it to the previous state which is first initialized as HIGH so disarmed 

        delay(50); // debounce 

        if (systemState == DISARMED) setArmedState(); //sets the state of the system 

        else setDisarmedState(); 

    } 

    lastButtonState = buttonState; //save the button state 

 

    // ARMING DELAY 

    if (armingInProgress) { 

        unsigned long elapsedArming = millis() - armingStartTime; //how long the arming has been going on 

 

        if (elapsedArming >= ARM_DELAY) { //if 10 seonds have passed 

            armingInProgress = false; //stops arming 

            previousLineValue = digitalRead(linetracking);  

            previousDoorValue = digitalRead(doorSensor); 

            Serial.println("SYSTEM: ARMED"); 

            client.publish(topic, (String("[") + getTimestamp() + "] SYSTEM: ARMED").c_str());  

            digitalWrite(greenPin, HIGH); // solid green after armed 

            digitalWrite(redPin, LOW); 

            digitalWrite(bluePin, LOW); 

        } else { 

            // blink green LED 

            if ((elapsedArming / 500) % 2 == 0) digitalWrite(greenPin, HIGH); 

            else digitalWrite(greenPin, LOW); 

 

            digitalWrite(redPin, LOW); 

            digitalWrite(bluePin, LOW); 

        } 

    } 

 

    //SENSOR LOGIC 

    if (systemState == ARMED && !alarmActive && !armingInProgress) { //sensors active when fully armed  

        currentLineValue = digitalRead(linetracking); 

        if (currentLineValue == HIGH && previousLineValue == LOW) 

            triggerAlarm("Motion detected");  

        previousLineValue = currentLineValue; 

 

        currentDoorValue = digitalRead(doorSensor); 

        if (currentDoorValue == HIGH && previousDoorValue == LOW) 

            triggerAlarm("Door opened"); 

        previousDoorValue = currentDoorValue; 

    } 

 

    // ALARM STATE 

    if (alarmActive) { 

        unsigned long elapsed = millis() - alarmStartTime; 

 

        // Blink red for first minute 

        if (elapsed < 60000) { 

            digitalWrite(redPin, (millis() / 500) % 2); 

        }  

        // After 1 minute 

        else { 

            digitalWrite(redPin, HIGH); 

            digitalWrite(greenPin, LOW); 

            digitalWrite(bluePin, LOW); 

 

            // Only log and send once 

            if (!alarmFinalLogged) { 

                Serial.println("ALARM > 1 minute"); 

                String msg = "[" + getTimestamp() + "] ALARM > 1 MIN: " + alarmReason; 

                client.publish(topic, msg.c_str()); 

 

 

                alarmFinalLogged = true; 

            } 

        } 

    } 

 

 

    //SYSTEM LED STATE  

    if (!armingInProgress && !alarmActive) {  //if the system is not in the process of arming and there is no alarm active  

        if (systemState == ARMED) { 

            digitalWrite(greenPin, HIGH); 

            digitalWrite(redPin, LOW); 

        }  

        else { // DISARMED 

            digitalWrite(redPin, LOW); 

            digitalWrite(greenPin, LOW); 

        } 

} 

 

 

    // LCD 

    lcdDisplay();  

} 