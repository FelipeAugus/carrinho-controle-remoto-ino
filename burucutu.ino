#include "BluetoothSerial.h"
BluetoothSerial BT;

// Configs do carro
#define HP_MAXIMO 100 // Vida
#define DAMAGE 10 // Dano por tiro
#define VELOCIDADE_INICIAL 220
#define VELOCIDADE_MENOR_80pct 200
#define VELOCIDADE_MENOR_40pct 180
#define TENSAO_DANO_LDR 2000 //tensao dano LDR 
#define TEMPO_TIRO 3000
#define TEMPO_DANO 5000


// Definindo os pinos
#define MOTOR_ESQ_FRENTE 27
#define MOTOR_ESQ_TRAS 33
#define MOTOR_DIR_FRENTE 26
#define MOTOR_DIR_TRAS 25

#define LASER1 00
#define LASER2 00
#define LED_LASER1 00
#define LED_LASER2 00

// #define LED_RGB_R 18
#define LED_RGB_G 21
#define LED_RGB_B 19

#define LDR1  13
#define LDR2  2
#define LDR3  12


// Variaveis de controle
int vida = HP_MAXIMO;
int velocidade = 220;  //Controle da velocidade do motor

unsigned long hora_dano = millis();
unsigned long hora_tiro_laser1 = millis()-TEMPO_TIRO;
unsigned long hora_tiro_laser2= millis()-TEMPO_TIRO;

bool ultima_direcao = true;

void ativaLedBT() {
  if(BT.connected()) digitalWrite(LED_BUILTIN, LOW);
  else digitalWrite(LED_BUILTIN, HIGH);
}

// Função para verificar a vida
void verificaVida() {
    /*
    Serial.print("Resistencia: ");  
    Serial.println(R);
    Serial.print ("HP: ");
    Serial.println(HP);
    */
    float resistencia_ldr1 = analogRead(LDR1);
    float resistencia_ldr2 = analogRead(LDR2);
    float resistencia_ldr3 = analogRead(LDR3);
    
    Serial.print("R1: ");
    Serial.println(resistencia_ldr1);
    Serial.print("R2: ");
    Serial.println(resistencia_ldr2);
    Serial.print("R3: ");
    Serial.println(resistencia_ldr3);

    bool tomou_dano = false;
    if (   ((resistencia_ldr1 > TENSAO_DANO_LDR) || (resistencia_ldr2 > TENSAO_DANO_LDR) || (resistencia_ldr3 > TENSAO_DANO_LDR) )
        && (millis() - hora_dano >= TEMPO_DANO)){      
        Serial.println(hora_dano);
        Serial.println("XXXXX TOMOU DANO XXXXX");
        vida = vida - DAMAGE;
        hora_dano = millis();
        tomou_dano= true;
    }

    if (vida >= 80) {
        velocidade = VELOCIDADE_INICIAL;
        // digitalWrite(LED_RGB_R, LOW);//Coloca vermelho em nível baixo
        digitalWrite(LED_RGB_G, HIGH);//Coloca verde em nível alto
        digitalWrite(LED_RGB_B, LOW);//Coloca azul em nível baixo
    } else if (vida >= 40 && vida < 80) {
        velocidade = VELOCIDADE_MENOR_80pct;
        // digitalWrite(LED_RGB_R, HIGH);//Coloca vermelho em nível alto
        digitalWrite(LED_RGB_G, LOW);//Coloca verde em nível alto
        digitalWrite(LED_RGB_B, HIGH);//Coloca azul em nível baixo
    } else if (vida > 0 && vida < 40){
        velocidade = VELOCIDADE_MENOR_40pct;
        // digitalWrite(LED_RGB_R, HIGH);//Coloca vermelho em nível alto
        digitalWrite(LED_RGB_G, HIGH);//Coloca verde em nível baixo
        digitalWrite(LED_RGB_B, HIGH);//Coloca azul em nível baixo
    } else {
        velocidade = 0;
        // digitalWrite(LED_RGB_R, LOW);//Coloca vermelho em nível baixo
        digitalWrite(LED_RGB_G, LOW);//Coloca verde em nível baixo
        digitalWrite(LED_RGB_B, LOW);//Coloca azul em nível baixo
        vida = 100;
    }
    Serial.print("VIDA: ");
    Serial.println(vida);

    if(tomou_dano) {
      Serial.println("XXXXX REDUZINDO VELOCIDADE XXXXX");
      if(ultima_direcao){  // Se true era para frente
        analogWrite(MOTOR_ESQ_FRENTE, velocidade);
        analogWrite(MOTOR_DIR_FRENTE, velocidade);
      } else { 
        analogWrite(MOTOR_ESQ_TRAS, velocidade);
        analogWrite(MOTOR_DIR_TRAS, velocidade);
      }
      if(velocidade == 0) delay(10000);
    }

    
}

// Função para verificar a municao
void verificaMunicao() {
    if (millis() - hora_tiro_laser1 >= TEMPO_TIRO) digitalWrite(LED_LASER1, HIGH);
    if (millis() - hora_tiro_laser2 >= TEMPO_TIRO) digitalWrite(LED_LASER2, HIGH);
}

