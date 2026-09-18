/*
  TESTE DE LCD - Show do Milhao
  ------------------------------
  Sketch simples so pra confirmar que o LCD esta ligado certo.
  Se aparecer o texto na tela, esta tudo ok e pode partir pro jogo completo.

  Pinagem usada (modo 4 bits):
    RS -> 12
    E  -> 11
    D4 -> 5
    D5 -> 4
    D6 -> 3
    D7 -> 2
*/

#include <LiquidCrystal.h>

// LiquidCrystal(RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  lcd.begin(16, 2); // 16 colunas, 2 linhas

  lcd.setCursor(0, 0);
  lcd.print("Show do Milhao");

  lcd.setCursor(0, 1);
  lcd.print("LCD funcionando!");
}

void loop() {
  // nao precisa fazer nada aqui, so testando o LCD
}
