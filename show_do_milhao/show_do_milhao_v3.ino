#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>

// =====================================================
// LCD
// =====================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);


// =====================================================
// FITA DE LED WS2812B
// =====================================================

const int PINO_FITA = 10;
const int NUM_LEDS = 13;

Adafruit_NeoPixel fita(
  NUM_LEDS,
  PINO_FITA,
  NEO_GRB + NEO_KHZ800
);


// Cores
const uint32_t COR_J1     = fita.Color(0, 255, 0);       // Verde
const uint32_t COR_J2     = fita.Color(0, 0, 255);       // Azul
const uint32_t COR_OURO   = fita.Color(255, 255, 0);     // Amarelo
const uint32_t COR_BRANCO = fita.Color(255, 255, 255);
const uint32_t APAGADO    = fita.Color(0, 0, 0);


// =====================================================
// PINOS DOS BOTÕES
// =====================================================

// Buzzers dos jogadores
const int BUZZER_J1 = 6;
const int BUZZER_J2 = 7;

// Botões do mestre
const int BOTAO_ACERTOU = 8;
const int BOTAO_ERROU   = 9;


// =====================================================
// BUZZER DE SOM
// =====================================================

const int PINO_SOM = 13;


// =====================================================
// SETAS PERSONALIZADAS DO LCD
// =====================================================

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


// =====================================================
// MODOS DE JOGO
// =====================================================

enum ModoJogo {
  FACIL,
  HARDCORE
};

ModoJogo modoJogo = FACIL;


// =====================================================
// ESTADO DO JOGO
// =====================================================

enum Estado {
  AGUARDANDO_BUZZER,
  AGUARDANDO_JULGAMENTO,
  JOGO_ACABOU
};

Estado estado = AGUARDANDO_BUZZER;


// =====================================================
// VARIÁVEIS DO JOGO
// =====================================================

int pontos[2] = {0, 0};

const int META_PONTOS = 5;
const int PONTOS_PARA_OURO = 4;

int jogadorQueBuzou = -1;


// Indica se o jogo entrou na fase
// da Pergunta de Ouro
bool perguntaDeOuroAtiva = false;


// =====================================================
// ANIMAÇÃO LCD
// =====================================================

unsigned long ultimaAtualizacaoAnimacao = 0;

int frameAnimacao = 0;

const int INTERVALO_ANIMACAO = 400;


// =====================================================
// ANIMAÇÃO DA PERGUNTA DE OURO
// =====================================================

bool piscandoOuro = false;

bool ledOuroAceso = false;

unsigned long ultimaPiscadaOuro = 0;

const int INTERVALO_PISCA_OURO = 250;


// =====================================================
// SETUP
// =====================================================

void setup() {

  // ---------------- LCD ----------------

  lcd.init();
  lcd.backlight();

  lcd.createChar(
    CHAR_SETA_DIR,
    setaDireita
  );

  lcd.createChar(
    CHAR_SETA_ESQ,
    setaEsquerda
  );


  // ---------------- FITA ----------------

  fita.begin();

  fita.setBrightness(80);

  fita.show();


  // ---------------- BOTÕES ----------------

  pinMode(
    BUZZER_J1,
    INPUT_PULLUP
  );

  pinMode(
    BUZZER_J2,
    INPUT_PULLUP
  );

  pinMode(
    BOTAO_ACERTOU,
    INPUT_PULLUP
  );

  pinMode(
    BOTAO_ERROU,
    INPUT_PULLUP
  );


  // ---------------- SOM ----------------

  pinMode(
    PINO_SOM,
    OUTPUT
  );


  // ---------------- INÍCIO ----------------

  mostrarTelaAbertura();

  escolherModo();

  atualizarFita();

  mostrarPlacarEIniciarEspera();
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  switch (estado) {

    // -----------------------------------------
    // Esperando alguém apertar o buzzer
    // -----------------------------------------

    case AGUARDANDO_BUZZER:

      if (perguntaDeOuroAtiva) {

        animarTelaPerguntaOuro();

        animarLedOuro();

      } else {

        animarTelaEspera();
      }

      checarBuzzers();

      break;


    // -----------------------------------------
    // Jogador já apertou.
    // Esperando ACERTOU ou ERROU.
    // -----------------------------------------

    case AGUARDANDO_JULGAMENTO:

      animarLedOuro();

      checarJulgamento();

      break;


    // -----------------------------------------
    // Jogo terminou
    // -----------------------------------------

    case JOGO_ACABOU:

      // A função de vitória já fica
      // presa na animação da cobrinha.

      break;
  }
}


