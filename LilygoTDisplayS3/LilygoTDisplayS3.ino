#include <WiFi.h>
#include <WiFiUdp.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>

// Configurações de Rede
const char* ssid = "Archenar";
const char* password = "PASS1234567890000";
unsigned int localPort = 4210;

WiFiUDP udp;
TFT_eSPI tft = TFT_eSPI();

void setup() {
  // Ativação do backlight/pinos de controle (específico para algumas placas como T-Display)
  pinMode(15, OUTPUT); digitalWrite(15, HIGH);
  pinMode(38, OUTPUT); digitalWrite(38, HIGH);

  // Inicialização do Display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  // Feedback visual de conexão
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(10, 10);
  tft.println("Conectando WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  tft.fillScreen(TFT_BLACK);
  tft.println("WiFi Conectado!");
  tft.print("IP: "); tft.println(WiFi.localIP());
  
  udp.begin(localPort);
  delay(2000);
}

void loop() {
  int packetSize = udp.parsePacket();

  if (packetSize) {
    char buffer[512];
    int len = udp.read(buffer, 512);
    buffer[len] = 0;

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, buffer);

    if (!error) {
      // Limpa a tela antes de atualizar os novos dados
      tft.fillScreen(TFT_BLACK);
      tft.setTextSize(2); // Aumenta o tamanho da fonte para melhor leitura

      // Exibição da CPU
      tft.setCursor(10, 20);
      tft.setTextColor(TFT_GREEN);
      tft.printf("CPU: %d%%", doc["cpu"].as<int>());

      // Exibição da RAM
      tft.setCursor(10, 70);
      tft.setTextColor(TFT_CYAN);
      tft.printf("RAM: %d MB", doc["ram"].as<int>());

      // Exibição do Processo Principal
      tft.setCursor(10, 120);
      tft.setTextColor(TFT_YELLOW);
      tft.print("TOP: ");
      tft.print(doc["process"].as<const char*>());
    }
  }
}