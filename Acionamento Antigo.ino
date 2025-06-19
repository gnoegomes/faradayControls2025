#define delayRTDS 3000

/* Código arduino Faraday Racing - 2018
 * 
 * 
 *  
 * 
 * 
 */

// PINOS
// Analogico
const int pinAcel1  = A4;                                  
const int pinAcel2  = A3;        
const int pinFreio  = A5;                                       //testar assim que tiver montada

// Digital
const int pinRTD = 5;                                             // Pino de entrada do sinal RTD
const int pinFBair = 13;                                          // Pino de entrada do sinal do feedback AIR
const int pinRTDS = 12;                                           // Pino Sirene
const int pinShtd = 11;                                           // Pino comando Relé de shutdown                                * 
const int pinSeatswitch = 4;                                      // Pino de saida do Seatswitch 
const int pinFootswitch = 2;                                      // Pino de saida do Footswitch
const int pinLedRTD = 7;                                          // Pino da led RTD  
//const int pinLedShtd = 8;                                         // Pino led Shutdown
                                                                  // Pino do sinal PWM do motor
                                                                  //Pino de Entrada que recebe o sinall de erro do IMD.


// Variáveis de input e output 
int APPS1   = 0;                                                  // Variável Acelerador1 (0~100%)
int APPS2   = 0;                                                  // Variável Acelerador2 (0~100%)                                                // variavel PWM pro motor
//int shtdout = 0;                                                  // Variável de indicação de Shutdown externo
int diferenca = 0;                                                // Variavel que ve a diferenca de Acel1 e Acel2
int freio = 0;                                                    // Variavel freio



// Variáveis de limite

//const int limitefreioSeat = 125; // valor do freio que precisa esta ativo 100~220 (medição Tav).


// Variáveis de estado
boolean feedbackAIR = 0;            // estado do FeedBack AIR
boolean estadoRTD = 0;              // estado do RTD
boolean shtdE = 0;                  // estado do sht Externo
boolean shtd = 0;                   // Variável de controle do shutdown

// Variáveis de tempo
unsigned long tempo{0};

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() 
{
  //pinMode (pinRTD         ,INPUT);
  //pinMode (pinFBair       ,INPUT);
  pinMode (pinRTDS         ,OUTPUT);
  pinMode (pinShtd           ,OUTPUT);
  pinMode (pinSeatswitch     ,OUTPUT);
  pinMode (pinFootswitch     ,OUTPUT);
  pinMode (pinLedRTD         ,OUTPUT);
  pinMode (pinLedShtd        ,OUTPUT);

  Serial.begin(9600);                               //inicia o envio de dados pela porta serial a 9600 bits/s

  

}