// =====================================================
// TELA DE ABERTURA
// =====================================================

void mostrarTelaAbertura() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Show do Milhao");

  lcd.setCursor(0, 1);
  lcd.print("Arduino Edition");

  tocarBeep(100);

  delay(150);

  tocarBeep(100);

  delay(1800);
}


// =====================================================
// ESCOLHA DO MODO
// =====================================================

void escolherModo() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Escolha o modo");

  lcd.setCursor(0, 1);
  lcd.print("V=Hard A=Easy");


  while (true) {

    // =================================================
    // ACERTOU = MODO FÁCIL
    // =================================================

    if (digitalRead(BOTAO_ACERTOU) == LOW) {

      delay(30);

      if (digitalRead(BOTAO_ACERTOU) == LOW) {

        modoJogo = FACIL;

        // Espera o botão ser solto
        while (
          digitalRead(BOTAO_ACERTOU) == LOW
        ) {
          delay(10);
        }


        lcd.clear();

        lcd.setCursor(0, 0);
        lcd.print("MODO FACIL");

        lcd.setCursor(0, 1);
        lcd.print("Primeiro a 5!");


        tocarSomSelecao();

        animacaoModoFacil();

        delay(1000);

        return;
      }
    }


    // =================================================
    // ERROU = MODO HARDCORE
    // =================================================

    if (digitalRead(BOTAO_ERROU) == LOW) {

      delay(30);

      if (digitalRead(BOTAO_ERROU) == LOW) {

        modoJogo = HARDCORE;

        // Espera o botão ser solto
        while (
          digitalRead(BOTAO_ERROU) == LOW
        ) {
          delay(10);
        }


        lcd.clear();

        lcd.setCursor(0, 0);
        lcd.print("MODO HARDCORE");

        lcd.setCursor(0, 1);
        lcd.print("OURO aos 4!");


        tocarSomSelecao();

        animacaoModoHardcore();

        delay(1000);

        return;
      }
    }
  }
}


// =====================================================
// ANIMAÇÃO DE SELEÇÃO - FÁCIL
// =====================================================

void animacaoModoFacil() {

  apagarFita();


  // Verde indo da esquerda
  for (int i = 0; i <= 4; i++) {

    fita.setPixelColor(
      i,
      COR_J1
    );

    fita.show();

    delay(80);
  }


  // Azul vindo da direita
  for (int i = 12; i >= 8; i--) {

    fita.setPixelColor(
      i,
      COR_J2
    );

    fita.show();

    delay(80);
  }


  delay(400);

  apagarFita();
}


// =====================================================
// ANIMAÇÃO DE SELEÇÃO - HARDCORE
// =====================================================

void animacaoModoHardcore() {

  apagarFita();


  // Pisca amarelo 3 vezes
  for (int vez = 0; vez < 3; vez++) {

    for (int i = 0; i < NUM_LEDS; i++) {

      fita.setPixelColor(
        i,
        COR_OURO
      );
    }

    fita.show();

    tocarBeep(70);

    delay(150);


    apagarFita();

    delay(150);
  }
}


// =====================================================
// APAGAR FITA
// =====================================================

void apagarFita() {

  for (int i = 0; i < NUM_LEDS; i++) {

    fita.setPixelColor(
      i,
      APAGADO
    );
  }

  fita.show();
}


// =====================================================
// MOSTRAR PLACAR
// =====================================================

void mostrarPlacarEIniciarEspera() {

  atualizarFita();


  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("J1:");
  lcd.print(pontos[0]);

  lcd.print("     ");

  lcd.print("J2:");
  lcd.print(pontos[1]);


  lcd.setCursor(0, 1);


  if (modoJogo == FACIL) {

    lcd.print("Modo: FACIL");

  } else {

    lcd.print("Modo: HARDCORE");
  }


  delay(1200);


  lcd.clear();

  frameAnimacao = 0;

  ultimaAtualizacaoAnimacao = millis();

  estado = AGUARDANDO_BUZZER;
}


