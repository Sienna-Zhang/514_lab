#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// IMPORTANT: these must be defined BEFORE including FirebaseClient.h
#define ENABLE_USER_AUTH
#define ENABLE_DATABASE
#include <FirebaseClient.h>

// =====================
// Pin config (change to your wiring)
// =====================
// You said: VCC=3V3, ECHO=A0, TRIG=A1
// If your board does NOT support A0/A1 names, replace with the actual GPIO numbers.
static const int PIN_ECHO = A0;
static const int PIN_TRIG = A1;

// =====================
// WiFi config
// =====================
static const char *WIFI_SSID = "UW MPSK";
static const char *WIFI_PASS = "tE5ihepiWG46x9FC";

// =====================
// Firebase config (Email/Password)
// =====================
static const char *FIREBASE_WEB_API_KEY = "";
static const char *FIREBASE_USER_EMAIL  = "";
static const char *FIREBASE_USER_PASS   = "";
static const char *RTDB_URL             = ""; // keep trailing '/'

// =====================
// Strategy parameters (tune these)
// =====================
// Strategy A: periodic upload every T_A seconds (baseline)
static const uint32_t T_A_SLEEP_SEC = 10;     // deep sleep duration for Strategy A
static const uint32_t A_RUN_CYCLES  = 3;      // run A for N wake cycles (3 cycles ~ 30s if T_A_SLEEP_SEC=10)

// Strategy B: conditional upload, longer sleep when idle
static const uint32_t T_B_SLEEP_SEC = 15;     // deep sleep duration for Strategy B
static const uint32_t B_RUN_CYCLES  = 3;      // run B for N wake cycles (3 cycles ~ 45s if T_B_SLEEP_SEC=15)
static const float    DIST_THRESHOLD_CM = 50; // only upload when distance <= threshold

// Ultrasonic sampling
static const uint32_t PULSE_TIMEOUT_US = 30000; // 30ms (~5m max)

// =====================
// Firebase objects
// =====================
UserAuth user_auth(FIREBASE_WEB_API_KEY, FIREBASE_USER_EMAIL, FIREBASE_USER_PASS);
FirebaseApp app;

WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient async_client(ssl_client);

RealtimeDatabase Database;
AsyncResult dbResult;

// =====================
// RTC state (survives deep sleep)
// =====================
RTC_DATA_ATTR uint32_t g_wake_count = 0;   // counts wake cycles overall
RTC_DATA_ATTR uint8_t  g_phase = 0;        // 0 = Strategy A, 1 = Strategy B

// =====================
// Helpers
// =====================
static void processData(AsyncResult &aResult)
{
  if (!aResult.isResult()) return;

  if (aResult.isError())
  {
    Serial.printf("Firebase error: %s (code %d)\n",
                  aResult.error().message().c_str(),
                  aResult.error().code());
  }

  // Optional: print payload/debug/event if you want
  // if (aResult.available()) Serial.printf("Firebase payload: %s\n", aResult.c_str());
}

static void wifiOff()
{
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(50);
}

static bool wifiOnAndConnect(uint32_t timeout_ms = 8000)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeout_ms)
  {
    delay(200);
  }
  return (WiFi.status() == WL_CONNECTED);
}

static void firebaseInit()
{
  ssl_client.setInsecure();
  ssl_client.setHandshakeTimeout(15);

  initializeApp(async_client, app, getAuth(user_auth));
  app.getApp<RealtimeDatabase>(Database);
  Database.url(RTDB_URL);
}

static float readUltrasonicCm()
{
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, PULSE_TIMEOUT_US);
  if (duration <= 0) return -1.0f;

  float cm = duration * 0.0343f / 2.0f;
  return cm;
}

static bool sendToFirebase(float distance_cm, const char *tag)
{
  // Build a small JSON-like string (easy to read in RTDB)
  String payload = "{";
  payload += "\"tag\":\"" + String(tag) + "\",";
  payload += "\"distance_cm\":" + String(distance_cm, 2) + ",";
  payload += "\"millis\":" + String(millis()) + ",";
  payload += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  payload += "}";

  // Save under /lab8/...
  String path = "/lab8/ultrasonic";

  Database.set<String>(async_client, path, payload, dbResult);
  processData(dbResult);

  // Simple “best effort” status print
  if (dbResult.isResult() && !dbResult.isError())
  {
    Serial.println("RTDB write: OK");
    return true;
  }
  Serial.println("RTDB write: attempted (check errors above if any)");
  return false;
}

