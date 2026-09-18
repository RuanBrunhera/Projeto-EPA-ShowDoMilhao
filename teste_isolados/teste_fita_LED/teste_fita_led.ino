/*
  TESTE DE FITA DE LED - WS2812B (13 LEDs)
  ------------------------------------------
  Sketch simples so pra confirmar que a fita esta ligada certo
  antes de integrar com o jogo completo.

  Ligacao:
    GND -> GND
    5V  -> 5V
    DI  -> pino 10

  IMPORTANTE: instale a biblioteca "Adafruit NeoPixel" pelo
  Gerenciador de Bibliotecas do Arduino IDE antes de compilar.
*/

#include <Adafruit_NeoPixel.h>

const int PINO_FITA = 10;
const int NUM_LEDS = 13;

Adafruit_NeoPixel fita(NUM_LEDS, PINO_FITA, NEO_GRB + NEO_KHZ800);

void setup() {
  fita.begin();
  fita.show(); // garante que comeca tudo apagado
  fita.setBrightness(80); // 0 a 255 - comeca num brilho seguro, ajuste se precisar
}

void loop() {
  // ---------- Teste 1: acende cada LED um por vez, na cor branca ----------
  for (int i = 0; i < NUM_LEDS; i++) {
    fita.clear();
    fita.setPixelColor(i, fita.Color(255, 255, 255));
    fita.show();
    delay(150);
  }

  delay(500);

  // ---------- Teste 2: acende tudo de uma vez em vermelho, verde e azul ----------
  testarCorSolida(fita.Color(255, 0, 0)); // vermelho
  testarCorSolida(fita.Color(0, 255, 0)); // verde
  testarCorSolida(fita.Color(0, 0, 255)); // azul

  fita.clear();
  fita.show();
  delay(1000);
}

void testarCorSolida(uint32_t cor) {
  for (int i = 0; i < NUM_LEDS; i++) {
    fita.setPixelColor(i, cor);
  }
  fita.show();
  delay(800);
}
