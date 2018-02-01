
#include <ESP8266WiFi.h> // biblioteca para usar as funções de Wifi do módulo ESP8266
#include <Wire.h>         // biblioteca de comunicação I2C
#include <math.h>

//Definições de alguns endereços mais comuns do MPU6050
//os registros podem ser facilmente encontrados no mapa de registros do MPU6050

const int MPU_ADDR =      0x68; // definição do endereço do sensor MPU6050 (0x68)
const int WHO_AM_I =      0x75; // registro de identificação do dispositivo
const int PWR_MGMT_1 =    0x6B; // registro de configuração do gerenciamento de energia
const int GYRO_CONFIG =   0x1B; // registro de configuração do giroscópio
const int ACCEL_CONFIG =  0x1C; // registro de configuração do acelerômetro
const int ACCEL_XOUT =    0x3B; // registro de leitura do eixo X do acelerômetro

const int sda_pin = D5; // definição do pino I2C SDA
const int scl_pin = D6; // definição do pino I2C SCL

int i, eixo;
float leiturasEixoY[300] = {}, mediaLeiturasX = 0, somaLeiturasX = 0, variacao = 0, valorAtual = 0; 
float valorInicial = 0, leiturasEixoX[300], leiturasEixoZ[300],somaLeiturasZ = 0, somaLeiturasY = 0;  
float mediaLeiturasZ, mediaLeiturasY,anguloyxz, angulo, anguloGraus, anguloX,anguloY, plato,sg,compensaAngulo;


// variáveis para armazenar os dados "crus" do acelerômetro
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; 


// Definições da rede Wifi
const char* SSID = "villaGiulia";
const char* PASSWORD = "redSOX857204";
String apiKey = "QNP3RKUA47KWRMSJ"; //API key  ThingSpeak channel

const char* server = "api.thingspeak.com";
WiFiClient client;

//Iniciar a comunicação I2C para os pinos declarados D5 e D6
void initI2C() 
{
  Wire.begin(sda_pin, scl_pin);
}

//função que escreve um dado valor em um dado registro
 
void writeRegMPU(int reg, int val)      //aceita um registro e um valor como parâmetro
{
  Wire.beginTransmission(MPU_ADDR);     // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                      // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                      // escreve o valor no registro
  Wire.endTransmission(true);           // termina a transmissão
}

//função que lê de um dado registro
uint8_t readRegMPU(uint8_t reg)        // aceita um registro como parâmetro
{
  uint8_t data;
  Wire.beginTransmission(MPU_ADDR);     // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                      // envia o registro com o qual se deseja trabalhar
  Wire.endTransmission(false);          // termina transmissão mas continua com I2C aberto (envia STOP e START)
  Wire.requestFrom(MPU_ADDR, 1);        // configura para receber 1 byte do registro escolhido acima
  data = Wire.read();                   // lê o byte e guarda em 'data'
  return data;                          //retorna 'data'
}

// função que procura pelo sensor no endereço 0x68
void findMPU(int mpu_addr)
{
  Wire.beginTransmission(MPU_ADDR);
  int data = Wire.endTransmission(true);

  if(data == 0)
  {
    Serial.print("Dispositivo encontrado no endereço: 0x");
    Serial.println(MPU_ADDR, HEX);
  }
  else 
  {
    Serial.println("Dispositivo não encontrado!");
  }
}

//função que verifica se o sensor responde e se está ativo
void checkMPU(int mpu_addr)
{
  findMPU(MPU_ADDR);
    
  int data = readRegMPU(WHO_AM_I); // Register 117 – Who Am I - 0x75
  
  if(data == 104) 
  {
    Serial.println("MPU6050 Dispositivo respondeu OK! (104)");

    data = readRegMPU(PWR_MGMT_1); // Register 107 – Power Management 1-0x6B

    if(data == 64) Serial.println("MPU6050 em modo SLEEP! (64)");
    else Serial.println("MPU6050 em modo ACTIVE!"); 
  }
  else Serial.println("Verifique dispositivo - MPU6050 NÃO disponível!");
}

// função de inicialização do sensor
void initMPU()
{
  setSleepOff();
  setGyroScale();
  setAccelScale();
}

//função para configurar o sleep bit  

void setSleepOff()
{
  writeRegMPU(PWR_MGMT_1, 0); // escreve 0 no registro de gerenciamento de energia(0x68), colocando o sensor em o modo ACTIVE
}


void setGyroScale()
{
  //configura a escla que o giroscópio vai trabalhar
  writeRegMPU(GYRO_CONFIG, 0);
}