// =====================================================
// TELA DE ESPERA NORMAL
// =====================================================

void animarTelaEspera() {

  unsigned long agora = millis();

  if (
    agora - ultimaAtualizacaoAnimacao
    < INTERVALO_ANIMACAO
  ) {

    return;
  }


  ultimaAtualizacaoAnimacao = agora;


  const char* frases[] = {

    "Feita",
    "Feita.",
    "Feita..",
    "Feita..."
  };


  lcd.setCursor(0, 0);

  lcd.print("Pergunta sendo  ");


  lcd.setCursor(0, 1);

  lcd.print(
    frases[
      frameAnimacao % 4
    ]
  );


  lcd.print("       ");


  frameAnimacao++;
}


// =====================================================
// TELA DA PERGUNTA DE OURO
// =====================================================

void animarTelaPerguntaOuro() {

  unsigned long agora = millis();

  if (
    agora - ultimaAtualizacaoAnimacao
    < 900
  ) {

    return;
  }


  ultimaAtualizacaoAnimacao = agora;


  lcd.clear();


  // Alterna duas telas
  if (frameAnimacao % 2 == 0) {

    lcd.setCursor(0, 0);
    lcd.print("PERGUNTA DE OURO");

    lcd.setCursor(0, 1);
    lcd.print("Preparem-se!");

  } else {

    lcd.setCursor(0, 0);
    lcd.print("QUEM ACERTAR");

    lcd.setCursor(0, 1);
    lcd.print("VENCE O JOGO!");
  }


  frameAnimacao++;
}


// =====================================================
// VERIFICAR BUZZERS
// =====================================================

void checarBuzzers() {

  // ===================================================
  // JOGADOR 1
  // ===================================================

  if (digitalRead(BUZZER_J1) == LOW) {

    delay(30);

    if (digitalRead(BUZZER_J1) == LOW) {

      travarBuzzer(0);

      return;
    }
  }


  // ===================================================
  // JOGADOR 2
  // ===================================================

  if (digitalRead(BUZZER_J2) == LOW) {

    delay(30);

    if (digitalRead(BUZZER_J2) == LOW) {

      travarBuzzer(1);

      return;
    }
  }
}


// =====================================================
// TRAVAR PRIMEIRO JOGADOR
// =====================================================

void travarBuzzer(int jogador) {

  jogadorQueBuzou = jogador;

  estado = AGUARDANDO_JULGAMENTO;


  tocarBeep(150);


  lcd.clear();

  lcd.setCursor(0, 0);


  // ===================================================
  // JOGADOR 1
  // ===================================================

  if (jogador == 0) {

    lcd.print("J1 apertou! ");

    lcd.write(
      CHAR_SETA_ESQ
    );
  }


  // ===================================================
  // JOGADOR 2
  // ===================================================

  else {

    lcd.print("J2 apertou! ");

    lcd.write(
      CHAR_SETA_DIR
    );
  }


  lcd.setCursor(0, 1);


  // ===================================================
  // PERGUNTA DE OURO
  // ===================================================

  if (perguntaDeOuroAtiva) {

    lcd.print("PERGUNTA OURO!");

    piscandoOuro = true;

    ultimaPiscadaOuro = millis();
  }


  // ===================================================
  // PERGUNTA NORMAL
  // ===================================================

  else {

    lcd.print("Acertou/Errou?");
  }


  // Espera o jogador soltar o botão
  while (
    digitalRead(
      jogador == 0
      ? BUZZER_J1
      : BUZZER_J2
    ) == LOW
  ) {

    delay(10);
  }
}


// =====================================================
// JULGAMENTO DO PROFESSOR
// =====================================================

