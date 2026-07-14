// Detects a face on-device via the Freenove ESP32-S3 CAM's OV2640, and while
// a face stays in frame, continuously sweeps a servo back and forth between
// 0 and 90 degrees.
//
// Set ENABLE_WEB_VIEWER to 1 for a WiFi + browser demo view: a refreshing
// still image (not true streaming video -- see note below) plus a canvas
// overlay box drawn around the detected face. Set to 0 for fully standalone
// operation with no WiFi/network involved.
//
// Note: originally tried a real MJPEG video stream (both the library's
// built-in eloq::viz::faceStream and its plain eloq::viz::mjpeg), running
// on a separate background thread that captures its own frames. Confirmed
// directly that this never delivers a single video frame: camera.capture()
// only waits 1000ms for the shared camera lock, but our accurate/two-stage
// detection holds that same lock longer than that per cycle, so the video
// thread's capture attempts always time out -- no amount of thread-priority
// or scheduling tuning fixes a lock genuinely held too long. Simpler fix:
// don't run a second competing capture at all. This loop already captures
// one frame per cycle for detection; the web page just polls for and
// displays that same already-in-memory frame as a refreshing still image.

#define ENABLE_WEB_VIEWER 1

#include <Arduino.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/face/detection.h>
#include <ESP32Servo.h>

#if ENABLE_WEB_VIEWER
#include <eloquent_esp32cam/extra/esp32/wifi/sta.h>
#include <WebServer.h>
using eloq::wifi;

const char *WIFI_SSID = "hem-saanvi-deco";
const char *WIFI_PASSWORD = "tGarmuwkak22!";

WebServer demoServer(82);

const char DEMO_PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head><title>Face Detection Demo</title>
<style>
  body { background:#111; color:#eee; font-family:sans-serif; text-align:center; }
  #wrap { position:relative; display:inline-block; margin-top:20px; }
  #wrap img { width:480px; height:480px; display:block; }
  #box { position:absolute; top:0; left:0; width:480px; height:480px; pointer-events:none; }
  #status { font-size:20px; margin-top:10px; }
</style></head>
<body>
  <h2>ESP32-S3 Face Detection</h2>
  <div id="wrap">
    <img id="video" src="">
    <canvas id="box" width="480" height="480"></canvas>
  </div>
  <div id="status">Waiting...</div>
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

const int SERVO_PIN = 14;
const int SWEEP_MIN_DEG = 0;
const int SWEEP_MAX_DEG = 90;
const int SWEEP_STEP_DEG = 3;

// Detection naturally misses a single frame here and there even while a
// face stays in view (observed directly in testing). Without debouncing,
// each missed frame snapped the sweep back to 0 and restarted it, which
// looked erratic instead of a smooth continuous sweep.
const unsigned long FACE_LOST_GRACE_MS = 800;

Servo servo;
int sweepAngle = SWEEP_MIN_DEG;
int sweepDirection = 1;
unsigned long lastSeenFaceMs = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

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
  // earlier in esp32_camera_web) -- without correcting orientation here,
  // the face detector sees upside-down faces and never matches.
  camera.sensor.vflip(true);

  // Single-stage detection (the default) produces low-confidence candidates
  // (~0.10-0.15 observed) even for a real face dead-center in frame. The
  // two-stage MSR01+MNP01 pipeline verifies/refines those into much higher,
  // trustworthy scores -- slower per-frame, but correctness matters more
  // than speed here.
  detection.accurate();

  // Real detections scored 0.96-1.00 in testing; 0.6 cleanly filters noise.
  detection.confidence(0.6);

  servo.attach(SERVO_PIN);
  servo.write(SWEEP_MIN_DEG);

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
                  ",\"h\":" + detection.first.height + "}";
    demoServer.send(200, "application/json", json);
  });
  demoServer.begin();
  Serial.print("Demo page: http://");
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
    Serial.print("Face present, sweeping. Angle: ");
    Serial.println(sweepAngle);

    sweepAngle += sweepDirection * SWEEP_STEP_DEG;
    if (sweepAngle >= SWEEP_MAX_DEG) {
      sweepAngle = SWEEP_MAX_DEG;
      sweepDirection = -1;
    } else if (sweepAngle <= SWEEP_MIN_DEG) {
      sweepAngle = SWEEP_MIN_DEG;
      sweepDirection = 1;
    }
    servo.write(sweepAngle);
  } else {
    Serial.println("No face");
    sweepAngle = SWEEP_MIN_DEG;
    sweepDirection = 1;
    servo.write(SWEEP_MIN_DEG);
  }
}
