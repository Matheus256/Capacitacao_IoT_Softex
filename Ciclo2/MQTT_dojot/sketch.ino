#include <WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>

// Configurações de Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configurações MQTT
const char* mqttServer = "IP_address";
const int mqttPort = ;  // Porta padrão MQTT
const char* mqttUser = "user_name:device_id";  // Apenas o usuário, sem senha
const char* mqttTopic = "user_name:device_id/attrs";  // Substitua <device_id> pelo ID do dispositivo no Dojot
//MUDAR PARA O SEU USUARIO E ID DO DISPOSITVO

// Cria instâncias para cliente Wi-Fi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

int value = 0;
float temperatura = 0;
//srand(time(NULL));

// Função para conectar ao Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// Função de callback MQTT (não será usada neste exemplo, mas pode ser útil para receber mensagens)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Função para conectar ao broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT... ");
    // Conecta apenas com o id do cliente e usuário, o terceiro parâmetro é para senha
    if (client.connect("ESP32Client", mqttUser, "")) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //Gerando um inteiro aleatório entre 0 e 14
  value = rand()%15;
  temperatura = (rand()%22)/7.0;

  // Publica uma mensagem de exemplo no tópico
  //String payload = "{\"Valor\": 12}";
  //Se for usar o tipo (objeto) String, então precisa passar payload.c_str() no client.publish
  char payload[52];
  sprintf(payload, "{\"Temperatura\": %.2f, \"pH\": %d}", temperatura, value);

  Serial.print("Enviando payload: ");
  Serial.println(payload);

  if (client.publish(mqttTopic, payload)) {
    Serial.println("Mensagem publicada com sucesso");
  } else {
    Serial.println("Falha ao publicar a mensagem");
  }

  delay(5000);  // Aguarda 10 segundos antes de enviar novamente
}
