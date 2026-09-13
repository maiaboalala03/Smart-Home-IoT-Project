#include <DHT.h>
#include <SoftwareSerial.h>

// ================= PIN DEFINITIONS =================
#define DHT_PIN 4
#define DHT_TYPE DHT11
const int TRIG_PIN = 2;
const int ECHO_PIN = 3;
const int PIR_PIN = 5;
const int IR_PIN = 6;
const int GAS_PIN = A0;
const int LED_PIN = 7;
const int BUZZER_PIN = 8;
const int BT_RX = 10;
const int BT_TX = 11;

// ================= THRESHOLDS =================
const int GAS_THRESHOLD = 500;
const float TEMP_THRESHOLD = 35.0;
const int DISTANCE_THRESHOLD = 50;

// ================= OBJECTS =================
DHT dht(DHT_PIN, DHT_TYPE);
SoftwareSerial bluetooth(BT_RX, BT_TX);

// ================= SYSTEM STATES =================
bool motionAlert = false;
bool intruderConfirmed = false;
String lastStatus = "";

// ================= ULTRASONIC =================
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) {
    return 999;
  }
  long distance = duration * 0.034 / 2;
  return distance;
}

// ================= SEND MESSAGE =================
void sendMessage(String message) {
  Serial.println(message);
  bluetooth.println(message);
}

// ================= PROCESS COMMAND =================
void processCommand(String command) {
  command.trim();
  if (command.equalsIgnoreCase("SAFE")) {
    motionAlert = false;
    intruderConfirmed = false;
    sendMessage("STATUS: SAFE");
    sendMessage("SYSTEM RETURNED TO NORMAL");
  }
  else if (command.equalsIgnoreCase("INTRUDER")) {
    motionAlert = true;
    intruderConfirmed = true;
    sendMessage("INTRUDER CONFIRMED");
    sendMessage("SECURITY ALARM ACTIVATED");
  }
}

// ================= SETUP =================
void setup() {// ================= MAIN LOOP =================
void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int gasValue = analogRead(GAS_PIN);
  int motion = digitalRead(PIR_PIN);
  int irValue = digitalRead(IR_PIN);
  long distance = getDistance();

  if (isnan(temperature) || isnan(humidity)) {
    if (lastStatus != "DHT ERROR") {
      sendMessage("DHT11 ERROR");
      lastStatus = "DHT ERROR";
    }
    delay(1000);
    return;
  }

  bool gasDetected = gasValue > GAS_THRESHOLD;
  bool highTemperature = temperature > TEMP_THRESHOLD;
  bool fireAlert = gasDetected && highTemperature;
  bool irDetected = (irValue == LOW);
  bool objectNear = distance < DISTANCE_THRESHOLD;

  if ((motion == HIGH || irDetected) && !motionAlert) {
    motionAlert = true;
    sendMessage("MOTION ALERT");
    sendMessage("MOTION OR ENTRY DETECTED");
    sendMessage("SEND SAFE OR INTRUDER");
  }

  if (bluetooth.available()) {
    String command = bluetooth.readStringUntil('\n');
    processCommand(command);
  }

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }

  if (fireAlert) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else if (gasDetected) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else if (intruderConfirmed) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }

  String currentStatus = "";
  if (fireAlert) {
    currentStatus = "FIRE";
    if (lastStatus != currentStatus) {
      sendMessage("FIRE ALERT");
      sendMessage("GAS + HIGH TEMPERATURE");
      sendMessage("LED: ON");
      sendMessage("BUZZER: ON");
      lastStatus = currentStatus;
    }
  }
  else if (gasDetected) {
    currentStatus = "GAS";
    if (lastStatus != currentStatus) {
      sendMessage("GAS ALERT");
      sendMessage("GAS LEVEL HIGH");
      sendMessage("BUZZER: ON");
      lastStatus = currentStatus;
    }
  }
  else if (intruderConfirmed) {
    currentStatus = "INTRUDER";
    if (lastStatus != currentStatus) {
      sendMessage("INTRUDER ALERT");
      sendMessage("SECURITY ALARM: ON");
      lastStatus = currentStatus;
    }
  }
  else if (motionAlert) {
    currentStatus = "MOTION";
    if (lastStatus != currentStatus) {
      sendMessage("MOTION ALERT");
      sendMessage("WAITING FOR USER CONFIRMATION");
      lastStatus = currentStatus;
    }
  }
  else if (objectNear) {
    currentStatus = "OBJECT NEAR";
    if (lastStatus != currentStatus) {
      sendMessage("OBJECT NEAR DOOR");
      lastStatus = currentStatus;
    }
  }
  else {
    currentStatus = "NORMAL";
    if (lastStatus != currentStatus) {
      sendMessage("SYSTEM: NORMAL");
      lastStatus = currentStatus;
    }
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Gas: ");
  Serial.println(gasValue);
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  Serial.print("PIR: ");
  Serial.println(motion ? "DETECTED" : "NONE");
  Serial.print("IR: ");
  Serial.println(irDetected ? "DETECTED" : "NONE");
  Serial.println("--------------------");
  delay(1500);
}
  Serial.begin(9600);
  bluetooth.begin(9600);
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(GAS_PIN, INPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  sendMessage("SMART HOME SYSTEM");
  sendMessage("SYSTEM STATUS: NORMAL");
}
