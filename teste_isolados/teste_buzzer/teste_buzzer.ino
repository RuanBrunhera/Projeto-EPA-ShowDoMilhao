/*
  TESTE DE BUZZER
  ----------------
  Sketch simples pra descobrir se o buzzer e ATIVO ou PASSIVO,
  e confirmar se a ligacao esta correta.

  Ligacao do modulo de buzzer:
    + (VCC) -> 5V
    - (GND) -> GND
    S (sinal) -> pino 13

  Como interpretar o teste:
    - Se ouvir um "bip" continuo e constante durante o digitalWrite HIGH,
      e silencio total no tone() -> seu buzzer e ATIVO.
    - Se ouvir os tons diferentes (grave, medio, agudo) na parte do tone() ->
      seu buzzer e PASSIVO (esse e o que o codigo do jogo precisa pra variar o som).
    - Se nao ouvir NADA em nenhuma das duas partes -> problema de fiacao,
      pino errado, ou buzzer com defeito/pino S nao conectado direito.
*/

const int PINO_SOM = 13;

void setup() {
  pinMode(PINO_SOM, OUTPUT);
}

void loop() {
  // ---------- Teste 1: digitalWrite (buzzer ATIVO responde a isso) ----------
  digitalWrite(PINO_SOM, HIGH);
  delay(1000);
  digitalWrite(PINO_SOM, LOW);
  delay(1000);

  // ---------- Teste 2: tone() em 3 frequencias (buzzer PASSIVO responde a isso) ----------
  tone(PINO_SOM, 300);  // grave
  delay(700);
  noTone(PINO_SOM);
  delay(300);

  tone(PINO_SOM, 1000); // medio
  delay(700);
  noTone(PINO_SOM);
  delay(300);

  tone(PINO_SOM, 2500); // agudo
  delay(700);
  noTone(PINO_SOM);
  delay(1000);
}
