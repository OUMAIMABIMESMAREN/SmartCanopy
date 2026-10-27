// ─── Credentials ───────────────────────────
#define BLYNK_TEMPLATE_ID   "TMPL56GXjBb8I"
#define BLYNK_TEMPLATE_NAME "Smart Canopy"
#define BLYNK_AUTH_TOKEN    "ip31_xspnwhXVhMgAzbSCPDoIoGZC47s"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <ESP32Servo.h>


const char* ssid     = "Redmi 13C";
const char* password = "oumaima200304";

// ─── Pins ──────────────────────────────────
#define DHT_PIN    18
#define SERVO_PIN  19
#define DHT_TYPE   DHT22

// ─── Seuils ────────────────────────────────
#define TEMP_STATUS 30.0
#define HUM_STATUS  50.0

// ─── Angles servo ──────────────────────────
#define ANGLE_OPEN  90
#define ANGLE_CLOSE 0
#define SERVO_DELAY 1000

// ─── Objets ────────────────────────────────
DHT dht(DHT_PIN, DHT_TYPE);
Servo servoCanopy;
BlynkTimer timer;

// ─── État global ───────────────────────────
bool isManual   = false;
bool canopyOpen = false;

// ─── Fonctions servo ───────────────────────
void openCanopy() {
  if (!canopyOpen) {
    servoCanopy.write(ANGLE_OPEN);
    delay(SERVO_DELAY);
    canopyOpen = true;
    Blynk.virtualWrite(V6, 1);
    Serial.println(">> Canopy OUVERTE");
  }
}

void closeCanopy() {
  if (canopyOpen) {
    servoCanopy.write(ANGLE_CLOSE);
    delay(SERVO_DELAY);
    canopyOpen = false;
    Blynk.virtualWrite(V6, 0);
    Serial.println(">> Canopy FERMEE");
  }
}

// ─── V8 : Mode Auto / Manuel ───────────────
BLYNK_WRITE(V8) {
  isManual = param.asInt();
  Serial.print("Mode : ");
  Serial.println(isManual ? "MANUEL" : "AUTO");
}

// ─── V5 : Segmented Switch (Manuel) ────────
BLYNK_WRITE(V1) {
  Serial.print("V1 = ");
  Serial.println(param.asInt());

  if (!isManual) return;

  int val = param.asInt();

  if (val == 0)
    openCanopy();
  else
    closeCanopy();
}

// ─── Lecture DHT22 + logique auto ──────────
void readDHT() {
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Erreur lecture DHT22");
    return;
  }

  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" C  |  Hum: "); Serial.print(hum); Serial.println(" %");

  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V3, hum);

  if (!isManual) {
    if (temp <= TEMP_STATUS || hum >= HUM_STATUS) {
      openCanopy();
    } else {
      closeCanopy();
    }
  }
}

// ─── Setup ─────────────────────────────────
void setup() {
  Serial.begin(115200);
  dht.begin();

  servoCanopy.attach(SERVO_PIN);
  servoCanopy.write(ANGLE_CLOSE);
  delay(1000);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Blynk.virtualWrite(V6, 0);

  timer.setInterval(2000L, readDHT);
}

// ─── Loop ──────────────────────────────────
void loop() {
  Blynk.run();
  timer.run();
}