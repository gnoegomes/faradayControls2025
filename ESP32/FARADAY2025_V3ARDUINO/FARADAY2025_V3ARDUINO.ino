#include <Nextion.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <HardwareSerial.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_ADS1X15.h>

const byte buffSize =9;
const uint8_t startMarker = 229;
const uint8_t endMarker = 228;
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;


//------------------------------------------------------------------------
//LEITURA ADC
#define TENS_MAX_IMD 2.8

Adafruit_ADS1115 ads;

float adcbse1, adcbse2, adcBateria, adcIMD;

//------------------------------------------------------------------------

//VALORES CONSTANTES
int temErro = 0;
int erroAnterior = 0;        // 0 = sem erro, 1 = com erro
float valorMaxBse = 1.5;
float valorMinBse = 0.5;
int erroAPPSBSE = 0;
int erroSHUTDOWN = 0;
int jaMudou = 0;
//------------------------------------------------------------------------
//COMUNICAÇÃO SERIAL STM32
#define UARTCOMRX 32
#define UARTCOMTX 33

#define BUFFER_SIZE 10 // Define o tamanho do nosso pacote de dados
#define START_BYTE   229

uint8_t enviado[3] = {0};
uint8_t recebido[buffSize];
unsigned long lastUpdate = 0;  // variável global para controle do tempo
uint8_t estado_IMD = 0;

HardwareSerial Serialsegundo(1);


//------------------------------------------------------------------------
//NEXTION
#define NEX_RX_PIN 16  // ou o pino que você realmente está usando
#define NEX_TX_PIN 17

HardwareSerial Serial2Nextion(2);


NexPage page0    = NexPage(0, 0, "page0");
NexPage page1    = NexPage(1, 0, "page1");
NexPage page2    = NexPage(2, 0, "page2");
NexPage page3    = NexPage(3, 0, "page3");
NexPage page4    = NexPage(4, 0, "page4");
NexPage page5    = NexPage(5, 0, "page5");

//6, 7, 10, 12, 1
NexText      appsText    = NexText(1, 4, "apps");
NexText      bseText    = NexText(1, 5, "bse");
NexText      nivelBateriaText    = NexText(1, 6, "NivelBateria");
NexText      EstadoIMD    = NexText(1, 8, "EstadoIMD");
NexText      telaErro     = NexText(2, 1, "erro");

//DADOS PLOTAR
uint8_t implausabilidadeDObse = 0;
uint8_t implausabilidadeDOapps = 0;
uint8_t shutdown = 0;
//char byteZero = 0;
//char byteOne = 1;
float appsTensao;
float aceleracaoPorcentagem;
float bseTensao;
float freioPorcentagem;
float nivelBateriaPorcentagem;
float statusIMD;
//float tensaoIMD;

char statusIMDTXT[10];
char appsTensaoTXT[10];
char bseTensaoTXT[10];
char nivelBateriaTXT[10];
char telaErroTXT[20];
char tempBuffer[10]; 

char imdStatus1TXT[30] = "Short/IMD off";
char imdStatus2TXT[30] = "Normal";
char imdStatus3TXT[30] = "Undervoltage";
char imdStatus4TXT[30] = "Speed Start";
char imdStatus5TXT[30] = "Device Error";
char imdStatus6TXT[30] = "Fault Earth Connection";
char imdStatusFinal[30];

//------------------------------------------------------------------------

