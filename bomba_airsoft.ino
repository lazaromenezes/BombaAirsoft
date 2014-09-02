#include <RGBLed.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Time.h>

/*************************************************
 * Objetos Hardware
 *************************************************/

RGBLed rgb1(13, 12, 11);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Fios da bomba, cada pino indica um fio
int fios[3] = {2, 3, 4};

int pinoBuzzer = 5;

int pinoArmar = 6;
int pinoModo = 7;
int pinoDown = 8;
int pinoUp = 9;

/*************************************************
 * Variveis para logica do funcionamento
 *************************************************/
int fioEscolhido;
long tempoBomba;
int modoAtual;

const unsigned int *sequencias[2][3] = { 
  {RGBLed::COR_VERMELHO, RGBLed::COR_AMARELO, RGBLed::COR_VERDE},
  {RGBLed::COR_AMARELO, RGBLed::COR_VERDE, RGBLed::COR_AZUL}
};

boolean mudouModo = false;

boolean explodiu = false;
boolean armada = false;

int tempoDelay = 1000;

const int MODO_JOGO = 0;
const int MODO_CONFIGURACAO = 1;
const int MODO_TESTE = 2;

struct Configuracao{
  int tempoBomba;
  int sequencia; 
};

Configuracao configuracao;

/*************************************************
 * Funcoes e procedimentos
 *************************************************/

/*
 * Mostra um texto no LCD
 */
void mostrar(char* mensagem, int linha, int coluna, boolean limpar = false){
  if(limpar){
    lcd.clear(); 
  }

  lcd.setCursor(linha, coluna);
  lcd.print(mensagem);
}

/*
 * Mostra um número no LCD
 */
void mostrar(long mensagem, int linha, int coluna, boolean limpar = false){
  if(limpar){
    lcd.clear(); 
  }

  lcd.setCursor(linha, coluna);
  lcd.print(mensagem);

}

/*
 * Mostra o relógio
 */
void mostrarRelogio(long tempoMillis){
  
  tmElements_t tm;
  time_t tempo = tempoMillis / 1000UL;

  breakTime(tempo, tm);

  char buffer[16];
  sprintf(buffer, "%02d:%02d:%02d", tm.Hour, tm.Minute, tm.Second);
  mostrar(buffer, 0, 1);
  
}

void reset(){
  explodiu = false;
  armada = false;
  tempoBomba = configuracao.tempoBomba;
  digitalWrite(pinoBuzzer, LOW);
  rgb1.apagar();
}

int obterFioSolto(){
  int qtdeFios = sizeof(fios) / sizeof(*fios);
  for(int i = 0; i < qtdeFios; i++){
    if(digitalRead(fios[i]) == LOW){
      return fios[i]; 
    }
  }
  return -1;
}

// Explode a bomba
void explodir(){
  mostrar("EXPLODIU!!!", 0, 0, true);
  mostrar(":(", 0, 1);
  digitalWrite(pinoBuzzer, 255);
  rgb1.acender(RGBLed::COR_VERMELHO);
  explodiu = true;
}

// Desarma a bomba
void desarmar(){
  mostrar("DESARMADA!!!", 0, 0, true);
  mostrar(":)", 0, 1);
  rgb1.acender(RGBLed::COR_VERDE);
  armada = false;
}

// Sorteia um fio para a bomba
int sorteiaFio(){
  randomSeed(analogRead(0));
  return random(2, 5);
}

// Arma a bomba (reseta o jogo)
void armar(long tempoEmMinutos){
  if(obterFioSolto() > -1){
    mostrar("POR FAVOR, LIGUE ", 0, 0, true);
    mostrar("OS FIOS.", 0, 1);
  }
  else{
    explodiu = false;
    armada = true;    
    fioEscolhido = sorteiaFio();
    tempoBomba = tempoEmMinutos * 60 * 1000;
    mostrar("Bomba armada", 0, 0, true);
    digitalWrite(pinoBuzzer, LOW); 
    rgb1.apagar(); 
  }
}

