// Vision "brain" for the face-following car. Reuses the same on-device
// face detection as esp32_face_servo (same camera, same two-stage accurate
// detector, same orientation fix, same debounce logic) but instead of
// driving a local servo, it sends simple one-character commands over a
// second serial port to an Arduino Mega running mega_motor_control, which
// does the actual driving via an Arduino Motor Shield Rev3.
//
// Stage 1 only: 'F' (forward) while a face is present, 'S' (stop) when it
// isn't. Steering ('L'/'R') and distance-based speed come in later stages.

#include <Arduino.h>
#include <eloquent_esp32cam.h>
#include <eloquent_esp32cam/face/detection.h>

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

  Serial.println("Ready, looking for a face...");
}

void loop() {
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
    CarLink.println('F');
  } else {
    Serial.println("No face -> sending S (stop)");
    CarLink.println('S');
  }
}
