#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

// --- GSM CONFIGURATION  ---
SoftwareSerial sim(11, 10); // RX, TX
String number = "+6289652480979"; //  -> Change with your number
String _buffer;
int _timeout;

// --- DISPLAY CONFIGURATION [cite: 15] ---
int16_t alamat_i2c_lcd = 0x27;
LiquidCrystal_I2C lcd(alamat_i2c_lcd, 16, 2);

// --- ANEMOMETER PARAMETERS [cite: 15] ---
volatile byte rpmcount; // [cite: 15]
volatile unsigned long last_micros;
unsigned long timeold;
unsigned long timemeasure = 1.00; // detik
int countThing = 0;
int GPIO_pulse = 2; // [cite: 17] Arduino = D2

// --- WIND SPEED VARIABLES [cite: 18] ---
float rpm, rotasi_per_detik;
float kecepatan_kilometer_per_jam;
float kecepatan_meter_per_detik;

// --- WIND DIRECTION CONFIGURATION [cite: 18] ---
SoftwareSerial dataserial(7, 6); // D7, D6 [cite: 19]
String data, arah_angin, s_angin;
int a, b;

// --- LED PINS ---
const int LED_PIN_1 = 10; // Note: Pin 10 is also TX for GSM. This might flicker during GSM comms.
const int LED_PIN_2 = 9;
const int LED_PIN_3 = 8; // CHANGED from 11 to 8 to avoid conflict with GSM RX

// --- ALARM SETTINGS ---
float windSpeedThreshold = 10.0; // Send SMS if speed > 10.0 km/h (Adjust this value)
unsigned long lastSmsTime = 0;
const long smsInterval = 60000; // Delay between SMS alerts (60 seconds) to prevent spamming

void setup() {
  // Setup LEDs
  // pinMode(10, OUTPUT); // Disabled because Pin 10 is now GSM TX
  pinMode(LED_PIN_3, OUTPUT); // Originally Pin 11 
  pinMode(LED_PIN_2, OUTPUT); // Pin 9 

  // Setup Serial and LCD
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();

  // Setup GSM
  sim.begin(9600);
  _buffer.reserve(50); // [cite: 3]
  Serial.println("System Started...");
  delay(1000);

  // Setup Wind Vane Serial
  dataserial.begin(9600); 

  // Setup Anemometer Interrupts
  pinMode(GPIO_pulse, INPUT_PULLUP);
  digitalWrite(GPIO_pulse, LOW);
  detachInterrupt(digitalPinToInterrupt(GPIO_pulse)); // [cite: 21]
  attachInterrupt(digitalPinToInterrupt(GPIO_pulse), rpm_anemometer, RISING);
  
  rpmcount = 0;
  rpm = 0;
  timeold = 0; // [cite: 22]
  
  // Default to listening to the Wind Vane sensor
  dataserial.listen(); 

  lcd.setCursor(0, 0);
  lcd.print("Kec :");
  lcd.setCursor(0, 0);
  lcd.print("0.0 M/S  ");
}

