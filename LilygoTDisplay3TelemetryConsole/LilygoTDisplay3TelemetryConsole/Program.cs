
using System.Net.Sockets;
using System.Text;
using System.Text.Json;
using System.Diagnostics;

string esp32Ip = "192.168.15.132"; // IP do seu S3
int port = 4210;
using UdpClient udpClient = new UdpClient();

// Contadores de performance
using var cpuCounter = new PerformanceCounter("Processor", "% Processor Time", "_Total");
using var ramCounter = new PerformanceCounter("Memory", "Available MBytes");
cpuCounter.NextValue(); // Warm up

while (true)
{
    var data = new
    {
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
