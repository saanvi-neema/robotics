// Connects the ESP32 to WiFi and serves a test page over HTTP to confirm
// the network stack works end to end.

const char* WIFI_SSID = "hem-saanvi-deco";
const char* WIFI_PASSWORD = "tGarmuwkak22!";

WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", "<h1>ESP32 is alive!</h1><p>Connected to WiFi.</p>");
}

void setup() {
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