uint8_t broadcastAddress[] = {0x7c, 0x9e, 0xbd, 0x45, 0xba, 0xd0};
//ESPNOW
typedef struct struct_message {
  int estadoRTD;
  int implausabilidadeAPPS;
  int implausabilidadeBSE;
  int shutdownstatus;
  float leituraAccel;
  float leituraBrake;
  float leituraBateria;
  char leituraIMD[30];
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void verificar_estado_IMD(int16_t adcIMD) {

    if (adcIMD < 0.28f) {
        EstadoIMD.setText(imdStatus1TXT);
        strcpy(imdStatusFinal, imdStatus1TXT);        
        estado_IMD = 0;
    } else if (adcIMD < 0.84f) {
        EstadoIMD.setText(imdStatus2TXT);
        strcpy(imdStatusFinal, imdStatus2TXT);        
        estado_IMD = 1;
    } else if (adcIMD < 1.4f) {
       EstadoIMD.setText(imdStatus3TXT);
        strcpy(imdStatusFinal, imdStatus3TXT);        
        estado_IMD = 2;
    } else if (adcIMD < 1.96f) {
        EstadoIMD.setText(imdStatus4TXT);
        strcpy(imdStatusFinal, imdStatus4TXT);        
        estado_IMD = 3;
    } else if (adcIMD < 2.52f) {
        EstadoIMD.setText(imdStatus5TXT);
        strcpy(imdStatusFinal, imdStatus5TXT);        
        estado_IMD = 4;
    } else {
        EstadoIMD.setText(imdStatus6TXT);
        strcpy(imdStatusFinal, imdStatus6TXT);        
        estado_IMD = 5;
    }

 
}

void trocadepagina_implausibilidade() {
  static char ultimoTextoErro[20] = ""; // Armazena o último erro mostrado
  temErro = 0;

  // Determina o tipo de erro atual
  if (shutdown == 1) {
    strcpy(telaErroTXT, "SHUTDOWN");
    temErro = 1;
  }
  else if (implausabilidadeDObse == 1 && implausabilidadeDOapps == 1) {
    strcpy(telaErroTXT, "IMPLAUSABILIDADE BSE/APPS");  
    temErro = 1;
  }  
  else if (implausabilidadeDObse == 1) {
    strcpy(telaErroTXT, "IMPLAUSABILIDADE BSE");
    erroAPPSBSE = 1;
    temErro = 1;
  }
  else if (implausabilidadeDOapps == 1) {
    strcpy(telaErroTXT, "IMPLAUSABILIDADE APPS --");  
    temErro = 1;
  }

  // ERRO APARECEU (e antes não tinha erro) → muda de página
  if (temErro == 1 && erroAnterior == 0) {
    strcpy(ultimoTextoErro, telaErroTXT);  // salva erro atual
    if (telaErroTXT[1] != 0){
    page2.show();  // mostra tela de erro
    }
    Serial.println(telaErroTXT);
    telaErro.setText(telaErroTXT);
  }

  // ERRO CONTINUA IGUAL → não faz nada (não atualiza tela ou texto)

  // ERRO SUMIU → volta para a tela principal
  if (temErro == 0 && erroAnterior == 1) {
    page1.show();
    telaErroTXT[20] = {0};
    ultimoTextoErro[0] = '\0'; // limpa o texto armazenado
  }

  erroAnterior = temErro;  // atualiza estado anterior
}

void setup() {
  enviado[0] = 229;
  //INICIALIZAÇÃO SERIAL
  Serial2Nextion.begin(112500, SERIAL_8N1, NEX_RX_PIN, NEX_TX_PIN);
  Serialsegundo.begin(9600,  SERIAL_8N1, UARTCOMRX, UARTCOMTX);
  Serial.begin(9600);
  //------------------------------------------------------------------------
  //INICIALIZAÇÃO NEXTION
  nexInit();
  page0.show();
  //------------------------------------------------------------------------
  //INICIALIZAÇÃO ESPNOW
  WiFi.mode(WIFI_STA);
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  //------------------------------------------------------------------------
  //INICIALIZAÇÃO ADS
  ads.setGain(GAIN_ONE); 
   if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  Serial.println("ESP32 pronto.");
  //page1.show();
} 
    



void loop() {
  getDataFromPC();

  if (newDataFromPC) {
   unsigned long now = millis(); 
   int16_t adc0, adc1, adc2, adc3;

   adc0 =  ads.readADC_SingleEnded(0);
   adc1 =  ads.readADC_SingleEnded(1);
   adc2 = ads.readADC_SingleEnded(2);
   adc3 = ads.readADC_SingleEnded(3);

    adcbse1 = ads.computeVolts(adc0);
    adcbse2 = ads.computeVolts(adc1);
    adcBateria = ads.computeVolts(adc2);
    adcIMD = ads.computeVolts(adc3);
   
    Serial.println(adcbse1);
    Serial.println(adcbse2);


//------------------------------------------------------------------------
    //IMPLAUSABILIDADE BSE
    //VERIFICA IMPLAUSABILIDADE DO BSE

    uint8_t adcbse1BIT = (adcbse1 >= valorMaxBse || adcbse1 <= valorMinBse) ? 1 : 0;
    uint8_t adcbse2BIT = (adcbse2 >= valorMaxBse || adcbse2 <= valorMinBse) ? 1 : 0;
    // 4. Envia os dois valores para o STM32
  if (adcbse1BIT == 1 || adcbse2BIT == 1){
      implausabilidadeDObse = 1;
      enviado[1] = 1;
  }
  else{
      implausabilidadeDObse = 0;
      enviado[1] = 0;
  }
//------------------------------------------------------------------------
//ENVIO DE DADOS PARA O STM32
  enviado[2] = 40;
  Serialsegundo.write(enviado, 3); // Envia 2º bit
//------------------------------------------------------------------------      
  //TRATAR DADOS PARA O PLOT

    shutdown = recebido[7];
    implausabilidadeDOapps = recebido[1];

  //CONCATENA OS DADOS RECEBIDOS UART DE APPS TENSAO
  if (now - lastUpdate >= 250) {  // atualiza só se passaram 100 ms

  if(recebido[6] == 1 && temErro == 0 && jaMudou == 0){
      page1.show();
      jaMudou = 1;
    }
    
    else if(recebido[6] == 0 && temErro == 0 && jaMudou == 1){
      page0.show();
      jaMudou = 0;
    }

  union {
  float f;
  uint8_t b[4];
  } tensaoUnion;
  tensaoUnion.b[0] = recebido[2];
  tensaoUnion.b[1] = recebido[3];
  tensaoUnion.b[2] = recebido[4];
  tensaoUnion.b[3] = recebido[5];
  appsTensao = tensaoUnion.f;

  //CALCULO PORCENTAGEM BATERIA
  nivelBateriaPorcentagem = (adcBateria / 3.3) * 100.0;
  
  //PORCENTAGEM ACCELERACAO E FREIO

  //PORCENTAGEM FREIO
  freioPorcentagem = ((adcbse1 - 0.5f) / (1.5f - 0.5f)) * 100.0f;
  //PORCENTAGEM ACELERAÇÃO
  aceleracaoPorcentagem = (appsTensao / 3.3) * 100.0;


//------------------------------------------------------------------------      

  dtostrf(aceleracaoPorcentagem, 3, 1, tempBuffer); // ex: "45.32"
  snprintf(appsTensaoTXT, sizeof(appsTensaoTXT), "%s%%", tempBuffer); // ex: "45.32%"

  // bseTensao com %
  dtostrf(freioPorcentagem, 3, 1, tempBuffer); 
  snprintf(bseTensaoTXT, sizeof(bseTensaoTXT), "%s%%", tempBuffer);

  // nivelBateria com %
  dtostrf(nivelBateriaPorcentagem, 4, 1, tempBuffer); 
  snprintf(nivelBateriaTXT, sizeof(nivelBateriaTXT), "%s%%", tempBuffer);
  
  trocadepagina_implausibilidade();
  verificar_estado_IMD(adcIMD);

  Serial.print("Shutdown: "); Serial.println(shutdown);
  Serial.print("BSE: "); Serial.println(implausabilidadeDObse);
  Serial.print("APPS: "); Serial.println(implausabilidadeDOapps);

  appsText.setText(appsTensaoTXT);
  bseText.setText(bseTensaoTXT);  
  nivelBateriaText.setText(nivelBateriaTXT);

   myData.estadoRTD = recebido[6];
   myData.implausabilidadeAPPS = implausabilidadeDOapps;
   myData.implausabilidadeBSE = implausabilidadeDObse;
   myData.leituraAccel = aceleracaoPorcentagem;
   myData.leituraBrake = freioPorcentagem;
   myData.leituraBateria = nivelBateriaPorcentagem;
   myData.shutdownstatus = shutdown;
   strncpy(myData.leituraIMD, imdStatusFinal, sizeof(myData.leituraIMD) - 1);
   myData.leituraIMD[sizeof(myData.leituraIMD) - 1] = '\0'; 

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
    if (result == ESP_OK) {
  Serial.println("Sent with success");
       }
      else {
  Serial.println("Error sending the data");
       }

 

  //EstadoIMD.setText()
  lastUpdate = now;  // atualiza o tempo da última atualização
  newDataFromPC = false;  // Limpa a flag para receber o próximo pacote
  }

    }

    }

    

  
void getDataFromPC() {
  while (Serialsegundo.available() > 0) {
    uint8_t byteRecebido = Serialsegundo.read();

    if (!readInProgress) {
      // Procurando o byte de início do pacote
      if (byteRecebido == startMarker) {
        readInProgress = true;
        bytesRecvd = 0;
        recebido[bytesRecvd++] = byteRecebido;
      }
    } else {
      // Já estamos dentro de um pacote em andamento
      recebido[bytesRecvd++] = byteRecebido;

      // Se recebemos o número de bytes esperado, verificamos marcadores
      if (bytesRecvd >= buffSize) {
        readInProgress = false;  // fim do pacote (válido ou não)

        // Verifica se o pacote é válido: primeiro é start e último é end
        if (recebido[0] == startMarker && recebido[buffSize-1] == endMarker) {
          newDataFromPC = true;   // pacote válido completo recebido
        }
        // Se estiver corrompido (marcadores errados), ele simplesmente é descartado

        bytesRecvd = 0;  // Prepara para o próximo pacote
      }
    }
  }
}