void loop() {
  // --- WIND SPEED CALCULATION ---
  if ((millis() - timeold) >= timemeasure * 1000) {
    countThing++;
    detachInterrupt(digitalPinToInterrupt(GPIO_pulse)); // [cite: 24]
    
    rotasi_per_detik = float(rpmcount) / float(timemeasure); // [cite: 24]
    
    // Calibration Formula [cite: 26]
    kecepatan_meter_per_detik = ((-0.0181 * (rotasi_per_detik * rotasi_per_detik)) + (1.3859 * rotasi_per_detik) + 1.4055);
    
    // Filter noise [cite: 27]
    if (kecepatan_meter_per_detik <= 1.5) {
      kecepatan_meter_per_detik = 0.0;
    }
    
    kecepatan_kilometer_per_jam = kecepatan_meter_per_detik * 3.6; // [cite: 28]

    // --- LED LOGIC ---
    if (kecepatan_kilometer_per_jam >= 4) {
      // High speed logic (using Pin 9 and Pin 8)
      // digitalWrite(10, HIGH); // Removed (GSM Pin)
      digitalWrite(LED_PIN_3, LOW); // Turn off the "safe" LED
      
      digitalWrite(LED_PIN_2, HIGH); // Turn on Warning LED
      delay(100); 
    } else {
      // Normal speed logic
      digitalWrite(LED_PIN_2, LOW);
      
      digitalWrite(LED_PIN_3, HIGH); // Turn on Safe LED
      delay(100);
      digitalWrite(LED_PIN_3, LOW);
    }

    // --- SERIAL MONITOR OUTPUT ---
    Serial.print("rotasi_per_detik=");
    Serial.print(rotasi_per_detik);
    Serial.print("   m/s=");
    Serial.print(kecepatan_meter_per_detik);
    Serial.print("   km/h=");
    Serial.println(kecepatan_kilometer_per_jam);

    // --- LCD OUTPUT ---
    lcd.setCursor(0, 0);
    lcd.print("Kec :");
    lcd.setCursor(6, 0);
    lcd.print(kecepatan_meter_per_detik);
    lcd.print(" M/S  ");

    // --- SMS ALERT LOGIC (New Addition) ---
    if (kecepatan_kilometer_per_jam >= windSpeedThreshold) {
      // Check if enough time has passed since last SMS to avoid spam
      if (millis() - lastSmsTime > smsInterval) {
        Serial.println("WARNING: High wind detected! Sending SMS...");
        
        // 1. Switch Serial focus to GSM module
        sim.listen(); 
        delay(100);
        
        // 2. Send the message
        SendMessage();
        
        // 3. Update timer
        lastSmsTime = millis();
        
        // 4. Switch Serial focus back to Wind Vane sensor
        dataserial.listen(); 
      }
    }

    if (countThing == 1) {
       // Serial.println("Mengirim data ke server"); // Optional
       countThing = 0;
    }
    
    timeold = millis();
    rpmcount = 0;
    attachInterrupt(digitalPinToInterrupt(GPIO_pulse), rpm_anemometer, RISING); // [cite: 38]
  }

  // --- WIND DIRECTION LOGIC ---
  // Only reads if dataserial is listening (which is default unless sending SMS)
  if (dataserial.available()) {
    data = dataserial.readString(); // [cite: 38]
    a = data.indexOf("*");
    b = data.indexOf("#");
    
    if (a != -1 && b != -1) {
      s_angin = data.substring(a + 1, b); // [cite: 41]
      
      if (s_angin.equals("1")) arah_angin = "Utara     ";
      else if (s_angin.equals("2")) arah_angin = "Timur Laut";
      else if (s_angin.equals("3")) arah_angin = "Timur     ";
      else if (s_angin.equals("4")) arah_angin = "Tenggara  ";
      else if (s_angin.equals("5")) arah_angin = "Selatan   ";
      else if (s_angin.equals("6")) arah_angin = "Barat Daya";
      else if (s_angin.equals("7")) arah_angin = "Barat     ";
      else if (s_angin.equals("8")) arah_angin = "Barat Laut";
      
      Serial.println(arah_angin);
      lcd.setCursor(0, 1);
      lcd.print(arah_angin);
    }
  }
  
  // --- GSM DEBUGGING (Optional: Forward GSM messages to PC) ---
  /* // Note: Enabling this might interfere with wind vane reading because of .listen() switching
  if (sim.available() > 0) {
    Serial.write(sim.read());
  }
  */
}

// --- RPM INTERRUPT FUNCTION [cite: 51] ---
void rpm_anemometer() {
  if (long(micros() - last_micros) >= 5000) {
    rpmcount++;
    last_micros = micros();
  }
}

// --- SEND MESSAGE FUNCTION [cite: 7] ---
void SendMessage() {
  Serial.println("Sending SMS...");
  
  sim.println("AT+CMGF=1"); // Sets the GSM Module in Text Mode [cite: 8]
  delay(200);
  
  sim.println("AT+CMGS=\"" + number + "\"\r"); // Mobile phone number [cite: 9]
  delay(200);
  
  // Custom Message with Data
  String SMS = "ALERT: High Wind Speed Detected! Speed: " + String(kecepatan_kilometer_per_jam) + " km/h";
  
  sim.println(SMS);
  Serial.println(SMS); // [cite: 10]
  delay(100);
  
  sim.println((char)26); // ASCII code of CTRL+Z
  delay(200);
  
  _buffer = _readSerial();
}

// --- GSM HELPER FUNCTION [cite: 12] ---
String _readSerial() {
  _timeout = 0;
  while (!sim.available() && _timeout < 12000) {
    delay(13);
    _timeout++;
  }
  if (sim.available()) {
    return sim.readString();
  }
  return "";
}