void setAccelScale()
{
  //configura a escla que o acelerômetro vai trabalhar
  writeRegMPU(ACCEL_CONFIG, 0);
}

void readRawMPU()
{  
  Wire.beginTransmission(MPU_ADDR);       // inicia comunicação com endereço do MPU6050
  Wire.write(ACCEL_XOUT);                 // envia o registro com o qual se deseja trabalhar, começando com registro 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);            // termina transmissão mas continua com I2C aberto (envia STOP e START)
  Wire.requestFrom(MPU_ADDR, 14);         // configura para receber 14 bytes começando do registro escolhido acima (0x3B)

  AcX = Wire.read() << 8;                 // lê primeiro o byte mais significativo
  AcX |= Wire.read();                     // depois lê o bit menos significativo
  AcY = Wire.read() << 8;
  AcY |= Wire.read();
  AcZ = Wire.read() << 8;
  AcZ |= Wire.read();

  Tmp = Wire.read() << 8;
  Tmp |= Wire.read();

  GyX = Wire.read() << 8;
  GyX |= Wire.read();
  GyY = Wire.read() << 8;
  GyY |= Wire.read();
  GyZ = Wire.read() << 8;
  GyZ |= Wire.read(); 
                                
}

//função que conecta o NodeMCU na rede Wifi
void reconnectWiFi() 
{
  if(WiFi.status() == WL_CONNECTED)
    return;
  WiFi.begin(SSID, PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
}
 
void initWiFi()
{
  delay(10);
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");
  reconnectWiFi();
}

void devolveAngulo(){


   for(i = 0; i < 300;i++)
 {         
     leiturasEixoY[i] = AcY;
     leiturasEixoX[i] = AcX;
     leiturasEixoZ[i] = AcZ;
     readRawMPU(); 
  }
        
 for(i = 0; i < 300; i++)
 { 
     somaLeiturasY =  somaLeiturasY + leiturasEixoY[i];
     somaLeiturasX = somaLeiturasX +  leiturasEixoX[i];
     somaLeiturasZ = somaLeiturasZ +  leiturasEixoZ[i];
 }
   
  mediaLeiturasX = somaLeiturasX/300;
  mediaLeiturasX = mediaLeiturasX / 100;
  mediaLeiturasY = somaLeiturasY/300;
  mediaLeiturasY = mediaLeiturasY / 100;
  mediaLeiturasZ = somaLeiturasZ/300;
  mediaLeiturasZ = mediaLeiturasZ / 100;
           
  somaLeiturasY = 0;
  somaLeiturasX = 0;
  somaLeiturasZ = 0;
  
  anguloX = atan2 (-mediaLeiturasY,-mediaLeiturasZ)   * 57.2957795 + 180;
    
}




void setup() 
{
  Serial.begin(115200);
  Serial.println("nIniciando configuração WiFin");
  initWiFi();
  Serial.println("nIniciando configuração do MPU6050n");

  initI2C();
  initMPU();
  checkMPU(MPU_ADDR); 

  
  delay(120000);
  //Espera um minuto
  readRawMPU();
  devolveAngulo();
  if(anguloX != 267.7) {
  compensaAngulo = anguloX - 267.7;
  
  }



}

void loop() {
    
  readRawMPU();    // faz uma leitura do sensor antes de guardar nas variáveis

  if (client.connect(server, 80)) {
    devolveAngulo();
    anguloX = anguloX - compensaAngulo;
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(anguloX);
    postStr += "&field2=";
    postStr += String(Tmp/340.00+36.53);
    //plato = -0.037575797*(anguloX*anguloX) + 21.8652595*anguloX - 3158.070334;
    plato = -0.102807132*(anguloX*anguloX) + 58.32428072*anguloX - 8248.869842;
    plato = abs(plato);
    postStr += "&field3=";
    postStr += String(plato);
    sg = 1 +(plato/(258.6 - ((plato/258.2)*227.1)));
    postStr += "&field4=";
    postStr += String(sg*1000);
     
    
    postStr += "\r\n\r\n";

    //Uplad the postSting with temperature and Humidity information every
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.print("Temperatura: ");
    Serial.print(Tmp/340.00+36.53);
    Serial.print("Angulo ");
    Serial.print(anguloX);
    Serial.println("% enviando para Thingspeak");
  }
  client.stop();

  Serial.println("Aguardando nova leitura");
  // thingspeak needs minimum 15 sec delay between updates
  delay(7200000);








} 
