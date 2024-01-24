#include <Arduino.h>
#include "secrets.h"
#include "WiFi.h"
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

float bt_simulated_value = 25.0;
float et_simulated_value = 25.0;

AsyncWebServer server(80);
AsyncWebSocket ws("/WebSocket");

void onWebSocketEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      // Traitement des données WebSocket
      // Serial.printf("Received %u bytes of data: %s\n", len, (const char*)data);

      // Conversion des données en une chaîne de caractères
      String jsonData = String((const char*)data);

      // Analyse JSON
      JsonDocument jsonDocument;
      DeserializationError error = deserializeJson(jsonDocument, jsonData);

      // Vérifiez s'il y a une erreur lors de l'analyse JSON
      if (error) {
        Serial.print(F("JSON parsing failed: "));
        Serial.println(error.c_str());
      } else {
        Serial.println("Parsed JSON object:");
        serializeJsonPretty(jsonDocument, Serial);

        // Récupérez les valeurs du JSON
        const char* command = jsonDocument["command"];
        int id = jsonDocument["id"];
        int roasterID = jsonDocument["roasterID"];

        // Faites quelque chose avec les valeurs récupérées
        Serial.println("Received JSON data:");
        Serial.print("Command: "); Serial.println(command);
        Serial.print("ID: "); Serial.println(id);
        Serial.print("Roaster ID: "); Serial.println(roasterID);


        // Vérifiez la commande getData
        if (strcmp(command, "getData") == 0) {
          JsonDocument responseData;
          responseData["id"] = id;
          responseData["data"]["BT"] = bt_simulated_value;
          responseData["data"]["ET"] = et_simulated_value;

          // Convertissez la réponse en une chaîne JSON
          String responseJson;
          serializeJson(responseData, responseJson);

          Serial.println("JSON response:");
          serializeJsonPretty(responseData, Serial);

          // Envoyez la réponse au client WebSocket
          client->text(responseJson);
          client->printf("%s", responseJson.c_str());
        } 


        bt_simulated_value += 2.1;
        et_simulated_value += 1.1;
        
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Connexion au WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // print the IP address
  Serial.println(WiFi.localIP());

  // Configuration du serveur WebSocket
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  server.begin();
}

void loop() {
  // Votre code principal ici
}
