###Roteiro Video 22 - Comuncando-se com o Lilygo T Display S3 - Telemetria

### 1 - Código Arduino (ESP32-S3)

Abra o Arduino IDE e crie um novo Sketch

**Adicionar a URl com as definições de pacote do ESP32 T Displauy S3**

Vá em **File > Preferences** > **Additional Boards Manager URLs**, cole o seguinte link: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
    
Vá em **Tools > Board > Boards Manager**, procure por **esp32** (da Espressif Systems) e instale a versão mais recente.

**Download das configuraçöes de pinos**

Baixa o repositório do GitHub
https://github.com/Xinyuan-LilyGO/T-Display-S3

Após baixar o repositório, acesse a pasta Lib e copie a pasta TFT_eSPI para a biblioteca do Arduino que fica por padrão em "C:\Users\seu user\Documents\Arduino\libraries"

O código do projeto e listado abaixo, copie e cole o código no seu Sketch, não esqueça de alterar o ssid e o password

```C++
#include <WiFi.h>
#include <WiFiUdp.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>

// Configurações de Rede
const char* ssid = "NOME_DA_SUA_REDE";
const char* password = "SUA_SENHA";
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
```

### 2 - Código C# (.NET 8)

Criar um novo projeto do tipo console e colar o código a seguir.

Instalar pacote NuGet System.Diagnostics.PerformanceCounter  **Rodar como Administrador**.
 
C#

```CSharp
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Diagnostics;

string esp32Ip = "192.168.XXX.XXX"; // IP do seu S3
int port = 4210;
using UdpClient udpClient = new UdpClient();

// Contadores de performance
using var cpuCounter = new PerformanceCounter("Processor", "% Processor Time", "_Total");
using var ramCounter = new PerformanceCounter("Memory", "Available MBytes");
cpuCounter.NextValue(); // Warm up

while (true)
{
    var data = new {
        cpu = (int)cpuCounter.NextValue(),
        ram = (int)ramCounter.NextValue(),
        process = Process.GetProcesses().OrderByDescending(p => p.WorkingSet64).First().ProcessName
    };

    string json = JsonSerializer.Serialize(data);
    byte[] bytes = Encoding.ASCII.GetBytes(json);
    await udpClient.SendAsync(bytes, bytes.Length, esp32Ip, port);
    
    Console.WriteLine($"Enviado: {json}");
    await Task.Delay(1000);
}
```
