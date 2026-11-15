# AlertaSubsolo — Sistema de Alagamento Inteligente

**Simulação no Wokwi — Arduino Uno R3**

Grupo **MIXTAPE** · Global Solution 2026.2 · FIAP · Engenharia de Software
Disciplina: **Edge Computing & Computer Systems**

---

## Integrantes

| Integrante | RM | Links |
|---|---|---|
| Gabriel Souza Bore de Carvalho | RM576428 | [GitHub](https://github.com/mixtapegbl) · [LinkedIn](https://www.linkedin.com/in/gabriel-souza-9a185a2b5/) · [YouTube](https://www.youtube.com/@mixtape-s39) |
| Rafael de Oliveira Santos Zuccon | RM575995 | [GitHub](https://github.com/rafazuccon) · [Site](https://www.stockwave.com.br/) |

**Repositório:** https://github.com/ZucconDEV/Global-Solutions---MixTape

---

## 1. O problema

Garagens, estacionamentos e passagens subterrâneas alagam em poucos minutos durante chuvas fortes. Quem está lá dentro costuma ser o último a saber, porque justamente nesses lugares o sinal de celular não chega. Quando o aviso vem de fora, já é tarde: o carro está submerso ou a pessoa está no meio da água.

O AlertaSubsolo resolve isso tirando a decisão da nuvem e colocando-a no próprio local. O dispositivo mede, decide e age sozinho, sem depender de internet. É esse o conceito de **computação de borda**: processar o dado onde ele nasce.

---

## 2. O que o sistema faz

Ele monitora dois pontos ao mesmo tempo:

- **O subsolo** (a garagem): quanta água já está no piso.
- **O reservatório** (a garrafa): quanta água foi captada da chuva.

A partir disso ele classifica a situação em três níveis e responde de forma diferente em cada um:

| Situação | O que significa | Como o sistema reage |
|---|---|---|
| **NORMAL** | Nada fora do comum | Só mede e informa no display e no Serial |
| **ALERTA** | Água acima do normal | Bipe curto e bomba de dreno ligada |
| **CRÍTICO** | Risco a pessoas | Alarme grave, fita de LED vermelha e bomba pulsando |

---

## 3. A ideia central do código

O sensor ultrassônico mede **distância até a água**. Mas quem pensa no problema pensa em **altura de água**. Por isso o programa faz a conversão uma única vez, logo depois de ler o sensor:

```
altura da água = altura do sensor − distância lida + ajuste
```

Daí para a frente, o programa inteiro trabalha em centímetros de água. Todos os limites de decisão estão escritos nessa unidade, que é a que você enxerga olhando a maquete.

Isso tem uma consequência prática importante: **se a maquete mudar de tamanho, basta alterar a Seção 1 do código.** Nenhuma outra linha precisa ser tocada.

---

## 4. Medidas da maquete (Seção 1 do código)

| Constante | Valor | O que é |
|---|---|---|
| `SUBSOLO_ALTURA_DO_SENSOR` | 22,0 cm | Do piso do subsolo até o sensor, que fica no teto |
| `SUBSOLO_ALTURA_DO_TETO` | 11,5 cm | Pé-direito do subsolo, usado para calcular o % alagado |
| `SUBSOLO_AJUSTE` | 0,0 cm | Correção fina se o sensor ler a mais ou a menos |
| `GARRAFA_ALTURA_DO_SENSOR` | 25,0 cm | Do fundo da garrafa até o sensor, na tampa |
| `GARRAFA_ALTURA_MAXIMA` | 13,0 cm | Altura de água que enche a garrafa |
| `GARRAFA_VOLUME_MAXIMO` | 500 mL | Volume correspondente a essa altura |
| `BOMBA_ML_POR_SEGUNDO` | 14 mL/s | Vazão medida na bancada: 100 mL em 7 segundos |

---

## 5. Limites de decisão (Seção 2 do código)

Cada número tem um motivo. Número sem justificativa é chute; número com justificativa é projeto.

| Limite | Valor | Por quê |
|---|---|---|
| Subsolo em alerta | 1,5 cm | A água passa da borda do pneu e molha motores baixos |
| Subsolo em crítico | 2,5 cm | A água chega nas tomadas rentes ao chão: risco elétrico |
| Garrafa em alerta | 2,0 cm | Liga a bomba cedo: esvaziar devagar é melhor que correr depois |
| Garrafa em crítico | 8,0 cm | Passou da metade; hora de limpar a mangueira antes de encher |

---

## 6. O diagnóstico da bomba



A bomba não é mágica. Se ela está **ligada** e mesmo assim a água **continua subindo**, alguma coisa está errada: ou a mangueira entupiu, ou está entrando mais água do que ela consegue tirar.

Então o programa vigia a tendência. A cada 5 segundos ele compara o nível de agora com o nível de antes:

- **Desceu ou ficou parado** → a bomba está vencendo, continua ligada direto.
- **Continuou subindo** → entra em modo pulso, ligando e desligando a cada 5 segundos para soltar a sujeira presa na mangueira. O motivo aparece escrito no Serial Monitor.

Acima de 8 cm o pulso entra de qualquer jeito, por precaução.

O ponto que isso demonstra: o sistema não apenas liga um motor quando passa de um número. Ele **verifica se a própria ação dele está funcionando** e muda de estratégia quando não está.

---

## 7. Tempo estimado de esvaziamento

Com a vazão medida de 14 mL/s, o Serial mostra quanto falta para esvaziar:

| Água na garrafa | Volume | Bomba contínua | Bomba pulsando |
|---|---|---|---|
| 2 cm | 76 mL | 5 s | 10 s |
| 5 cm | 192 mL | 13 s | 26 s |
| 8 cm | 307 mL | 21 s | 42 s |
| 11,7 cm | 450 mL | 32 s | 64 s |
| 13 cm | 500 mL | 35 s | 70 s |

Pulsando, a bomba trabalha metade do tempo, então demora o dobro.

---

## 8. Ligação dos pinos

| Pino | Componente | Função |
|---|---|---|
| D2 | HC-SR04 (garrafa) | Trig — disparo |
| D3 | HC-SR04 (garrafa) | Echo — retorno |
| D4 | HC-SR04 (subsolo) | Echo — retorno |
| D5 | Botão azul | Sensor de chuva (na maquete: sensor de gotas) |
| D6 | Buzzer | Alarme sonoro |
| D7 | DHT22 | Temperatura e umidade do ar |
| D8 | Módulo de relé 1 | Bomba de dreno |
| D9 | Módulo de relé 2 | Bomba de captação de chuva |
| D10 | Fita WS2812B | Guia de evacuação |
| D11 | HC-SR04 (subsolo) | Trig — disparo |
| A0 | Potenciômetro | Umidade do solo (na maquete: sensor capacitivo) |
| A1 / A2 | LDR 1 e 2 | Luminosidade |
| A3 | Botão verde | Toque (trava e modo manual) |
| A4 / A5 | Display OLED e relógio | Barramento I2C compartilhado |

**Cada sensor ultrassônico tem o seu próprio pino de disparo.** Se os dois dividissem o mesmo pino, disparariam juntos e o eco de um poderia entrar no outro, bagunçando a medida.

---

## 9. O circuito das bombas

Os dois relés são módulos de verdade, iguais aos da maquete. O circuito da bomba tem **fonte de energia própria, separada da do Arduino**:

```
Fonte externa → COM do relé → NO do relé → resistor → bomba → GND da fonte
```

O Arduino só comanda a bobina do relé pelo pino `IN`. A corrente do motor nunca passa pelos pinos da placa.

Isso não é detalhe estético. Um pino do Arduino entrega poucos miliampères; a bobina de um relé e um motor puxam muito mais. Ligar o motor direto no pino danifica a placa — e foi exatamente isso que aconteceu com o relé do pino D9 na nossa primeira montagem física.

O Wokwi não tem peça de motor DC nem de bomba d'água, então a bomba é representada por um LED no circuito chaveado: **aceso significa bombeando**.

---

## 10. Trocas entre a simulação e a maquete física

| Na maquete | No Wokwi | Por que serve |
|---|---|---|
| Sensor de gotas | Botão azul | Entrega o mesmo sinal digital |
| Sensor de solo capacitivo | Potenciômetro | Mesma leitura analógica de 0 a 1023 |
| Mini bomba submersa 5V | LED no circuito do relé | Mostra quando o relé fecha |
| Relógio DS3231 | DS1307 | Mesma biblioteca RTClib |
| Sensor DHT11 | DHT22 | Mesma biblioteca DHT |
| Arduino Uno R4 WiFi | Arduino Uno R3 | Exigência da disciplina |

A maquete física usa os componentes reais. A simulação prova a lógica.

---

## 11. Por que o código tem cuidados de memória

O Arduino Uno tem apenas **2 KB de RAM**. É pouquíssimo, e a primeira versão do programa não cabia na placa. Três mudanças resolveram:

1. **Display em modo página.** O OLED desenha a tela em pedaços de 128 bytes, em vez de guardar a tela inteira (1024 bytes) na memória. É o `_1_` no nome do objeto, em vez de `_F_`.

2. **Nenhuma classe `String`.** String parece fácil, mas vai pedindo memória enquanto o programa roda e acaba travando o Arduino. No lugar dela, o código usa `tela.print()` e `Serial.print()`, que escrevem direto sem guardar nada.

3. **Todo texto fixo dentro de `F("...")`.** Isso manda o texto ficar na memória de programa (32 KB, com folga) em vez da RAM (2 KB, que é o gargalo).


---

## 12. Como rodar no Wokwi

1. Entre em `wokwi.com`, faça login e crie um projeto novo de **Arduino Uno**.
2. Abra a aba `diagram.json`, apague o conteúdo e cole o `diagram.json` deste pacote.
3. Abra a aba `sketch.ino`, apague o conteúdo e cole o `sketch.ino` deste pacote.
4. Se alguma biblioteca faltar, use o **Library Manager** e adicione as de `libraries.txt`.
5. Clique em play. A animação de abertura deve aparecer e o Serial Monitor começar a imprimir uma linha por segundo.
6. Salve e use **Share → Copy link** para gerar o link público.

Os testes de cada situação estão em `COMO_TESTAR.md`.

---

## 13. Limitações conhecidas

- A simulação não envia dados para a nuvem. Essa parte roda na maquete física, com o Arduino Uno R4 WiFi.
- O Wokwi não simula motor nem água, então a bomba aparece como LED.
- O relógio DS1307 começa com a data e a hora da compilação, não com a hora real.
- Os valores de umidade do solo na simulação vêm de um potenciômetro, então não representam uma curva real de encharcamento.
- A detecção de chuva é binária (chove ou não chove), sem medir intensidade.

---

## 14. Estrutura deste pacote

```
sistema de alagamento inteligente wolki -GS/
├── sketch.ino        Programa do Arduino, comentado linha a linha
├── diagram.json      Circuito do Wokwi
├── libraries.txt     Bibliotecas usadas
├── README.md         Esta documentação
└── COMO_TESTAR.md    Roteiro de testes e de gravação do vídeo
```