void checarJulgamento() {

  bool acertou =
    digitalRead(BOTAO_ACERTOU) == LOW;

  bool errou =
    digitalRead(BOTAO_ERROU) == LOW;


  // ===================================================
  // OS DOIS BOTÕES = PULAR PERGUNTA
  // ===================================================

  if (acertou && errou) {

    delay(30);

    if (
      digitalRead(BOTAO_ACERTOU) == LOW &&
      digitalRead(BOTAO_ERROU) == LOW
    ) {

      aguardarSoltarPontos();

      proximaRodadaSemPonto();
    }

    return;
  }


  // ===================================================
  // ACERTOU
  // ===================================================

  if (acertou) {

    delay(30);

    if (
      digitalRead(BOTAO_ACERTOU) == LOW
    ) {

      aguardarSoltarPontos();

      // Quem buzou ganha o ponto
      julgarResposta(
        jogadorQueBuzou,
        true
      );
    }

    return;
  }


  // ===================================================
  // ERROU
  // ===================================================

  if (errou) {

    delay(30);

    if (
      digitalRead(BOTAO_ERROU) == LOW
    ) {

      aguardarSoltarPontos();


      // =================================================
      // NO HARDCORE + OURO
      // errar NÃO dá ponto para o adversário
      // =================================================

      if (
        modoJogo == HARDCORE &&
        perguntaDeOuroAtiva
      ) {

        julgarResposta(
          jogadorQueBuzou,
          false
        );
      }


      // =================================================
      // NAS PERGUNTAS NORMAIS
      // erro dá ponto para o adversário
      // =================================================

      else {

        int outroJogador;

        if (jogadorQueBuzou == 0) {

          outroJogador = 1;

        } else {

          outroJogador = 0;
        }


        julgarResposta(
          outroJogador,
          false
        );
      }
    }

    return;
  }
}


// =====================================================
// ESPERAR SOLTAR OS BOTÕES
// =====================================================

void aguardarSoltarPontos() {

  while (
    digitalRead(BOTAO_ACERTOU) == LOW ||
    digitalRead(BOTAO_ERROU) == LOW
  ) {

    delay(10);
  }
}


// =====================================================
// JULGAR RESPOSTA
// =====================================================

void julgarResposta(
  int jogadorQuePontua,
  bool foiAcerto
) {

  piscandoOuro = false;

  ledOuroAceso = false;


  // ===================================================
  // MODO HARDCORE
  // PERGUNTA DE OURO
  // ===================================================

  if (
    modoJogo == HARDCORE &&
    perguntaDeOuroAtiva
  ) {


    // =================================================
    // ACERTOU A PERGUNTA DE OURO
    // VENCE IMEDIATAMENTE
    // =================================================

    if (foiAcerto) {

      tocarSomAcerto();

      delay(300);

      mostrarVencedor(
        jogadorQueBuzou,
        true
      );

      return;
    }


    // =================================================
    // ERROU A PERGUNTA DE OURO
    // NINGUÉM GANHA PONTO
    // =================================================

    else {

      tocarSomErro();


      lcd.clear();

      lcd.setCursor(0, 0);

      lcd.print("RESPOSTA ERRADA");


      lcd.setCursor(0, 1);

      lcd.print("Ouro continua!");


      delay(1800);


      atualizarFita();

      frameAnimacao = 0;

      ultimaAtualizacaoAnimacao = millis();

      piscandoOuro = true;

      ultimaPiscadaOuro = millis();


      estado = AGUARDANDO_BUZZER;

      return;
    }
  }


  // ===================================================
  // PERGUNTA NORMAL
  // ===================================================

  if (foiAcerto) {

    tocarSomAcerto();

  } else {

    tocarSomErro();
  }


  // Adiciona ponto
  pontos[jogadorQuePontua]++;


  atualizarFita();


  // ===================================================
  // MODO FÁCIL
  // PRIMEIRO A 5
  // ===================================================

  if (modoJogo == FACIL) {

    if (
      pontos[jogadorQuePontua]
      >= META_PONTOS
    ) {

      mostrarVencedor(
        jogadorQuePontua,
        false
      );

      return;
    }
  }


  // ===================================================
  // MODO HARDCORE
  // CHEGOU A 4 = ATIVA PERGUNTA DE OURO
  // ===================================================

  if (modoJogo == HARDCORE) {

    if (
      pontos[0] >= PONTOS_PARA_OURO ||
      pontos[1] >= PONTOS_PARA_OURO
    ) {

      ativarPerguntaDeOuro();

      return;
    }
  }


  mostrarPlacarEIniciarEspera();
}


