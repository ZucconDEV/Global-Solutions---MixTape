# Como testar e como gravar o vídeo

Grupo MIXTAPE · AlertaSubsolo · Global Solution 2026.2 — FIAP

---

## Antes de começar

Com a simulação rodando, você muda os valores **clicando em cima da peça**:

- **Sensor ultrassônico:** aparece um slider de distância.
- **Botão azul (CHUVA):** segure pressionado para simular chuva.
- **Botão verde (TOQUE):** toque rápido ou segure por 2 segundos.
- **DHT22:** clique para alterar temperatura e umidade.
- **Potenciômetro:** gire para alterar a umidade do solo.

Lembre que o programa usa a **média das últimas 5 leituras**. Depois de mexer no slider, espere uns 6 segundos para o valor estabilizar. Isso não é atraso do simulador, é a suavização funcionando.

---
Edge Computing (até 5 minutos)
## Tabela de testes

O sensor da garrafa está a 25 cm do fundo e o do subsolo a 22 cm do piso. Por isso o slider mostra a **distância**, e o display mostra a **altura da água**.

| # | O que testar | Onde mexer | Valor | O que deve acontecer |
|---|---|---|---|---|
| 1 | Tudo normal | ultra1 / ultra2 | 24 cm / 22 cm | Display em NORMAL, fita apagada, sem som |
| 2 | Garrafa em alerta | ultra1 | 23 cm | Display mostra ~2 cm e 76 mL, bipe curto, relé 1 fecha, LED vermelho aceso contínuo, marcador `B1` |
| 3 | Garrafa em crítico | ultra1 | 17 cm | Display ~8 cm e 307 mL, som grave, LED vermelho piscando a cada 5 s, marcador muda para `P1` |
| 4 | Garrafa cheia | ultra1 | 12 cm | Display ~13 cm e 500 mL |
| 5 | Subsolo em alerta | ultra2 | 20 cm | Display ~1,5 cm, bipe de aviso, fita apagada (só acende no crítico) |
| 6 | Subsolo em crítico | ultra2 | 19 cm | Display ~2,5 cm e o % alagado, som mais agudo, fita em onda vermelha, bomba de dreno liga |
| 7 | Leitura inválida | ultra2 | 80 cm | Display mostra `ERRO` e `ERR` no canto; **nenhuma bomba liga** |
| 8 | Chuva | botão azul | segurar | `CHUVA` no display, relé 2 fecha, LED amarelo acende |
| 9 | Trava de segurança | botão azul segurado + ultra1 em 17 cm | — | O LED amarelo apaga sozinho: garrafa cheia não pode encher mais |
| 10 | Trava de tempo | botão verde | toque rápido | O número no canto muda entre 5s, 10s e 20s |
| 11 | Modo manual | botão verde | segurar 2 s, três vezes | 1ª vez acende o amarelo, 2ª acende amarelo e vermelho, 3ª apaga os dois |

---

## O teste mais importante: o diagnóstico da bomba


1. Coloque o `ultra1` em **23 cm**. A bomba liga contínua e o Serial mostra `Dreno: LIGADA`.
2. Agora vá **baixando o slider devagar**, de 23 para 22, 21, 20. Isso simula a água subindo mesmo com a bomba trabalhando.
3. Em cerca de 5 segundos o Serial escreve:

```
[DIAGNOSTICO] A agua subiu com a bomba ligada. Pulsando para desentupir.
```

4. O marcador no display muda de `B1` para `P1` e o LED passa a piscar.

---
