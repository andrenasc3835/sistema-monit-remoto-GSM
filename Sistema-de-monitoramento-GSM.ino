#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <TinyGsmClient.h>
#include <HttpClient.h>

#define TINY_GSM_MODEM_SIM800  // Altere conforme o módulo GSM usado

// Configurações de pinos para a comunicação serial
#define MODEM_RST             5
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON        23
#define MODEM_TX              27
#define MODEM_RX              26
#define I2C_SDA               21
#define I2C_SCL               22

// Configurações da rede GSM
const char apn[]  = "sua_apn";  // Substitua pelo APN da sua operadora
const char user[] = "";
const char pass[] = "";

// Configurações do servidor
const char server[] = "seu_servidor.com";
const int  port     = 80;

HardwareSerial SerialAT(1);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
HttpClient http(client, server, port);

Adafruit_BME280 bme;

void setup() {
  // Inicializa a comunicação serial
  Serial.begin(115200);
  delay(10);

  // Inicializa a comunicação serial com o modem
  SerialAT.begin(9600, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  // Inicializa o modem GSM
  modem.restart();
  Serial.println("Inicializando modem...");
  modem.gprsConnect(apn, user, pass);

  if (modem.isNetworkConnected()) {
    Serial.println("Conectado à rede GSM!");
  } else {
    Serial.println("Falha ao conectar na rede GSM");
  }

  // Inicializa a comunicação I2C para os sensores
  if (!bme.begin(0x76)) {
    Serial.println("Não foi possível encontrar o sensor BME280!");
    while (1);
  }
}

void loop() {
  // Leitura dos sensores
  float temperatura = bme.readTemperature();
  float umidade = bme.readHumidity();

  // Cria a URL da requisição HTTP
  String url = "/api/sensordata?temp=" + String(temperatura) + "&hum=" + String(umidade);

  // Envia a requisição HTTP
  Serial.print("Enviando requisição HTTP: ");
  Serial.println(url);

  http.get(url);

  // Aguarda a resposta do servidor
  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Código de resposta: ");
  Serial.println(statusCode);
  Serial.print("Resposta do servidor: ");
  Serial.println(response);

  // Aguarda 1 minuto antes de enviar os dados novamente
  delay(60000);
}
