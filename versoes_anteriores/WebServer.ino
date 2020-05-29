#include <ESP8266WiFi.h> // biblioteca para usar as funções de Wifi do módulo ESP8266
#include <Wire.h>         // biblioteca de comunicação I2C

/*
 * Definições de alguns endereços mais comuns do MPU6050
 * os registros podem ser facilmente encontrados no mapa de registros do MPU6050
 */
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
float mediaLeiturasZ, mediaLeiturasY;
float anguloyxz, angulo;

// variáveis para armazenar os dados "crus" do acelerômetro
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; 

// Definições da rede Wifi
const char* SSID = "arduino";
const char* PASSWORD = "12341234";
WiFiServer server(80);

  
WiFiClient client;
char c = client.read();

void initI2C() 
{
  //Serial.println("---inside initI2C");
  Wire.begin(sda_pin, scl_pin);
}

/*
 * função que escreve um dado valor em um dado registro
 */
void writeRegMPU(int reg, int val)      //aceita um registro e um valor como parâmetro
{
  Wire.beginTransmission(MPU_ADDR);     // inicia comunicação com endereço do MPU6050
  Wire.write(reg);                      // envia o registro com o qual se deseja trabalhar
  Wire.write(val);                      // escreve o valor no registro
  Wire.endTransmission(true);           // termina a transmissão
}

/*
 * função que lê de um dado registro
 */
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

/*
 * função que procura pelo sensor no endereço 0x68
 */
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

/*
 * função que verifica se o sensor responde e se está ativo
 */
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

/*
 * função de inicialização do sensor
 */
void initMPU()
{
  setSleepOff();
  setGyroScale();
  setAccelScale();
}

/* 
 *  função para configurar o sleep bit  
 */
void setSleepOff()
{
  writeRegMPU(PWR_MGMT_1, 0); // escreve 0 no registro de gerenciamento de energia(0x68), colocando o sensor em o modo ACTIVE
}

/* função para configurar as escalas do giroscópio
   registro da escala do giroscópio: 0x1B[4:3]
   0 é 250°/s
    FS_SEL  Full Scale Range
      0        ± 250 °/s      0b00000000
      1        ± 500 °/s      0b00001000
      2        ± 1000 °/s     0b00010000
      3        ± 2000 °/s     0b00011000
*/
void setGyroScale()
{
  writeRegMPU(GYRO_CONFIG, 0);
}

/* função para configurar as escalas do acelerômetro
   registro da escala do acelerômetro: 0x1C[4:3]
   0 é 250°/s
    AFS_SEL   Full Scale Range
      0           ± 2g            0b00000000
      1           ± 4g            0b00001000
      2           ± 8g            0b00010000
      3           ± 16g           0b00011000
*/
void setAccelScale()
{
  writeRegMPU(ACCEL_CONFIG, 0);
}

/* função que lê os dados 'crus'(raw data) do sensor
   são 14 bytes no total sendo eles 2 bytes para cada eixo e 2 bytes para temperatura:
  0x3B 59 ACCEL_XOUT[15:8]
  0x3C 60 ACCEL_XOUT[7:0]
  0x3D 61 ACCEL_YOUT[15:8]
  0x3E 62 ACCEL_YOUT[7:0]
  0x3F 63 ACCEL_ZOUT[15:8]
  0x40 64 ACCEL_ZOUT[7:0]
  0x41 65 TEMP_OUT[15:8]
  0x42 66 TEMP_OUT[7:0]
  0x43 67 GYRO_XOUT[15:8]
  0x44 68 GYRO_XOUT[7:0]
  0x45 69 GYRO_YOUT[15:8]
  0x46 70 GYRO_YOUT[7:0]
  0x47 71 GYRO_ZOUT[15:8]
  0x48 72 GYRO_ZOUT[7:0]
   
*/
void readRawMPU()
{  
  Wire.beginTransmission(MPU_ADDR);       // inicia comunicação com endereço do MPU6050
  Wire.write(ACCEL_XOUT);                       // envia o registro com o qual se deseja trabalhar, começando com registro 0x3B (ACCEL_XOUT_H)
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

/*
 * função que conecta o NodeMCU na rede Wifi
 * SSID e PASSWORD devem ser indicados nas variáveis
 */
void reconnectWiFi() 
{
  if(WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD);

  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede: ");
  Serial.println(SSID);
  Serial.print("IP obtido: ");
  Serial.println(WiFi.localIP());  
}

void initWiFi()
{
  delay(10);
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconnectWiFi();
}

void setup() {
 
  Serial.begin(115200);

  Serial.println("nIniciando configuração WiFin");
  initWiFi();

  Serial.println("nIniciando configuração do MPU6050n");
  initI2C();
  initMPU();
  checkMPU(MPU_ADDR);
    
  // Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(10000);
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
}

void loop() {

  // Listenning for new clients
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n' && blank_line) {
            //Leitura dos sensores
             readRawMPU();    // lê os dados do sensor

        for(i = 0; i < 300;i++){         
           leiturasEixoY[i] = AcY;
           leiturasEixoX[i] = AcX;
           leiturasEixoZ[i] = AcZ;
           readRawMPU(); 
           //leitura (); 
           }
        for(i = 0; i < 300; i++){ 
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
            /*
            Serial.print("Media eixo Y = ");
            Serial.println(mediaLeiturasY);
            Serial.print("Media eixo X = ");
            Serial.println(mediaLeiturasX);
            Serial.print("Media eixo Z = ");
            Serial.println(mediaLeiturasZ);
            */
            somaLeiturasY = 0;
            somaLeiturasX = 0;
            somaLeiturasZ = 0;
  
       //Serial.print("Temperatura Cº "); Serial.println(Tmp/340.00+36.53);
            
            
       anguloyxz = mediaLeiturasY/(sqrt( pow (mediaLeiturasX, 2) + (mediaLeiturasZ, 2)));
       angulo = atan(anguloyxz);  
            
       Serial.print("Tangente =  ");
       Serial.println( anguloyxz);
       Serial.print("Angulo em RAD =  ");
       Serial.println(angulo);
                          
       delay(100);  
                    
       }
            
        else{
              
              // You can delete the following Serial.print's, it's just for debugging purposes
              Serial.print("Eixo X ");
              Serial.print(mediaLeiturasX);
              Serial.print("Eixo Y ");
              Serial.print(mediaLeiturasY);
              Serial.print("Eixo Z ");
              Serial.print(mediaLeiturasZ);
              Serial.print(" %\t Temperatura: ");
              Serial.print(Tmp/340.00+36.53);
              Serial.print(" *C ");
            }
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            // your actual web page that displays temperature and humidity
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head></head><body><h1>ESP8266 - Temperatura</h1><h3>Temperatura em Celsius: ");
            client.println(Tmp/340.00+36.53);
            client.println("*C</h3><h3>Eixo X ");
            client.println(mediaLeiturasX);
            client.println("*C</h3><h3>Eixo Y");
            client.println(mediaLeiturasY);
            client.println("*C</h3><h3>Eixo Z");
            client.println(mediaLeiturasZ);

            client.println("%</h3><h3>");
            client.println("</body></html>");     
            break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
    }  
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }  
  