// Função para atirar
void atira(int arma) {
    if(arma == 1) {
        // Verifica se pode atirar
        if (millis() - hora_tiro_laser1 >= TEMPO_TIRO) {
            Serial.print(millis());
            Serial.println("   TIRO TIRO TIRO");  
            digitalWrite (LASER1, HIGH);
            delay(1000);
            digitalWrite(LASER1, LOW);
            digitalWrite(LED_LASER1, LOW);
            hora_tiro_laser1 = millis();
        }
    } else if(arma == 2) {
        // Verifica se pode atirar
        if (millis() - hora_tiro_laser2 >= TEMPO_TIRO) {
            Serial.print(millis());
            Serial.println("   TIRO TIRO TIRO");  
            digitalWrite (LASER2, HIGH);
            delay(1000);
            digitalWrite(LASER2, LOW);
            digitalWrite(LED_LASER2, LOW);
            hora_tiro_laser2 = millis();
        }
    }
}

// Função para escutar manete
void escutaManete() {
  char comando = 'c';

  if(BT.available()) {
    comando = BT.read();
    Serial.println("Av");
  }
  if(comando != 'c'){
    Serial.print("COMANDO: ");
    Serial.println(comando);
  }
  switch(comando) {
      case 'U': {
        // Frente
        analogWrite(MOTOR_ESQ_FRENTE, velocidade);  
        analogWrite(MOTOR_ESQ_TRAS, 0);

        analogWrite(MOTOR_DIR_FRENTE, velocidade);  
        analogWrite(MOTOR_DIR_TRAS, 0);
        ultima_direcao = true;
        break;
      }
      case 'D': {
        // Ré  
        analogWrite(MOTOR_ESQ_FRENTE, 0);  
        analogWrite(MOTOR_ESQ_TRAS, velocidade);

        analogWrite(MOTOR_DIR_FRENTE, 0);  
        analogWrite(MOTOR_DIR_TRAS, velocidade);
        ultima_direcao = false;
        break;
      }
      case 'L': {
        // Esquerda
        if(ultima_direcao){  // Se true era para frente
          analogWrite(MOTOR_ESQ_FRENTE, 0);  
          analogWrite(MOTOR_ESQ_TRAS, velocidade);
          analogWrite(MOTOR_DIR_FRENTE, velocidade);  
          analogWrite(MOTOR_DIR_TRAS, 0);
        } else {
          analogWrite(MOTOR_ESQ_FRENTE, velocidade);  
          analogWrite(MOTOR_ESQ_TRAS, 0);
          analogWrite(MOTOR_DIR_FRENTE, 0);  
          analogWrite(MOTOR_DIR_TRAS, velocidade);
        }  
        break;
      }
      case 'R': {
        // Direita
        if(ultima_direcao){  // Se true era para frente
          analogWrite(MOTOR_ESQ_FRENTE, velocidade);  
          analogWrite(MOTOR_ESQ_TRAS, 0);
          analogWrite(MOTOR_DIR_FRENTE, 0);  
          analogWrite(MOTOR_DIR_TRAS, velocidade);
        } else {
          analogWrite(MOTOR_ESQ_FRENTE, 0);  
          analogWrite(MOTOR_ESQ_TRAS, velocidade);
          analogWrite(MOTOR_DIR_FRENTE, velocidade);  
          analogWrite(MOTOR_DIR_TRAS, 0);
        }  
        break;
      }
      case '1': {
        atira(1);
        break;
      }
      case '2': {
        atira(2);
        break;
      }
      /*
      case 'A': {
        // Aumenta PWM motor
        if(velocidade < 255) velocidade += 10;
        if(ultima_direcao){  // Se true era para frente
          analogWrite(MOTOR_ESQ_FRENTE, velocidade);
          analogWrite(MOTOR_DIR_FRENTE, velocidade);
        } else { 
          analogWrite(MOTOR_ESQ_TRAS, velocidade);
          analogWrite(MOTOR_DIR_TRAS, velocidade);
        }  
        break;
      }
      case 'B': {
        // Reduz PWM motor
        if(velocidade > 175) velocidade -= 10;
        if(ultima_direcao){  // Se true era para frente
          analogWrite(MOTOR_ESQ_FRENTE, velocidade);
          analogWrite(MOTOR_DIR_FRENTE, velocidade);
        } else { 
          analogWrite(MOTOR_ESQ_TRAS, velocidade);
          analogWrite(MOTOR_DIR_TRAS, velocidade);
        }  
        break;
      }
      */
      case 'X': {
        // Para  
        analogWrite(MOTOR_ESQ_FRENTE, 0);  
        analogWrite(MOTOR_ESQ_TRAS, 0);

        analogWrite(MOTOR_DIR_FRENTE, 0);  
        analogWrite(MOTOR_DIR_TRAS, 0);
        break;
      }
      default:
        break;
    }
}

void setup() {
  
  // Inicializa os pinos do motor
  pinMode(MOTOR_DIR_FRENTE, OUTPUT);
  pinMode(MOTOR_DIR_TRAS, OUTPUT);
  pinMode(MOTOR_ESQ_FRENTE, OUTPUT);
  pinMode(MOTOR_ESQ_TRAS, OUTPUT);

  pinMode(LED_RGB_B, OUTPUT);       //Define a variável azul como saída
  pinMode(LED_RGB_G, OUTPUT);      //Define a variável verde como saída
  // pinMode(LED_RGB_R, OUTPUT);   //Define a variável vermelho como saída

  pinMode(LDR1, INPUT);
  pinMode(LDR2, INPUT);
  pinMode(LDR3, INPUT);
/*
  pinMode(LED_LASER1, OUTPUT);
  pinMode(LED_LASER2, OUTPUT);

  pinMode(LASER1, OUTPUT);
  pinMode(LASER2, OUTPUT);
*/

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  BT.begin("Brucutu");
}

void loop() {
  ativaLedBT();

  //Chamando as funções para verificar vida e escutar manete
  verificaVida();
  verificaMunicao();
  escutaManete();
  delay(1000);
}
