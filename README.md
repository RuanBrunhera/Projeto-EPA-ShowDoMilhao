# 🎮 Show do Milhão - Arduino Edition

Jogo de perguntas e respostas estilo *Show do Milhão*, feito com Arduino, para dois jogadores competirem em modo "buzzer" — quem apertar primeiro tem a vez de responder em voz alta.

Projeto desenvolvido para a disciplina de **Qualidade e Teste de Software**, ETEC 246, turma 3º DS.

## 👥 Integrantes

- Ruan Aronchi
- Ismael Netto
- João Belaz
- Pedro Colodeto
- Mateus Mendes

## 🕹️ Como funciona

- O apresentador faz a pergunta em voz alta (o LCD não mostra o texto da pergunta, só uma tela de espera animada).
- O jogador que souber a resposta aperta seu **buzzer** — o LCD trava mostrando quem apertou primeiro, com uma seta indicando o lado.
- O apresentador confirma se a resposta foi certa ou errada apertando um dos dois botões de julgamento:
  - **ACERTOU** → ponto vai para quem apertou o buzzer.
  - **ERROU** → ponto vai para o adversário.
  - Segurar os dois ao mesmo tempo → pula a pergunta, ninguém pontua.
- **Primeiro a fazer 5 pontos vence.**
- A **5ª pergunta respondida** por cada jogador é a "pergunta de ouro" — se ele acertar, vence na hora, mesmo estando atrás no placar. Enquanto essa pergunta está em aberto, o LED amarelo do meio da fita fica piscando.
- Ao final, a fita de LED faz uma animação: os pontos do perdedor viram branco, o lado do vencedor se completa até o meio, e uma "cobrinha" fica percorrendo a fita na cor do vencedor até o Arduino ser resetado.

## 🔌 Hardware utilizado

| Componente | Quantidade |
|---|---|
| Arduino Uno | 1 |
| Display LCD 16x2 com módulo I2C | 1 |
| Fita de LED WS2812B (Neopixel), 13 LEDs | 1 |
| Módulo de buzzer ativo (ex: Keyes, com transistor BC337) | 1 |
| Botão (push button) | 4 |
| Protoboard | 1 |
| Jumpers | conforme necessário |
| Cabo USB tipo B | 1 |

## 📌 Pinagem

| Componente | Pino no Arduino |
|---|---|
| LCD I2C — SDA | A4 |
| LCD I2C — SCL | A5 |
| LCD I2C — VCC | 5V |
| LCD I2C — GND | GND |
| Buzzer Jogador 1 | 6 |
| Buzzer Jogador 2 | 7 |
| Botão ACERTOU | 8 |
| Botão ERROU | 9 |
| Fita de LED — DI (dados) | 10 |
| Módulo de som — S/IN | 13 |

> Todos os botões usam `INPUT_PULLUP` — basta ligar um terminal no pino digital e o outro no GND, sem necessidade de resistor externo.

## 📚 Bibliotecas necessárias

Instale pelo Gerenciador de Bibliotecas do Arduino IDE (`Sketch > Incluir Biblioteca > Gerenciar Bibliotecas`):

- **LiquidCrystal I2C** (by Frank de Brabander)
- **Adafruit NeoPixel**

## 🚀 Como usar

1. Clone o repositório:
   ```bash
   git clone https://github.com/seu-usuario/show-do-milhao-arduino.git
   ```
2. Abra o arquivo `show_do_milhao_v3/show_do_milhao_v3.ino` no Arduino IDE.
3. Instale as bibliotecas listadas acima.
4. Monte o circuito conforme a tabela de pinagem (ou veja o diagrama, se houver, na pasta `docs/`).
5. Conecte o Arduino, selecione a placa/porta correta e faça o upload do código.

## 📁 Estrutura do repositório

```
show-do-milhao-arduino/
├── show_do_milhao_v3/
│   └── show_do_milhao_v3.ino     # versão final e completa do jogo
├── teste_lcd/
│   └── teste_lcd.ino              # sketch simples pra testar o LCD isolado
├── teste_buzzer/
│   └── teste_buzzer.ino           # sketch pra identificar buzzer ativo/passivo
├── teste_fita_led/
│   └── teste_fita_led.ino         # sketch pra testar a fita de LED isolada
└── README.md
```

## 🧪 Testes

A documentação de pontos de teste e casos de teste elaborados para este projeto está disponível em [`docs/testes.md`](docs/testes.md) (ou no relatório entregue para a disciplina).

## 📄 Licença

Projeto acadêmico, sem fins comerciais — sinta-se à vontade para usar como referência de estudo.