static void goDeepSleepSeconds(uint32_t seconds)
{
  Serial.printf("Entering deep sleep for %u s...\n", seconds);
  wifiOff();
  delay(50);

  esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
  esp_deep_sleep_start();
}

// =====================
// Strategy runners
// =====================
static void runStrategyA()
{
  Serial.println("\n==============================");
  Serial.println("[A] Strategy A: periodic WiFi upload (baseline)");
  Serial.printf("[A] wake_count=%lu\n", (unsigned long)g_wake_count);

  // 1) Sensor read
  float cm = readUltrasonicCm();
  Serial.printf("[A] distance_cm=%.2f\n", cm);

  // 2) WiFi + Firebase upload every wake
  bool wifi_ok = wifiOnAndConnect();
  Serial.printf("[A] WiFi connected: %s\n", wifi_ok ? "YES" : "NO");
  if (wifi_ok)
  {
    firebaseInit();

    // wait a short time for auth
    uint32_t t0 = millis();
    while (!app.ready() && (millis() - t0) < 4000)
    {
      app.loop();
      delay(50);
    }
    Serial.printf("[A] app.ready()=%s\n", app.ready() ? "true" : "false");

    if (app.ready())
    {
      sendToFirebase(cm, "A_periodic");
    }
  }

  // 3) Sleep
  goDeepSleepSeconds(T_A_SLEEP_SEC);
}

static void runStrategyB()
{
  Serial.println("\n==============================");
  Serial.println("[B] Strategy B: conditional WiFi upload (energy-saving)");
  Serial.printf("[B] wake_count=%lu\n", (unsigned long)g_wake_count);

  // 1) Sensor read
  float cm = readUltrasonicCm();
  Serial.printf("[B] distance_cm=%.2f\n", cm);

  // 2) Only upload if “activity detected”
  bool should_upload = (cm > 0 && cm <= DIST_THRESHOLD_CM);
  Serial.printf("[B] should_upload=%s (threshold=%.1fcm)\n",
                should_upload ? "YES" : "NO",
                DIST_THRESHOLD_CM);

  if (should_upload)
  {
    bool wifi_ok = wifiOnAndConnect();
    Serial.printf("[B] WiFi connected: %s\n", wifi_ok ? "YES" : "NO");

    if (wifi_ok)
    {
      firebaseInit();

      uint32_t t0 = millis();
      while (!app.ready() && (millis() - t0) < 4000)
      {
        app.loop();
        delay(50);
      }
      Serial.printf("[B] app.ready()=%s\n", app.ready() ? "true" : "false");

      if (app.ready())
      {
        sendToFirebase(cm, "B_conditional");
      }
    }
  }
  else
  {
    // WiFi stays OFF for this wake -> saves energy
    wifiOff();
  }

  // 3) Longer sleep
  goDeepSleepSeconds(T_B_SLEEP_SEC);
}

// =====================
// Arduino entry
// =====================
void setup()
{
  Serial.begin(115200);
  delay(300);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // Make WiFi off by default
  wifiOff();

  // Update wake counter
  g_wake_count++;

  // Decide which strategy phase we are in:
  // Phase 0: Strategy A for A_RUN_CYCLES wake cycles
  // Phase 1: Strategy B for B_RUN_CYCLES wake cycles
  if (g_phase == 0)
  {
    if (g_wake_count <= A_RUN_CYCLES)
    {
      runStrategyA();
      return;
    }
    else
    {
      // switch to phase B
      g_phase = 1;
      g_wake_count = 1; // reset counter for phase B
      runStrategyB();
      return;
    }
  }
  else
  {
    if (g_wake_count <= B_RUN_CYCLES)
    {
      runStrategyB();
      return;
    }
    else
    {
      // loop back to phase A for repeated measurement
      g_phase = 0;
      g_wake_count = 1;
      runStrategyA();
      return;
    }
  }
}

void loop()
{
  // Not used (deep sleep restarts setup each time).
}
