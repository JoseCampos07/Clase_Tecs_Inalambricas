#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

// Cabeceras oficiales del SDK de ESP-IDF para enrutamiento seguro
#include "lwip/lwip_napt.h"
#include "esp_netif.h"

// Configuración de la red que generará la ESP32
const char* ap_ssid = "Portal Cautivo ESP32";
IPAddress apIP(192, 168, 4, 1);
const byte DNS_PORT = 53;

DNSServer dnsServer;
WebServer server(80);

// Variables para el Wi-Fi
String sta_ssid = "";
String sta_password = "";

// Formulario de Inicio de Sesión
const char* login_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Iniciar Sesión en la Red</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #f0f2f5; text-align: center; padding: 50px 20px; margin: 0; }
        .container { max-width: 360px; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); margin: 0 auto; text-align: left; }
        h2 { color: #333; text-align: center; margin-bottom: 20px; }
        label { font-weight: bold; color: #555; display: block; margin-bottom: 5px; }
        input[type="text"], input[type="password"] { width: 100%; padding: 10px; margin-bottom: 20px; border: 1px solid #ccc; border-radius: 5px; box-sizing: border-box; font-size: 16px; }
        input[type="submit"] { width: 100%; background-color: #007bff; color: white; padding: 12px; border: none; border-radius: 5px; font-size: 16px; font-weight: bold; cursor: pointer; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Inicio de Sesión</h2>
        <form action="/login" method="POST">
            <label>Usuario:</label>
            <input type="text" name="usuario" placeholder="Ej. Pepito" required>
            <label>Contraseña:</label>
            <input type="password" name="contrasena" placeholder="••••••••" required>
            <input type="submit" value="Acceder a la Red">
        </form>
    </div>
</body>
</html>
)rawliteral";

// Pantalla de éxito
const char* success_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Acceso Concedido</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #f0f2f5; text-align: center; padding: 50px 20px; }
        .container { max-width: 360px; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); margin: 0 auto; }
        h2 { color: #28a745; }
    </style>
</head>
<body>
    <div class="container">
        <h2>¡Conexión Exitosa!</h2>
        <p>Tu inicio de sesión se ha registrado (Simulado).</p>
        <p>Ya puedes navegar libremente.</p>
    </div>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", login_page);
}

void handleLogin() {
  if (server.hasArg("usuario")) {
    Serial.print("[INFO] Usuario logeado: ");
    Serial.println(server.arg("usuario"));
  }
  server.send(200, "text/html", success_page);
}

esp_err_t tcpip_init_done_signal(void *arg) {
    ip_napt_enable(apIP, 1);
    Serial.println("[NAT] Retransmisión habilitada con éxito en el hilo seguro de TCPIP.");
    return ESP_OK; 
}

// Función para solicitar datos mediante el Monitor Serie
void configurarWifiPorSerial() {
  Serial.println("\n--- CONFIGURACIÓN DE RED WI-FI ---");
  Serial.println("Por favor, introduce el SSID (Nombre de tu Wi-Fi):");
  while (Serial.available() == 0) { delay(100); }
  sta_ssid = Serial.readStringUntil('\n');
  sta_ssid.trim();
  
  Serial.println("Introduce la Contraseña de dicha red:");
  while (Serial.available() == 0) { delay(100); }
  sta_password = Serial.readStringUntil('\n');
  sta_password.trim();

  Serial.print("Intentando conectar a: "); Serial.println(sta_ssid);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- Iniciando Retransmisor Wi-Fi Seguro ---");

  // 1. Solicitar credenciales dinámicas por Serial
  configurarWifiPorSerial();

  // 2. Configurar modo mixto (AP + STA)
  WiFi.mode(WIFI_AP_STA);

  // 3. Conectar a la red
  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
  
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[STA] Conectado exitosamente al módem.");
    Serial.print("[STA] IP asignada: "); Serial.println(WiFi.localIP());

    esp_netif_tcpip_exec(tcpip_init_done_signal, NULL);
  } else {
    Serial.println("\n[STA] Error: No se pudo conectar a la red Wi-Fi.");
  }

  // 4. Levantar la red local de la ESP32
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ap_ssid);
  Serial.print("[AP] Red abierta creada: "); Serial.println(ap_ssid);

  // 5. Iniciar DNS para el Portal Cautivo
  dnsServer.start(DNS_PORT, "*", apIP);

  // 6. Servidor Web
  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_POST, handleLogin);
  
  server.onNotFound([]() {
    server.send(200, "text/html", login_page);
  });

  server.begin();
  Serial.println("[HTTP] Servidor Web Listo.");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  delay(1);
}