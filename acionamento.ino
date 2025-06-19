 // Pinos Analogicos
#define pinAPPS1 PBA0                                  
#define pinAPPS2 PBA2        
#define pinBSE1 PBA1
#define pinBSE2 PBA3
#define pinSOCBMS ??
#define pinMHSSTA PB15  //Frequência de erro do IMD

//Pinos Digitais
#define TSSIR PB8
#define TSSIG PB9
#define pinRTD PB11                  // Pino de entrada do sinal RTD
#define pinFBair                     // Pino de entrada do sinal do feedback AIR
#define pinRTDS PB4                  // Pino Sirene 
#define pinSDR PB10                 // Pino comando Relé de shutdown                                                         * 
#define pinSDS PB14                  // Pino de saida do Seatswitch 
#define pinSDF PB13                  // Pino de saida do Footswitch                                          
#define pinSDA PB12
#define pinBMSFault ??
#define pinIMDFault PB7

//Variáveis de Limite 
#define delayRTDS 3000


//pino Relé APPS Inversor
//#define pinLedShtd 8                                         // Pino led Shutdown

/* Código arduino Faraday Racing - 2025
 * 
 * 
 *  
 * 
 * 
 */


// Variáveis de input e output 
int APPS1{0};                                                  // Variável Acelerador1 (0~100%)
int APPS2{0};                                                  // Variável Acelerador2 (0~100%)                                             
int Freq{0};
int SOC{0};
int BSE1{0};   
int BSE2{0};
unsigned int Tempo{0}; // Variavel freio




//const int totalBreak ???

//const int limitefreioSeat = 125; // valor do freio que precisa esta ativo 100~220 (medição Tav).


// Variáveis de estado
boolean feedbackAIR = 0;            // estado do FeedBack AIR
boolean estadoRTD = 0;              // estado do RTD
boolean shtd = 0;                   // Variável de controle do shutdown


// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void Shutdown(){
feedbackAIR = false; 
digitalWrite(pinFootswitch, LOW);   
digitalWrite(pinSeatswitch, LOW);  
digitalWrite(pinRAPPS, LOW);
shtd = True;
estadoRTD = False;
}

void ImplausabilidadeAPPS(int input1,int input2, unsigned int tempo){
  int timer;
  float diff = (input1 + input2);
  if (abs(diff/input1) > 0.1){
  timer = tempo;
  if (timer > 100){
  Shutdown();
  }}
  else {
  timer = 0;
  }
  

void ImplausabilidadeBSE(int input1, int input2, unsigned int tempo){
}








void setup() 
{
  pinMode (pinRTD         ,INPUT);
  pinMode (pinFBair       ,INPUT);
  pinMode (pinSDA        ,OUTPUT);
  pinMode (pinRTDS         ,OUTPUT);
  pinMode (pinSDR           ,OUTPUT);
  pinMode (pinSDS    ,OUTPUT);
  pinMode (pinSDF     ,OUTPUT);


  Serial.begin(9600);                               //inicia o envio de dados pela porta serial a 9600 bits/s

  

}

void loop()
{
Tempo = HAL_GetTick();

if(estadoRTD){
 digitalWrite(pinRAPPS, HIGH);   

 APPS1 = analogRead(pinAPPS2);
 APPS2 = analogRead(pinAPPS1); 
 ImplausabilidadeAPPS(APPS1,APPS2);
 
 brake = analogRead(brake);
 ImplausabilidadeBSE(brake);

 SOC = analogRead(pinBMS);

 if(digitalRead(pinBMSfault)){
   Shutdown();
 }

 if(digitalRead(pinIMDfault)){
   Freq = analogRead(pinMHSSTA);
   Shutdown();
 }
 
 
 delay(taxaA);
 
}  
else{
    feedbackAIR = digitalRead(pinFBair);              // Lê estado do feedback AIR, ou seja, o HV está ativado
    if(feedbackAIR == True)                           // Se feedbackAIR estiver ligado vai entrar no if
    {
      BSE1 = analogRead(pinBSE1);                  // Lê se o freio esta acionado ou não
      digitalWrite(pinSDF, HIGH);              // fecha a seatswitch
      if(BSE1 >= totalBrake && digitalRead(pinRTD) ){
        digitalWrite(pinSDS, HIGH);            // fecha o footswitch
        digitalWrite(pinRTDS, HIGH);                  // Liga a Sirene             // Acende o LED do RTD
        delay(delaySirene);
        digitalWrite(pinRTDS, LOW);                 // Desliga Sirene   
        estadoRTD = True;
      };               
    }}      
}
