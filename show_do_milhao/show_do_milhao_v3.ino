/*
  SHOW DO MILHAO - Arduino Edition (v3)
  --------------------------------------
  Diferencas da v2:
    - LCD nao mostra mais a pergunta (quem apresenta fala a pergunta
      em voz alta). Enquanto isso, o LCD fica com uma tela de espera
      animada tipo "Aguardando jogadores...".
    - Quando um jogador buza, aparece uma seta customizada apontando
      pro lado dele (J1 = seta pra direita, J2 = seta pra esquerda),
      junto com "J1 apertou!" ou "J2 apertou!".
    - Placar (pontos) aparece no LCD e tambem na fita de LED WS2812B
      (13 LEDs): verde pro J1, azul pro J2, dourado piscando na
      pergunta de ouro.
    - LCD agora e o modelo I2C (modulo com 4 pinos: GND, VCC, SDA, SCL).

  Regras do jogo (mantidas da v2):
    - Primeiro a fazer 5 pontos vence.
    - A 5a pergunta respondida por um jogador e a "pergunta de ouro":
      se ele acertar, vence na hora, mesmo estando atras no placar.
    - Segurar os dois botoes de ponto ao mesmo tempo = pula a pergunta
      (ninguem acertou, ninguem pontua).

  Pinagem:
    LCD I2C:
      GND -> GND
      VCC -> 5V
      SDA -> A4
      SCL -> A5
      (contraste se ajusta no trimpot azul do proprio modulo I2C)

    Botoes (todos INPUT_PULLUP, um terminal no pino, outro no GND):
      Buzzer Jogador 1  -> pino 6
      Buzzer Jogador 2  -> pino 7
      Botao 3 (ACERTOU) -> pino 8
      Botao 4 (ERROU)   -> pino 9

    Som:
      Modulo buzzer ATIVO (ex: Keyes, com transistor BC337) -> pino 13
        (+ ou VCC -> 5V, - ou GND -> GND, S ou IN -> pino 13)

    Logica dos botoes 3 e 4 (so tem efeito depois de alguem buzar):
      - Quem buzou (ex: J1) responde em voz alta.
      - Se acertou -> aperta o Botao 3 (ACERTOU) -> ponto vai pra quem buzou (J1).
      - Se errou   -> aperta o Botao 4 (ERROU)   -> ponto vai pro OUTRO jogador (J2).

  IMPORTANTE: instale a biblioteca "LiquidCrystal I2C" (by Frank de Brabander,
  ou "LiquidCrystal_I2C" nas bibliotecas do Arduino IDE / Tinkercad) antes
  de compilar. Se o texto nao aparecer, troque o endereco 0x27 abaixo por
  0x3F (o outro endereco comum desses modulos).

  IMPORTANTE 2: instale tambem a biblioteca "Adafruit NeoPixel" pela
  mesma tela de Gerenciador de Bibliotecas, pra fita de LED funcionar.

  Fita de LED WS2812B (13 LEDs):
    GND -> GND
    5V  -> 5V
    DI  -> pino 10

  Distribuicao dos LEDs (indices 0 a 12):
    0 a 4  -> pontuacao do Jogador 1 (verde), um LED por ponto
    5 a 7  -> meio, piscam amarelo na pergunta de ouro
    8 a 12 -> pontuacao do Jogador 2 (azul), um LED por ponto, de fora pra dentro

  Efeito de vitoria:
    1. Os pontos do jogador que PERDEU viram branco, um por um.
    2. O lado do jogador que GANHOU se completa em direcao ao meio,
       um LED por vez, ate encontrar o centro da fita.
    3. Depois disso, uma "cobrinha" fica percorrendo a fita inteira
       na cor do vencedor, infinitamente, ate resetar o Arduino.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // endereco, colunas, linhas

// ---------- Fita de LED ----------
const int PINO_FITA = 10;
const int NUM_LEDS = 13;
Adafruit_NeoPixel fita(NUM_LEDS, PINO_FITA, NEO_GRB + NEO_KHZ800);

const uint32_t COR_J1 = fita.Color(0, 255, 0);   // verde
const uint32_t COR_J2 = fita.Color(0, 0, 255);   // azul
const uint32_t COR_OURO = fita.Color(255, 255, 0); // amarelo
const uint32_t COR_BRANCO = fita.Color(255, 255, 255);
const uint32_t APAGADO = fita.Color(0, 0, 0);

// ---------- Pinos dos botoes ----------
const int BUZZER_J1 = 6;
const int BUZZER_J2 = 7;
const int BOTAO_ACERTOU = 8; // botao 3
const int BOTAO_ERROU   = 9; // botao 4

// ---------- Pino do som ----------
const int PINO_SOM = 13;

// ---------- Caracteres customizados (setas) ----------
byte setaDireita[8] = {
  0b00000,
  0b00100,
  0b00110,
  0b11111,
  0b00110,
  0b00100,
  0b00000,
  0b00000
};
byte setaEsquerda[8] = {
  0b00000,
  0b00100,
  0b01100,
  0b11111,
  0b01100,
  0b00100,
  0b00000,
  0b00000
};
const byte CHAR_SETA_DIR = 0;
const byte CHAR_SETA_ESQ = 1;

// ---------- Numero total de "rodadas" (nao ha texto de pergunta no LCD) ----------
// Esse numero e so pra controlar o fim de jogo por seguranca; o mestre
// controla as perguntas de fato (feitas em voz alta).

// ---------- Estado do jogo ----------
int pontos[2] = {0, 0};
int perguntasRespondidas[2] = {0, 0};

const int META_PONTOS = 5;
const int PERGUNTA_OURO = 5;

enum Estado { AGUARDANDO_BUZZER, AGUARDANDO_JULGAMENTO, JOGO_ACABOU };
Estado estado = AGUARDANDO_BUZZER;

int jogadorQueBuzou = -1;

// controle da animacao da tela de espera (sem usar delay(), pra nao travar a leitura dos botoes)
unsigned long ultimaAtualizacaoAnimacao = 0;
int frameAnimacao = 0;
const int INTERVALO_ANIMACAO = 400; // ms

// controle do piscar do LED de ouro (nao-bloqueante, continua ate o julgamento)
bool piscandoOuro = false;
bool ledOuroAceso = false;
unsigned long ultimaPiscadaOuro = 0;
const int INTERVALO_PISCA_OURO = 250; // ms

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.createChar(CHAR_SETA_DIR, setaDireita);
  lcd.createChar(CHAR_SETA_ESQ, setaEsquerda);

  fita.begin();
  fita.show(); // apaga todos os LEDs no inicio
  fita.setBrightness(80); // 0 a 255, ajuste se ficar forte demais
  atualizarFita();

  pinMode(BUZZER_J1, INPUT_PULLUP);
  pinMode(BUZZER_J2, INPUT_PULLUP);
  pinMode(BOTAO_ACERTOU, INPUT_PULLUP);
  pinMode(BOTAO_ERROU, INPUT_PULLUP);
  pinMode(PINO_SOM, OUTPUT);

  mostrarTelaAbertura();
  mostrarPlacarEIniciarEspera();
}

void loop() {
  switch (estado) {
    case AGUARDANDO_BUZZER:
      animarTelaEspera();
      checarBuzzers();
      break;

    case AGUARDANDO_JULGAMENTO:
      animarLedOuro();
      checarJulgamento();
      break;

    case JOGO_ACABOU:
      // trava aqui ate resetar o Arduino
      break;
  }
}

void mostrarTelaAbertura() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Show do Milhao");
  lcd.setCursor(0, 1);
  lcd.print("Preparando...");
  delay(2000);
}

void mostrarPlacarEIniciarEspera() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pontos J1:");
  lcd.print(pontos[0]);
  lcd.print(" J2:");
  lcd.print(pontos[1]);
  delay(1500);

  lcd.clear();
  frameAnimacao = 0;
  ultimaAtualizacaoAnimacao = millis();
  estado = AGUARDANDO_BUZZER;
}

// Tela de espera animada, tipo pontinhos aparecendo: "Aguardando..." "Aguardando.  " etc.
void animarTelaEspera() {
  unsigned long agora = millis();
  if (agora - ultimaAtualizacaoAnimacao < INTERVALO_ANIMACAO) return;
  ultimaAtualizacaoAnimacao = agora;

  const char* frases[] = {
    "Aguardando",
    "Aguardando.",
    "Aguardando..",
    "Aguardando..."
  };
  const int totalFrames = 4;

  lcd.setCursor(0, 0);
  lcd.print("Pergunta sendo  ");
  lcd.setCursor(0, 1);
  lcd.print(frases[frameAnimacao % totalFrames]);
  lcd.print("        "); // limpa resto da linha

  frameAnimacao++;
}

// ---------- Checagem dos buzzers ----------
void checarBuzzers() {
  if (digitalRead(BUZZER_J1) == LOW) {
    delay(30);
    if (digitalRead(BUZZER_J1) == LOW) {
      travarBuzzer(0);
      return;
    }
  }
  if (digitalRead(BUZZER_J2) == LOW) {
    delay(30);
    if (digitalRead(BUZZER_J2) == LOW) {
      travarBuzzer(1);
      return;
    }
  }
}

void travarBuzzer(int jogador) {
  jogadorQueBuzou = jogador;
  estado = AGUARDANDO_JULGAMENTO;

  tocarBeep(150); // beep curto de "trava a resposta"

  bool ehOuro = (perguntasRespondidas[jogador] + 1 == PERGUNTA_OURO);

  lcd.clear();
  lcd.setCursor(0, 0);
  if (jogador == 0) {
    lcd.print("J1 apertou! ");
    lcd.write(CHAR_SETA_ESQ); // seta aponta pro lado do J1 (esquerda da tela)
  } else {
    lcd.print("J2 apertou! ");
    lcd.write(CHAR_SETA_DIR); // seta aponta pro lado do J2 (direita da tela)
  }

  lcd.setCursor(0, 1);
  if (ehOuro) {
    lcd.print("PERGUNTA DE OURO!");
    piscandoOuro = true;
    ultimaPiscadaOuro = millis();
  } else {
    lcd.print("Acertou ou errou?");
  }

  // espera soltar o botao antes de aceitar julgamento
  while (digitalRead(jogador == 0 ? BUZZER_J1 : BUZZER_J2) == LOW) {
    delay(10);
  }
}

// ---------- Checagem do julgamento (mestre do jogo) ----------
// Botao ACERTOU -> ponto pra quem buzou. Botao ERROU -> ponto pro outro jogador.
void checarJulgamento() {
  bool acertou = (digitalRead(BOTAO_ACERTOU) == LOW);
  bool errou   = (digitalRead(BOTAO_ERROU) == LOW);

  if (acertou && errou) {
    // segurar os dois ao mesmo tempo = pular pergunta, ninguem pontua
    delay(30);
    if (digitalRead(BOTAO_ACERTOU) == LOW && digitalRead(BOTAO_ERROU) == LOW) {
      aguardarSoltarPontos();
      proximaRodadaSemPonto();
    }
    return;
  }

  if (acertou) {
    delay(30);
    if (digitalRead(BOTAO_ACERTOU) == LOW) {
      aguardarSoltarPontos();
      julgarResposta(jogadorQueBuzou); // ponto pra quem buzou
    }
    return;
  }

  if (errou) {
    delay(30);
    if (digitalRead(BOTAO_ERROU) == LOW) {
      aguardarSoltarPontos();
      int outroJogador = (jogadorQueBuzou == 0) ? 1 : 0;
      julgarResposta(outroJogador); // ponto pro adversario de quem buzou
    }
    return;
  }
}

void aguardarSoltarPontos() {
  while (digitalRead(BOTAO_ACERTOU) == LOW || digitalRead(BOTAO_ERROU) == LOW) {
    delay(10);
  }
}

void julgarResposta(int jogadorQuePontua) {
  piscandoOuro = false;
  bool foiAcerto = (jogadorQuePontua == jogadorQueBuzou);

  if (foiAcerto) {
    tocarSomAcerto();
  } else {
    tocarSomErro();
  }

  // conta a pergunta como "respondida" pra quem buzou (e quem define se e pergunta de ouro pra ele)
  perguntasRespondidas[jogadorQueBuzou]++;
  bool eraOuroDeQuemBuzou = (perguntasRespondidas[jogadorQueBuzou] == PERGUNTA_OURO);

  pontos[jogadorQuePontua]++;
  atualizarFita();

  // a pergunta de ouro so vale vitoria instantanea se quem ACERTOU (buzou e ganhou o ponto)
  // for o mesmo jogador pra quem essa era a pergunta de ouro
  if (eraOuroDeQuemBuzou && jogadorQuePontua == jogadorQueBuzou) {
    mostrarVencedor(jogadorQuePontua, true);
    return;
  }

  if (pontos[jogadorQuePontua] >= META_PONTOS) {
    mostrarVencedor(jogadorQuePontua, false);
    return;
  }

  mostrarPlacarEIniciarEspera();
}

void proximaRodadaSemPonto() {
  piscandoOuro = false;
  atualizarFita(); // restaura o placar, apagando o LED de ouro se estava piscando
  mostrarPlacarEIniciarEspera();
}

void mostrarVencedor(int jogador, bool foiOuro) {
  tocarSomVitoria();

  lcd.clear();
  lcd.setCursor(0, 0);
  if (foiOuro) {
    lcd.print("PERGUNTA DE OURO!");
    lcd.setCursor(0, 1);
    lcd.print("J");
    lcd.print(jogador + 1);
    lcd.print(" venceu!");
  } else {
    lcd.print("J");
    lcd.print(jogador + 1);
    lcd.print(" venceu!");
    lcd.setCursor(0, 1);
    lcd.print("Parabens!");
  }
  estado = JOGO_ACABOU;

  piscarVitoria(jogador); // fica rodando pra sempre a partir daqui, ate resetar o Arduino
}

// ---------- Sons (buzzer ATIVO: liga/desliga, sem variar tom) ----------
void tocarBeep(int duracaoMs) {
  digitalWrite(PINO_SOM, HIGH);
  delay(duracaoMs);
  digitalWrite(PINO_SOM, LOW);
}

void tocarSomAcerto() {
  // dois beeps curtos
  tocarBeep(100);
  delay(80);
  tocarBeep(100);
}

void tocarSomErro() {
  // um beep longo
  tocarBeep(500);
}

void tocarSomVitoria() {
  // varios beeps curtos tipo comemoracao
  for (int i = 0; i < 5; i++) {
    tocarBeep(120);
    delay(100);
  }
}

// ---------- Fita de LED ----------
// LEDs 0-4 = pontos do J1 (verde). LEDs 8-12 = pontos do J2 (azul), de fora pra dentro.
// LEDs 5,6,7 (meio) ficam apagados, exceto no flash de pergunta de ouro.
void atualizarFita() {
  for (int i = 0; i < NUM_LEDS; i++) {
    fita.setPixelColor(i, APAGADO);
  }

  for (int i = 0; i < pontos[0] && i < 5; i++) {
    fita.setPixelColor(i, COR_J1);
  }

  for (int i = 0; i < pontos[1] && i < 5; i++) {
    fita.setPixelColor(12 - i, COR_J2);
  }

  fita.show();
}

void animarLedOuro() {
  if (!piscandoOuro) return;

  unsigned long agora = millis();
  if (agora - ultimaPiscadaOuro < INTERVALO_PISCA_OURO) return;
  ultimaPiscadaOuro = agora;

  ledOuroAceso = !ledOuroAceso;
  uint32_t cor = ledOuroAceso ? COR_OURO : APAGADO;

  fita.setPixelColor(5, cor);
  fita.setPixelColor(6, cor);
  fita.setPixelColor(7, cor);
  fita.show();
}

// ---------- Efeito de vitoria ----------
// 1. Os LEDs do perdedor viram branco, um por um.
// 2. O lado do vencedor se completa em direcao ao meio, um LED por vez.
// 3. Depois disso, uma cobrinha fica percorrendo a fita inteira pra sempre.
void piscarVitoria(int jogador) {
  int perdedor = (jogador == 0) ? 1 : 0;
  uint32_t corVencedor = (jogador == 0) ? COR_J1 : COR_J2;

  apagarPontosDoPerdedor(perdedor);
  delay(300);
  completarLadoAteOMeio(jogador, corVencedor);
  delay(500);

  // ---------- Cobrinha infinita ----------
  const int TAMANHO_COBRA = 4;
  while (true) {
    for (int pos = 0; pos < NUM_LEDS + TAMANHO_COBRA; pos++) {
      for (int i = 0; i < NUM_LEDS; i++) {
        fita.setPixelColor(i, APAGADO);
      }
      for (int c = 0; c < TAMANHO_COBRA; c++) {
        int idx = pos - c;
        if (idx >= 0 && idx < NUM_LEDS) {
          fita.setPixelColor(idx, corVencedor);
        }
      }
      fita.show();
      delay(60);
    }
  }
  // nunca chega aqui, a cobrinha fica rodando pra sempre ate resetar o Arduino
}

// Vira branco, um por um, os LEDs que o perdedor tinha acendido com seus pontos
void apagarPontosDoPerdedor(int perdedor) {
  int qtdLeds = pontos[perdedor];
  if (qtdLeds > 5) qtdLeds = 5;

  for (int i = 0; i < qtdLeds; i++) {
    int idx = (perdedor == 0) ? i : (12 - i);
    fita.setPixelColor(idx, COR_BRANCO);
    fita.show();
    delay(150);
  }
}

// Acende, um por um, os LEDs que faltam do lado do vencedor ate chegar no meio da fita
void completarLadoAteOMeio(int vencedor, uint32_t cor) {
  if (vencedor == 0) {
    // J1: preenche do proximo LED depois do seu placar (indice pontos[0]) ate o indice 6 (meio)
    for (int idx = pontos[0]; idx <= 6; idx++) {
      fita.setPixelColor(idx, cor);
      fita.show();
      delay(150);
    }
  } else {
    // J2: preenche do proximo LED depois do seu placar (indice 12 - pontos[1]) ate o indice 6 (meio)
    for (int idx = 12 - pontos[1]; idx >= 6; idx--) {
      fita.setPixelColor(idx, cor);
      fita.show();
      delay(150);
    }
  }
}