// Mostrar Sequencia
void exibirSequencia(int sequenciaEscolhida){

  const unsigned int **sequencia = sequencias[sequenciaEscolhida];

  // Uma sequencia começa sempre com o branco
  rgb1.acender(RGBLed::COR_BRANCO);
  delay(500);

  const unsigned int *Cor1 = sequencia[0];
  rgb1.acender(Cor1[0], Cor1[1], Cor1[2]);
  delay(500);
  
  const unsigned int *Cor2 = sequencia[1];
  rgb1.acender(Cor2[0], Cor2[1], Cor2[2]);
  delay(500);

  const unsigned int *Cor3 = sequencia[2];
  rgb1.acender(Cor3[0], Cor3[1], Cor3[2]);
  delay(500);

}

/*************************************************
 * Estrutura arduino
 *************************************************/
void setup(){

  pinMode(pinoBuzzer, OUTPUT);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  
  mostrar("BOMBA AIRSOFT", 0, 0, true);
  mostrar("V 0.5", 0, 1);
  
  delay(2000);
  
  lcd.clear();

  modoAtual = MODO_TESTE;
  mudouModo = true;
}

void loop(){  



  switch(modoAtual){
  case MODO_JOGO:
  
    if(mudouModo){
      mostrar("MODO JOGO", 0, 0, true);
      mostrar("PRESS. ARMAR", 0, 1);
      mudouModo = false; 
    }
  
    if(digitalRead(pinoArmar) == HIGH){      
      if(!armada){
        armar(configuracao.tempoBomba);
        delay(100);
      }
    }else if(digitalRead(pinoModo) == HIGH){
     modoAtual = MODO_CONFIGURACAO;
     mudouModo = true;
     reset();
     delay(100);
    }
    else if(armada && !explodiu){
      
      mostrarRelogio(tempoBomba);
      delay(tempoDelay);  
      tempoBomba -= tempoDelay;
            
      if(tempoBomba > 0){

        int fioSolto = obterFioSolto();

        if(fioSolto != -1){
          if(fioSolto != fioEscolhido){
            explodir();       
          }
          else{
            desarmar();
          } 
        }

      }
      else{
        explodir(); 
      }

    }
    break;
  case MODO_CONFIGURACAO:      
    
    if(mudouModo){
      mostrar("MODO CONFIG.", 0, 0, true);
      mostrar("TEMPO BOMBA: ", 0, 1);
      mudouModo = false;
    }
    
    if(digitalRead(pinoArmar) == HIGH){
      //con
    }
    
    if(digitalRead(pinoUp) == HIGH){
      configuracao.tempoBomba++;
      delay(250);
    }
    
    if(digitalRead(pinoDown) == HIGH){
      if(configuracao.tempoBomba > 0){
        configuracao.tempoBomba--;      
      }
      delay(250);
    }
    
    if(digitalRead(pinoModo) == HIGH){
      modoAtual = MODO_TESTE;
      mudouModo = true;
      delay(250);
    }
    
    mostrar("       ", 13, 1); // GAMBIARRA PARA LIMPAR O DISPLAY DO TEMPO
    mostrar(configuracao.tempoBomba, 13, 1);
    
    delay(100);
    
    break;
  case MODO_TESTE:

    if(mudouModo){
      mostrar("MODO TESTE", 0, 0, true);
      mudouModo = false; 
    }

    if(digitalRead(pinoModo) == HIGH){
      modoAtual = MODO_JOGO;
      mudouModo = true;
      delay(250);
    }

    if(digitalRead(pinoUp) == HIGH){
      if(configuracao.sequencia < 1){
        configuracao.sequencia++;
      }
    }

    if(digitalRead(pinoDown) == HIGH){
      if(configuracao.sequencia > 0){
        configuracao.sequencia--;
      }
    }

    mostrar("SEQ: ", 0, 1);
    mostrar(configuracao.sequencia, 5, 1);

    exibirSequencia(configuracao.sequencia);

    break;
  }


}