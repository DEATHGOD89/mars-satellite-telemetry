/* ==============================================================================
 * PROJECT 1: Mars Satellite / CanSat Environmental Telemetry System
 * DEVELOPED FOR: Robotics & STEM Educator Interview / Portfolio
 * AUTHOR: Bhupender Singh
 * 
 * CONCEPT (Kya hai yeh project?):
 * Yeh ek mini-satellite (CanSat / Mars Rover Probe) ka model hai jo atmospheric data
 * (Temperature, Light Intensity, Air/Gas Quality) measure karta hai aur Ground Station
 * ko wireless telemetry packets (Data Stream) send karta hai.
 * ============================================================================== */

// --- LIBRARIES (Zaroori files include kar rahe hain) ---
#include <DHT.h>          // Temperature aur Humidity sensor (DHT11) ke liye library

// --- PIN DEFINITIONS (Kaunsa sensor kis pin pe laga hai) ---
#define DHTPIN 2          // DHT11 ka data pin Arduino ke Digital Pin 2 pe connect hai
#define DHTTYPE DHT11     // Hum DHT11 sensor use kar rahe hain (agar DHT22 ho to yahan change karein)

#define LDR_PIN A0        // LDR (Light Dependent Resistor) Arduino ke Analog Pin A0 pe laga hai
#define GAS_PIN A1        // Gas sensor (MQ series) Analog Pin A1 pe laga hai (Mars atmosphere testing)
#define BEACON_LED 13     // Satellite heart-beat LED (Onboard LED Pin 13) - pulse indicate karega
#define BUZZER_PIN 8      // Buzzer Pin 8 pe laga hai - emergency alert ya launch beep ke liye

// --- SENSOR OBJECT INITIALIZATION ---
DHT dht(DHTPIN, DHTTYPE); // DHT library ko pin aur type pass karke sensor initialize kiya

// --- GLOBAL VARIABLES (Data store karne ke liye variables) ---
unsigned long packetCount = 0;  // Telemetry packet number count karne ke liye (Packet #1, #2...)
unsigned long lastSendTime = 0; // Timer ke liye variable, bina delay() use kiye non-blocking code ke liye
const unsigned long interval = 2000; // Har 2000 milliseconds (2 seconds) mein ek data packet bhejna hai

void setup() {
  // Serial Communication start kar rahe hain 9600 baud rate pe (Ground Station ke sath link)
  Serial.begin(9600); 

  // Output pins set kar rahe hain
  pinMode(BEACON_LED, OUTPUT); // Pin 13 ko output banaya taaki LED ko ON/OFF kar sakein
  pinMode(BUZZER_PIN, OUTPUT); // Pin 8 ko output banaya sound alert ke liye

  // Input pins set kar rahe hain
  pinMode(LDR_PIN, INPUT);     // A0 pin se light intensity read karenge (Analog input)
  pinMode(GAS_PIN, INPUT);     // A1 pin se gas/air quality read karenge (Analog input)

  // DHT11 sensor ko activate kar rahe hain
  dht.begin(); 

  // --- SATELLITE INITIALIZATION BEEP (Power ON Indicator) ---
  // Satellite start hone par 2 choti beeps dega taaki pata chale system ready hai
  digitalWrite(BUZZER_PIN, HIGH); // Buzzer ON kiya
  delay(100);                     // 100 millisecond hold kiya
  digitalWrite(BUZZER_PIN, LOW);  // Buzzer OFF kiya
  delay(100);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);

  // Ground Station ko welcome handshake message bhej rahe hain
  Serial.println(F("=================================================="));
  Serial.println(F("[SYSTEM]: MARS SATELLITE TELEMETRY INITIALIZED"));
  Serial.println(F("[STATUS]: Ground Station Uplink Established (9600 baud)"));
  Serial.println(F("FORMAT: Packet_ID, Temp(C), Humidity(%), Light(%), Gas_Index, Mission_Time(s)"));
  Serial.println(F("=================================================="));
}

void loop() {
  // Current running time (milliseconds mein) note kar rahe hain
  unsigned long currentMillis = millis();

  // Non-blocking timer check: Kya 2 second beet chuke hain?
  // Isse Arduino freeze nahi hota aur delay() ke bina smooth kaam karta hai
  if (currentMillis - lastSendTime >= interval) {
    lastSendTime = currentMillis; // Timer reset kiya agle packet ke liye
    packetCount++;                // Packet counter 1 se badhaya

    // --- STEP 1: SENSOR DATA READING (Sensors se reading lena) ---
    float temperature = dht.readTemperature(); // Temperature read kiya (Degree Celsius mein)
    float humidity = dht.readHumidity();       // Humidity read kiya (Percentage mein)

    // Check karein agar DHT sensor disconnect ho ya reading fail ho jaye (Safety check)
    if (isnan(temperature) || isnan(humidity)) {
      temperature = -999.0; // Error indicator value agar sensor disconnected ho
      humidity = -999.0;
    }

    // LDR se light reading lena: Arduino analogRead 0 se 1023 deta hai
    int rawLDR = analogRead(LDR_PIN);
    // 0-1023 ko 0-100% mein convert kar rahe hain map function se
    int lightPercent = map(rawLDR, 0, 1023, 0, 100);

    // Gas sensor se Mars atmospheric reading lena (0 - 1023)
    int gasIndex = analogRead(GAS_PIN);

    // Mission elapsed time (Satellite kitne second se chal rahi hai)
    unsigned long missionTimeSec = currentMillis / 1000;

    // --- STEP 2: BEACON BLINK (Satellite Heartbeat) ---
    digitalWrite(BEACON_LED, HIGH); // Packet bhejte waqt LED ON hogi (Tx indicator)

    // --- STEP 3: TELEMETRY DATA PACKET TRANSMISSION (Ground Station ko Data bhejna) ---
    // Hum CSV (Comma Separated Values) format mein packet bhej rahe hain taaki graph easily ban sake
    Serial.print(F("PKT#"));
    Serial.print(packetCount);
    Serial.print(F(", Temp: "));
    Serial.print(temperature, 1); // 1 decimal point tak print karega
    Serial.print(F(" C, Hum: "));
    Serial.print(humidity, 1);
    Serial.print(F(" %, Light: "));
    Serial.print(lightPercent);
    Serial.print(F(" %, Gas: "));
    Serial.print(gasIndex);
    Serial.print(F(", Time: "));
    Serial.print(missionTimeSec);
    Serial.println(F("s"));

    // --- STEP 4: MARS ANOMALY / CRITICAL HAZARD CHECK (Logic Rule) ---
    // Agar Mars atmosphere mein temperature -50C se kam ho ya Gas level 700 cross kare:
    if (gasIndex > 700 || temperature > 60.0) {
      // Emergency warning sound on ground station
      digitalWrite(BUZZER_PIN, HIGH); // Buzzer bajega
      Serial.println(F(">>> [CRITICAL ALERT]: Mars Environmental Hazard Detected! <<<"));
    } else {
      digitalWrite(BUZZER_PIN, LOW);  // Normal condition mein buzzer chup rahega
    }

    delay(50); // Chota sa flash delay
    digitalWrite(BEACON_LED, LOW); // LED OFF kar di (Transmit complete)
  }
}