// =====================================================
// ATIVAR PERGUNTA DE OURO
// =====================================================

void ativarPerguntaDeOuro() {

  perguntaDeOuroAtiva = true;

  piscandoOuro = true;

  ledOuroAceso = false;

  ultimaPiscadaOuro = millis();


  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("PERGUNTA DE OURO");


  lcd.setCursor(0, 1);

  lcd.print("QUEM ACERTA VENCE");


  // Som especial
  tocarBeep(100);

  delay(100);

  tocarBeep(100);

  delay(100);

  tocarBeep(250);


  delay(1200);


  lcd.clear();

  frameAnimacao = 0;

  ultimaAtualizacaoAnimacao = millis();

  estado = AGUARDANDO_BUZZER;
}


// =====================================================
// PULAR PERGUNTA
// =====================================================

void proximaRodadaSemPonto() {

  piscandoOuro = false;

  ledOuroAceso = false;


  atualizarFita();


  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("Pergunta pulada");

  lcd.setCursor(0, 1);

  lcd.print("Sem pontos");


  delay(1200);


  // Se já estiver no Ouro,
  // continua no Ouro.
  if (perguntaDeOuroAtiva) {

    piscandoOuro = true;

    ultimaPiscadaOuro = millis();

    frameAnimacao = 0;

    ultimaAtualizacaoAnimacao = millis();

    lcd.clear();

    estado = AGUARDANDO_BUZZER;

    return;
  }


  mostrarPlacarEIniciarEspera();
}


// =====================================================
// MOSTRAR VENCEDOR
// =====================================================

void mostrarVencedor(
  int jogador,
  bool foiOuro
) {

  tocarSomVitoria();


  lcd.clear();


  // ===================================================
  // VITÓRIA NA PERGUNTA DE OURO
  // ===================================================

  if (foiOuro) {

    lcd.setCursor(0, 0);

    lcd.print("PERGUNTA OURO!");


    lcd.setCursor(0, 1);

    lcd.print("J");

    lcd.print(jogador + 1);

    lcd.print(" VENCEU!");
  }


  // ===================================================
  // VITÓRIA NORMAL
  // ===================================================

  else {

    lcd.setCursor(0, 0);

    lcd.print("J");

    lcd.print(jogador + 1);

    lcd.print(" VENCEU!");


    lcd.setCursor(0, 1);

    lcd.print("PARABENS!");
  }


  estado = JOGO_ACABOU;


  // Inicia animação infinita
  piscarVitoria(jogador);
}


// =====================================================
// SOM SIMPLES
// =====================================================

void tocarBeep(int duracaoMs) {

  digitalWrite(
    PINO_SOM,
    HIGH
  );

  delay(duracaoMs);

  digitalWrite(
    PINO_SOM,
    LOW
  );
}


// =====================================================
// SOM DE SELEÇÃO
// =====================================================

void tocarSomSelecao() {

  tocarBeep(80);

  delay(80);

  tocarBeep(150);
}


// =====================================================
// SOM DE ACERTO
// =====================================================

void tocarSomAcerto() {

  tocarBeep(100);

  delay(80);

  tocarBeep(100);
}


// =====================================================
// SOM DE ERRO
// =====================================================

void tocarSomErro() {

  tocarBeep(500);
}


// =====================================================
// SOM DE VITÓRIA
// =====================================================

void tocarSomVitoria() {

  for (int i = 0; i < 5; i++) {

    tocarBeep(120);

    delay(100);
  }
}


// =====================================================
// ATUALIZAR FITA COM O PLACAR
// =====================================================