void loop()
{
  feedbackAIR = digitalRead(pinFBair);              // Lê estado do feedback AIR

  if(feedbackAIR == HIGH)                           // Se feedbackAIR estiver ligado vai entrar no if
  {
    estadoBrk = analogRead(pinFreio);                  // Lê se o freio esta acionado ou não
    digitalWrite(pinSeatswitch, HIGH);              // fecha a seatswitch
    estadoRTD = digitalRead(pinRTD);                // Lê estado do botão RTD
    if(estadoRTD == HIGH && estadoBrk > limitefreioSeat)                            // se o pino RTD estiver ligado e o freio ativo
    {
      digitalWrite(pinFootswitch, HIGH);            // fecha o footswitch
      digitalWrite(pinSirene, HIGH);                // Liga a Sirene
      digitalWrite(pinLedRTD, HIGH);                // Acende o LED do RTD
      delay(delaySirene);
      digitalWrite(pinSirene, LOW);                 // Desliga Sirene
      

      while(1)                                      // loop infinito onde vai pegar os valores dos sensores do acelerador
      {
       feedbackAIR = digitalRead(pinFBair);
       acel1 = map(analogRead(pinAcel1), 1005, 641, 0, 255); // Adequa a leitura do Acel1 de 0~255 valor da porta PWM
       acel2 = map(analogRead(pinAcel2), 39, 380, 0, 255);   // Adequa a leitura do Acel2 de 0~255 valor da porta PWM
       freio = analogRead(pinFreio);
       corrente= analogRead(pinCorrente);
       IMD = digitalRead(pinErroIMD);

       
       
       //========================================================================================================================================================
       // PLAUSIBILIDADE
       diferenca = abs(acel1-acel2);
     
       if(diferenca >= limDeltaAC)                            // Se Acel1 e Acel2 tiverem uma diferença de 10 
       {
        delay(300);
        acel1 = map(analogRead(pinAcel1), 1005, 641, 0, 255); // Adequa a leitura do Acel1 de 0~255 valor da porta PWM
        acel2 = map(analogRead(pinAcel2), 39, 380, 0, 255);   // Adequa a leitura do Acel2 de 0~255 valor da porta PWM
        diferenca = abs(acel1-acel2);
        
        if(diferenca >= limDeltaAC)    // Se a diferença de Acel 1 for maior que 25 (10% por regulamento, o carro tem dar Shtd)
        {
          shtd = shtd + 1;
          shtdout = 1;
          estadoRTD=0;
          digitalWrite(pinSeatswitch, LOW);
          digitalWrite(pinFootswitch, LOW);
          digitalWrite(pinLedRTD, LOW);                    
          digitalWrite(pinLedShtd, HIGH);

          analogWrite(pinMotor, 0);                         // Envia 0 de PWM pro motor
          while(diferenca >= limDeltaAC)
          {
           acel1 = map(analogRead(pinAcel1), 1005, 641, 0, 255); // Adequa a leitura do Acel1 de 0~255 valor da porta PWM
           acel2 = map(analogRead(pinAcel2), 39, 380, 0, 255);   // Adequa a leitura do Acel2 de 0~255 valor da porta PWM
           diferenca = abs(acel1-acel2); 
           Serial.print("\n Shutdown : DIFERENÇA DE ACEL1 E ACEL2");
          }
          digitalWrite(pinLedShtd, LOW);
          shtdout = 0;
        }   
                        
       }
         
       if ((pinAcel1 || pinAcel2) <= 0) // Acelerador desconectado
       {
          shtd = shtd + 1;
          shtdout = 1;
          estadoRTD=0;
          digitalWrite(pinSeatswitch, LOW);
          digitalWrite(pinFootswitch, LOW);
          digitalWrite(pinLedRTD, LOW);                    
          digitalWrite(pinLedShtd, HIGH);
          
          analogWrite(pinMotor, 0);                         // Envia 0 de PWM pro motor
          while((pinAcel1 || pinAcel2) <= 0)
          {
           Serial.print("\n Shutdown : ACELERADOR SOLTO");
          }
          digitalWrite(pinLedShtd, LOW);
          shtdout = 0;
       }
               
       if((acel1 >= limAcMax) && ((freio) >= limitefreio)) // Acelerador e freio pressionados JUNTOS 
       {
          shtd = shtd + 1;
          shtdout = 1;
          estadoRTD=0;
          digitalWrite(pinSeatswitch, LOW);
          digitalWrite(pinFootswitch, LOW);
          digitalWrite(pinLedRTD, LOW);                    
          digitalWrite(pinLedShtd, HIGH);

          analogWrite(pinMotor, 0);                         // Envia 0 de PWM pro motor
          while(acel1 > limAcMin)
          {
            acel1 = map(analogRead(pinAcel1), 1005, 641, 0, 255); // Adequa a leitura do Acel1 de 0~255 valor da porta PWM
            Serial.print("\n Shutdown : DIFERENÇA DE ACELERAÇÃO.");
          }
          digitalWrite(pinLedShtd, LOW);
          shtdout = 0;
       }
       
       if((pinFreio)<=0)
       {
          shtd = shtd + 1;
          shtdout = 1;
          estadoRTD=0;
          digitalWrite(pinSeatswitch, LOW);
          digitalWrite(pinFootswitch, LOW);
          digitalWrite(pinLedRTD, LOW);                    
          digitalWrite(pinLedShtd, HIGH);
          
          analogWrite(pinMotor, 0);                         // Envia 0 de PWM pro motor          
          while((pinFreio)<=0)
          {
            Serial.print("\n Shutdown : PINO DO FREIO DESCONECTADO.");
          }
          digitalWrite(pinLedShtd, LOW);
          shtdout = 0;
       }
         
       if(feedbackAIR) //Se feedbackAIR estiver desligado durante a aceleracao
       {
          shtd = shtd + 1;
          shtdout = 1;
          estadoRTD=0;
          digitalWrite(pinSeatswitch, LOW);
          digitalWrite(pinFootswitch, LOW);
          digitalWrite(pinLedRTD, LOW);                    
          digitalWrite(pinLedShtd, HIGH);

          analogWrite(pinMotor, 0);                         // Envia 0 de PWM pro motor          
          while(feedbackAIR)
          {
            feedbackAIR = digitalRead(pinFBair);
            Serial.print("\n Shutdown : FEEDBACKAIR ligado");
          }
          digitalWrite(pinLedShtd, LOW);
          shtdout = 0;
       }

       if(IMD) //Se o IMD for ligado durante a aceleração
       {
          shtd = shtd + 1;
          shtdout = 1;
          estadoRTD=0;
          digitalWrite(pinSeatswitch, LOW);
          digitalWrite(pinFootswitch, LOW);
          digitalWrite(pinLedRTD, LOW);                    
          digitalWrite(pinLedShtd, HIGH);
          digitalWrite(pinShtd, LOW);
          
          analogWrite(pinMotor, 0);                         // Envia 0 de PWM pro motor          
          while(IMD)
          {
            IMD = digitalRead(pinErroIMD); 
            Serial.print("\n Shutdown :  IMD ATIVO.");
          }
          digitalWrite(pinLedShtd, LOW);
          digitalWrite(pinShtd, HIGH);
          shtdout = 0;
       }
       
       //==========================================================================================================================================================
        if (estadoRTD)
        {
         analogWrite (pinMotor,acel1); //Envia o valor de acel1 em PWM para o motor 
        }else{
              analogWrite(pinMotor, 0);                         // Envia 0 de PWM pro motor
              estadoRTD=0;
              break;
             }
        
      }
       
     }
      
  
  }else{
        digitalWrite(pinSeatswitch, LOW);            // abre a seatswitch
        digitalWrite(pinFootswitch, LOW);            // abre o footswitch 
        digitalWrite(pinLedRTD, LOW);    
       }
  
}
