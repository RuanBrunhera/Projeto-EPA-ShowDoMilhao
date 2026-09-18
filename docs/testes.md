# 🧪 Documentação de Testes - Show do Milhão Arduino

Disciplina: Qualidade e Teste de Software — ETEC 246, turma 3º DS

## Pontos de Teste

1. **Detecção de quem buzou primeiro (buzzer)**
   Verifica se o sistema identifica corretamente qual jogador apertou o botão primeiro e ignora o segundo botão apertado logo em seguida.

2. **Atribuição de ponto conforme julgamento (Acertou/Errou)**
   Verifica se o ponto vai pro jogador certo dependendo de qual botão o mestre do jogo aperta (ACERTOU → ponto pra quem buzou; ERROU → ponto pro adversário).

3. **Regra da Pergunta de Ouro**
   Verifica se a 5ª pergunta respondida por um jogador é tratada como "de ouro", e se o acerto dela gera vitória instantânea mesmo com placar desfavorável.

4. **Condição de vitória por pontuação (5 pontos)**
   Verifica se o jogo declara vencedor corretamente ao atingir a meta de pontos, e se trava o jogo (não aceita mais jogadas) após a vitória.

5. **Sincronização entre placar e fita de LED**
   Verifica se a quantidade de LEDs acesos (verde para J1, azul para J2) reflete corretamente a pontuação de cada jogador em tempo real.

## Casos de Teste

| Ponto de Teste | ID | Pré-condição | Ação (entrada) | Resultado esperado |
|---|---|---|---|---|
| **1. Buzzer** | CT01 | Jogo na tela de espera | J1 aperta o buzzer (pino 6) | LCD exibe "J1 apertou!" com seta pra esquerda |
| | CT02 | Jogo na tela de espera | J2 aperta o buzzer (pino 7) | LCD exibe "J2 apertou!" com seta pra direita |
| | CT03 | J1 já apertou o buzzer (estado travado) | J2 aperta o buzzer logo em seguida | Sistema ignora o toque de J2; continua mostrando "J1 apertou!" |
| **2. Julgamento** | CT04 | J1 buzou e está aguardando julgamento | Mestre aperta o botão ACERTOU (pino 8) | Ponto adicionado a J1; volta à tela de espera |
| | CT05 | J1 buzou e está aguardando julgamento | Mestre aperta o botão ERROU (pino 9) | Ponto adicionado a J2 (adversário); volta à tela de espera |
| | CT06 | Alguém buzou e está aguardando julgamento | Mestre aperta ACERTOU e ERROU ao mesmo tempo | Pergunta é pulada; nenhum jogador pontua |
| **3. Pergunta de Ouro** | CT07 | J1 já respondeu 4 perguntas (perguntasRespondidas[0] = 4) | J1 buza e mestre confirma ACERTOU | LCD exibe "PERGUNTA DE OURO!"; LED amarelo do meio pisca até o julgamento; após confirmar, J1 é declarado vencedor imediatamente, mesmo com placar menor que J2 |
| | CT08 | J1 está na sua 5ª pergunta (de ouro) | Mestre aperta ERROU | Ponto normal vai para J2; LED amarelo para de piscar; jogo continua (sem vitória automática) |
| **4. Vitória por pontuação** | CT09 | J1 está com 4 pontos | J1 buza, mestre confirma ACERTOU | J1 atinge 5 pontos; LCD exibe "J1 venceu!"; jogo entra em estado travado; fita de LED inicia animação de vitória |
| | CT10 | Jogo já declarou um vencedor (estado JOGO_ACABOU) | Qualquer botão é pressionado | Nenhuma ação ocorre; LCD e fita continuam na animação de vencedor |
| **5. Fita de LED** | CT11 | Placar zerado (J1: 0, J2: 0) | Início do jogo | Todos os 13 LEDs apagados |
| | CT12 | J1 marca o 1º ponto | Confirmação de acerto pra J1 | 1 LED verde acende na fita (posição 0) |
| | CT13 | J1: 3 pontos, J2: 2 pontos | Estado atual do placar | 3 LEDs verdes acesos (posições 0-2) e 2 LEDs azuis acesos (posições 12 e 11) |

## Sketches de teste isolado

Além dos casos de teste da lógica do jogo, foram usados sketches isolados para validar cada componente de hardware antes da integração:

- `teste_lcd/teste_lcd.ino` — confirma que o display LCD está recebendo e exibindo texto corretamente.
- `teste_buzzer/teste_buzzer.ino` — identifica se o buzzer é do tipo ativo ou passivo e confirma a fiação do módulo de som.
- `teste_fita_led/teste_fita_led.ino` — confirma que a fita de LED WS2812B está endereçando e acendendo cada LED individualmente na cor correta.