void atualizarFita() {

  // Apaga tudo
  for (int i = 0; i < NUM_LEDS; i++) {

    fita.setPixelColor(
      i,
      APAGADO
    );
  }


  // ===================================================
  // JOGADOR 1
  // LEDs 0 até 4
  // ===================================================

  for (
    int i = 0;
    i < pontos[0] && i < 5;
    i++
  ) {

    fita.setPixelColor(
      i,
      COR_J1
    );
  }


  // ===================================================
  // JOGADOR 2
  // LEDs 12 até 8
  // ===================================================

  for (
    int i = 0;
    i < pontos[1] && i < 5;
    i++
  ) {

    fita.setPixelColor(
      12 - i,
      COR_J2
    );
  }


  fita.show();
}


// =====================================================
// PISCAR LEDs DA PERGUNTA DE OURO
// =====================================================

void animarLedOuro() {

  if (!perguntaDeOuroAtiva) {

    return;
  }


  unsigned long agora = millis();


  if (
    agora - ultimaPiscadaOuro
    < INTERVALO_PISCA_OURO
  ) {

    return;
  }


  ultimaPiscadaOuro = agora;


  ledOuroAceso =
    !ledOuroAceso;


  uint32_t cor;


  if (ledOuroAceso) {

    cor = COR_OURO;

  } else {

    cor = APAGADO;
  }


  // LEDs centrais
  fita.setPixelColor(
    5,
    cor
  );

  fita.setPixelColor(
    6,
    cor
  );

  fita.setPixelColor(
    7,
    cor
  );


  fita.show();
}


// =====================================================
// ANIMAÇÃO DE VITÓRIA
// =====================================================

void piscarVitoria(int jogador) {

  int perdedor;


  if (jogador == 0) {

    perdedor = 1;

  } else {

    perdedor = 0;
  }


  uint32_t corVencedor;


  if (jogador == 0) {

    corVencedor = COR_J1;

  } else {

    corVencedor = COR_J2;
  }


  // Pontos do perdedor ficam brancos
  apagarPontosDoPerdedor(
    perdedor
  );


  delay(300);


  // Completa lado do vencedor
  completarLadoAteOMeio(
    jogador,
    corVencedor
  );


  delay(500);


  // ===================================================
  // COBRINHA INFINITA
  // ===================================================

  const int TAMANHO_COBRA = 4;


  while (true) {

    for (
      int pos = 0;
      pos < NUM_LEDS + TAMANHO_COBRA;
      pos++
    ) {


      // Apaga fita
      for (
        int i = 0;
        i < NUM_LEDS;
        i++
      ) {

        fita.setPixelColor(
          i,
          APAGADO
        );
      }


      // Desenha cobrinha
      for (
        int c = 0;
        c < TAMANHO_COBRA;
        c++
      ) {

        int idx = pos - c;


        if (
          idx >= 0 &&
          idx < NUM_LEDS
        ) {

          fita.setPixelColor(
            idx,
            corVencedor
          );
        }
      }


      fita.show();

      delay(60);
    }
  }
}


// =====================================================
// TRANSFORMAR PONTOS DO PERDEDOR EM BRANCO
// =====================================================

void apagarPontosDoPerdedor(
  int perdedor
) {

  int qtdLeds =
    pontos[perdedor];


  if (qtdLeds > 5) {

    qtdLeds = 5;
  }


  for (
    int i = 0;
    i < qtdLeds;
    i++
  ) {

    int idx;


    if (perdedor == 0) {

      idx = i;

    } else {

      idx = 12 - i;
    }


    fita.setPixelColor(
      idx,
      COR_BRANCO
    );


    fita.show();

    delay(150);
  }
}


// =====================================================
// COMPLETAR LADO DO VENCEDOR ATÉ O MEIO
// =====================================================

void completarLadoAteOMeio(
  int vencedor,
  uint32_t cor
) {


  // ===================================================
  // JOGADOR 1
  // ===================================================

  if (vencedor == 0) {

    for (
      int idx = pontos[0];
      idx <= 6;
      idx++
    ) {

      fita.setPixelColor(
        idx,
        cor
      );

      fita.show();

      delay(150);
    }
  }


  // ===================================================
  // JOGADOR 2
  // ===================================================

  else {

    for (
      int idx = 12 - pontos[1];
      idx >= 6;
      idx--
    ) {

      fita.setPixelColor(
        idx,
        cor
      );

      fita.show();

      delay(150);
    }
  }
}