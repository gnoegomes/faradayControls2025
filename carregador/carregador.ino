//CODIGO CARREGADOR 
// Include Wire Library for I2C
#include <Wire.h>

// Include Adafruit Graphics & OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>



#define OLED_RESET 4

// Pino ADC
const int analogPin = 14;  // Use um pino ADC válido do ESP32

//Pinos Erro
const int BMS1 = 34;
const int BMS2 = 35;
const int BMS3 = 33;
const int BMS4 = 32;

// Configurações do ADC
const float tensaoMaxima = 3.3;      // Tensão máxima (100%)
const int resolucaoADC = 4095;    // 12 bits -> valores de 0 a 4095

// Verificacáão ERRO BMS
char Erro_BMS1 = 0;
char Erro_BMS2 = 0;
char Erro_BMS3 = 0;
char Erro_BMS4 = 0;


Adafruit_SSD1306 display(OLED_RESET);



void setup() {
  Wire.begin();
  Serial.begin(115200);  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  analogReadResolution(12); 
  pinMode(BMS1, INPUT); 
  pinMode(BMS2, INPUT); 
  pinMode(BMS3, INPUT); 
  pinMode(BMS4, INPUT); 
}

void loop() {
  display.clearDisplay();

  //PLAUSABILIDADE DO BMS
  //-------------------------------------------------------------------------------
  if (digitalRead(BMS1) == LOW){
     Erro_BMS1 = 1;
  }
  else{
    Erro_BMS1 = 0;
  }
  if (digitalRead(BMS2) == LOW){
     Erro_BMS2 = 1;
  }
  else{
    Erro_BMS2 = 0;
  }
 if (digitalRead(BMS3) == LOW){
     Erro_BMS3 = 1;
  }
  else{
    Erro_BMS3 = 0;
  }
 if (digitalRead(BMS4) == LOW){
     Erro_BMS4 = 1;
  }
  else{
    Erro_BMS4 = 0;
  }

while (Erro_BMS1 || Erro_BMS2 || Erro_BMS3 || Erro_BMS4){
  if (Erro_BMS1 == 0){
      display.setCursor(0,0);
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.print("Erro BMS1");
  }
  if (Erro_BMS2 == 0){
      display.setCursor(0,16);
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.print("Erro BMS2");
  }
  if (Erro_BMS3 == 0){
      display.setCursor(0,32);
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.print("Erro BMS3");
  }
  if (Erro_BMS4 == 0){
      display.setCursor(0,48);
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.print("Erro BMS4");
  }
}
//-------------------------------------------------------------------------------
  


  int leituraBateria = analogRead(analogPin);
  // Converte leitura para tensão
  float tensaoBateria = (leituraBateria * tensaoMaxima) / resolucaoADC;
  // Converte para porcentagem da "carga da bateria"
  float porcentagemBateria = round((tensaoBateria / tensaoMaxima) * 100.0);
  // Mostra os resultados
  
 
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.print("| Tensão: ");
  display.print(tensaoBateria, 2);
  display.print("V");

  display.setCursor(0,32);
  display.print("Percentual de carga: ");
  display.print(tensaoBateria, 2);
  display.print("%");

  delay(100); // Aguarda 100 milisegundos antes de repetir
}
