# 🔌 AlertaSubsolo — Firmware (Arduino)

Código da maquete física do **AlertaSubsolo** (Global Solution 2026.2 · FIAP · Grupo MIXTAPE).
Placa final: **Arduino Uno R4 WiFi**. O **Uno R3** foi usado para desenvolvimento e testes.

## O que o programa faz

Mede a água no estacionamento e no reservatório, decide a situação **sozinho** (sem internet) e reage: alarme, fita de LED de evacuação, bombas e envio das leituras para a nuvem (Supabase).

| Situação | Quando | Ação |
|---|---|---|
| **NORMAL** | tudo calmo | só mede; rota de fuga **verde** |
| **ALERTA** | ≥ 1 cm no estacionamento (água na roda do carro 1:43), reservatório subindo ou chuva | alarme de atenção, bombas trabalhando |
| **CRÍTICO** | ≥ 2 cm no estacionamento (enchente) | alarme forte, rota de fuga **vermelha** (evacuação) |

**Papel de cada sensor:**
- **Ultrassônico do reservatório** → liga a **bomba do reservatório** (principal) quando a água sobe.
- **Sensor de chuva** → liga as **duas bombas**.
- **Ultrassônico do estacionamento** → detecta **enchente** e comanda o **LED de evacuação** (só acima de 2 cm) e o alarme.

## Ligação dos pinos (Uno R4 WiFi)

| Pino | Função | Pino | Função |
|---|---|---|---|
| D2 | Trig (comum aos 2 HC‑SR04) | D11 / D12 | OLED SCL / SDA (I2C por software) |
| D3 | Echo reservatório | D13 | Sensor de chuva |
| D4 | Echo estacionamento | A0 | Umidade do solo |
| D6 | Buzzer | A1 / A2 | LDR 1 / LDR 2 |
| D7 | DHT11 | A3 | Sensor de toque |
| D8 | Relé 1 — bomba do reservatório (via **BC547**) | A4 / A5 | RTC DS3231 (I2C de hardware) |
| D9 | Relé 2 — segunda bomba | D10 | Fita NeoPixel (20 LEDs) |

## Bibliotecas

Instale pela Arduino IDE: `RTClib`, `U8g2`, `DHT sensor library`, `Adafruit NeoPixel`.
A `WiFiS3` já vem com o board **Arduino UNO R4 Boards**.

## Como gravar

1. Selecione a placa **Arduino UNO R4 WiFi** e a porta.
2. Em `WIFI_NOME` / `WIFI_SENHA`, coloque a **sua rede** (2,4 GHz) — só no seu computador.
3. **Upload** e abra o Monitor Serial (9600).
4. Para testar na bancada sem Wi‑Fi, deixe `USAR_NUVEM = false`.

## Ajustes (ficam no topo do arquivo)

- **Seção 1** — alturas do sensor (medido: 11 cm com a caixa vazia) e `AJUSTE` fino.
- **Seção 2** — limites de decisão: estacionamento **1 cm** (alerta) / **2 cm** (crítico); solo capacitivo **573 seco / 236 molhado**.
- **Seção 3** — chaves liga/desliga: polaridade dos relés, nuvem, cor da fita.

> **OLED:** o display usa I2C por **software** em modo **buffer cheio** (`..._F_SW_I2C`), que é estável. Se a placa reiniciar sozinha ou travar, o problema costuma ser **energia** — alimente a fita de LED e as bombas por uma **fonte 5V externa**, com o GND junto ao do Arduino.
>
> **Relé D8:** passa por um transistor **BC547**. Se a bomba ligar ao contrário, troque só a linha `RELE1_LIGA_COM_LOW`.

## Segurança

Antes de subir no GitHub, **troque o nome e a senha do Wi‑Fi por placeholders**. A chave do Supabase no arquivo é a **pública** (anon) e pode ficar; **nunca** suba a chave *secret/service_role*.
