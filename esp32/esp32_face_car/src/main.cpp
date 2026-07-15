// Vision "brain" for the face-following car. Reuses the same on-device
// face detection as esp32_face_servo (same camera, same two-stage accurate
// detector, same orientation fix, same debounce logic) but instead of
// driving a local servo, it sends simple one-character commands over a
// second serial port to an Arduino Mega running mega_motor_control, which
// does the actual driving via an Arduino Motor Shield Rev3.
//
// Stage 1 only: 'F' (forward) while a face is present, 'S' (stop) when it
// isn't. Steering ('L'/'R') and distance-based speed come in later stages.
//
// Set ENABLE_WEB_VIEWER to 1 for a WiFi + browser debug view: a refreshing
// still image, a canvas overlay box around any detected face, and the
// current command being sent to the Mega ('F' or 'S'). Same still-image
// polling approach as esp32_face_servo (see that project's README for why
// true video streaming doesn't work here). Set to 0 for fully standalone
// operation with no WiFi/network involved.

#define ENABLE_WEB_VIEWER 1

#include <Arduino.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/face/detection.h>

#if ENABLE_WEB_VIEWER
#include <eloquent_esp32cam/extra/esp32/wifi/sta.h>
#include <WebServer.h>
using eloq::wifi;

const char *WIFI_SSID = "hem-saanvi-deco";
const char *WIFI_PASSWORD = "tGarmuwkak22!";

WebServer demoServer(82);

const char DEMO_PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head><title>Face Car Debug View</title>
<style>
  body { background:#111; color:#eee; font-family:sans-serif; text-align:center; }
  #wrap { position:relative; display:inline-block; margin-top:20px; }
  #wrap img { width:480px; height:480px; display:block; }
  #box { position:absolute; top:0; left:0; width:480px; height:480px; pointer-events:none; }
  #status { font-size:20px; margin-top:10px; }
  #command { font-size:28px; margin-top:6px; font-weight:bold; }
</style></head>
<body>
  <h2>ESP32-S3 Face Car -- Live Debug View</h2>
  <div id="wrap">
    <img id="video" src="">
    <canvas id="box" width="480" height="480"></canvas>
  </div>
  <div id="status">Waiting...</div>
  <div id="command">--</div>
<script>
  var img = document.getElementById('video');
  var ctx = document.getElementById('box').getContext('2d');
  var scale = 480 / 240;

  function refreshImage() {
    img.src = '/frame.jpg?t=' + Date.now();
  }

  function poll() {
    fetch('/detect').then(r => r.json()).then(d => {
      ctx.clearRect(0, 0, 480, 480);
      if (d.found) {
        ctx.strokeStyle = '#00ff00';
        ctx.lineWidth = 3;
        ctx.strokeRect(d.x * scale, d.y * scale, d.w * scale, d.h * scale);
        document.getElementById('status').textContent = 'Face detected';
      } else {
        document.getElementById('status').textContent = 'No face';
      }
      document.getElementById('command').textContent =
        d.command === 'F' ? 'Sending: F (forward)' : 'Sending: S (stop)';
    }).catch(() => {});
  }

  refreshImage();
  setInterval(refreshImage, 700);
  setInterval(poll, 300);
</script>
</body></html>
)HTML";
#endif

using eloq::camera;
using eloq::face::detection;

// Free GPIO pins (confirmed against Freenove's official pinout, see
// esp32/README.md) not used by the camera -- wired to the Mega's RX/TX.
const int CAR_LINK_TX_PIN = 21;
const int CAR_LINK_RX_PIN = 14;
const long CAR_LINK_BAUD = 9600;

// Same idea as esp32_face_servo: detection naturally misses a single frame
// here and there even while a face stays in view, so we don't react to
// every single frame -- only treat the face as truly gone after it's been
// missing continuously for this long.
const unsigned long FACE_LOST_GRACE_MS = 800;

HardwareSerial CarLink(1);
unsigned long lastSeenFaceMs = 0;
char lastCommandSent = 'S';

void setup() {
  Serial.begin(115200);
  delay(2000);

  CarLink.begin(CAR_LINK_BAUD, SERIAL_8N1, CAR_LINK_RX_PIN, CAR_LINK_TX_PIN);

  camera.pinout.freenove_s3();
  camera.brownout.disable();
  camera.resolution.face();
  camera.quality.high();

  while (!camera.begin().isOk()) {
    Serial.println(camera.exception.toString());
    delay(1000);
  }
  Serial.println("Camera OK");

  // This board's camera is physically mounted upside-down (confirmed
  // earlier in esp32_camera_web and esp32_face_servo) -- without
  // correcting orientation here, the face detector sees upside-down faces
  // and never matches.
  camera.sensor.vflip(true);

  // Single-stage detection (the default) produces low-confidence
  // candidates (~0.10-0.15 observed) even for a real face dead-center in
  // frame. The two-stage MSR01+MNP01 pipeline verifies/refines those into
  // much higher, trustworthy scores -- slower per-frame, but correctness
  // matters more than speed here.
  detection.accurate();

  // Real detections scored 0.96-1.00 in testing; 0.6 cleanly filters noise.
  detection.confidence(0.6);

#if ENABLE_WEB_VIEWER
  Serial.print("Connecting to WiFi");
  while (!wifi.connect(WIFI_SSID, WIFI_PASSWORD).isOk()) {
    Serial.println(wifi.exception.toString());
    delay(1000);
  }
  Serial.println("WiFi connected");

  demoServer.on("/", HTTP_GET, []() {
    demoServer.send_P(200, "text/html", DEMO_PAGE);
  });
  demoServer.on("/frame.jpg", HTTP_GET, []() {
    if (!camera.hasFrame()) {
      demoServer.send(503, "text/plain", "No frame yet");
      return;
    }
    demoServer.send_P(200, "image/jpeg", (const char *) camera.frame->buf, camera.frame->len);
  });
  demoServer.on("/detect", HTTP_GET, []() {
    String json = String("{\"found\":") + (detection.found() ? "true" : "false") +
                  ",\"x\":" + detection.first.x +
                  ",\"y\":" + detection.first.y +
                  ",\"w\":" + detection.first.width +
                  ",\"h\":" + detection.first.height +
                  ",\"command\":\"" + lastCommandSent + "\"}";
    demoServer.send(200, "application/json", json);
  });
  demoServer.begin();
  Serial.print("Debug view: http://");
  Serial.print(wifi.ip());
  Serial.println(":82");
#endif

  Serial.println("Ready, looking for a face...");
}

void loop() {
#if ENABLE_WEB_VIEWER
  // Serves /frame.jpg from whatever camera.frame this loop captured last
  // cycle -- safe to read here since nothing reassigns it until this same
  // thread's own camera.capture() call further down.
  demoServer.handleClient();
#endif

  if (!camera.capture().isOk()) {
    Serial.println(camera.exception.toString());
    return;
  }

  if (!detection.run().isOk()) {
    Serial.println(detection.exception.toString());
    return;
  }

  if (detection.found()) {
    lastSeenFaceMs = millis();
  }

  bool faceRecentlyPresent = (millis() - lastSeenFaceMs) < FACE_LOST_GRACE_MS;

  if (faceRecentlyPresent) {
    Serial.println("Face present -> sending F (forward)");
    lastCommandSent = 'F';
    CarLink.println('F');
  } else {
    Serial.println("No face -> sending S (stop)");
    lastCommandSent = 'S';
    CarLink.println('S');
  }
}
