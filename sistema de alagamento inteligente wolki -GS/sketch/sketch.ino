/*
  ================================================================
   ALERTASUBSOLO - Sistema de Alagamento Inteligente
   Simulacao no Wokwi com Arduino Uno R3
  ================================================================

   Grupo: MIXTAPE
   Global Solution 2026.2 - FIAP - Engenharia de Software
   Disciplina: Edge Computing & Computer Systems

   ----------------------------------------------------------------
   INTEGRANTES
   ----------------------------------------------------------------
   Gabriel Souza Bore de Carvalho - RM576428
     GitHub:   https://github.com/mixtapegbl
     LinkedIn: https://www.linkedin.com/in/gabriel-souza-9a185a2b5/
     YouTube:  https://www.youtube.com/@mixtape-s39

   Rafael de Oliveira Santos Zuccon - RM575995
     GitHub: https://github.com/rafazuccon
     Site:   https://www.stockwave.com.br/

   Repositorio do projeto:
     https://github.com/ZucconDEV/Global-Solutions---MixTape

  ================================================================
   O QUE ESTE PROGRAMA FAZ
  ================================================================

   Garagens e subsolos alagam em poucos minutos, e quase sempre
   em lugares onde o sinal de celular nao chega. Este dispositivo
   mede quanta agua existe no subsolo e no reservatorio, decide
   sozinho em que situacao esta e reage ali mesmo, sem internet:

     - toca um alarme sonoro;
     - acende uma fita de LED que indica a saida;
     - liga as bombas de agua atraves de reles.

   Sao tres situacoes possiveis:

     NORMAL   nada ligado, so medindo e informando.
     ALERTA   bipe curto, bomba de dreno ligada.
     CRITICO  alarme grave, fita vermelha, bomba pulsando.

  ================================================================
   A IDEIA MAIS IMPORTANTE DO CODIGO
  ================================================================

   O sensor ultrassonico mede DISTANCIA ate a agua. Mas quem
   pensa no problema (e quem assiste a apresentacao) pensa em
   ALTURA DE AGUA. Por isso a conversao acontece uma unica vez,
   logo depois da leitura:

       altura da agua = altura do sensor - distancia lida + ajuste

   Dai para a frente o programa inteiro trabalha em centimetros
   de agua. E por isso que todos os limites da secao 2 estao em
   cm de agua, que e como voce enxerga olhando a maquete.

  ================================================================
   DIFERENCAS ENTRE A SIMULACAO E A MAQUETE FISICA
  ================================================================

   Algumas pecas nao existem no simulador e foram trocadas por
   equivalentes que dao exatamente o mesmo tipo de sinal:

     Sensor de gotas   -> botao azul (apertado = esta chovendo)
     Sensor de solo    -> potenciometro (mesma leitura analogica)
     Bomba de agua     -> LED ligado pelo rele (aceso = bombeando)
     Relogio DS3231    -> DS1307 (mesma biblioteca)
     Sensor DHT11      -> DHT22 (mesma biblioteca)

   Os dois reles sao modulos de verdade, iguais aos da maquete.
   As bombas tem fonte de energia PROPRIA, separada do Arduino:
   o Arduino so manda o sinal, a corrente do motor nunca passa
   pelos pinos dele.

  ================================================================
   POR QUE O CODIGO TEM ALGUNS CUIDADOS ESTRANHOS
  ================================================================

   O Arduino Uno tem apenas 2 KB de memoria RAM. E pouquissimo.
   Por isso o programa usa tres truques de economia:

     1. O display trabalha em "modo pagina". Ele desenha a tela
        em pedacos de 128 bytes em vez de guardar a tela inteira
        (1024 bytes) na memoria. E o "_1_" no nome do objeto.

     2. Nao existe nenhum texto montado com a classe String.
        String parece facil, mas vai pedindo memoria enquanto o
        programa roda e acaba travando o Arduino. No lugar dela
        usamos oled.print() e Serial.print(), que escrevem
        direto, sem guardar nada.

     3. Todo texto fixo esta dentro de F("..."). Isso manda o
        texto ficar na memoria de programa (32 KB, sobra espaco)
        em vez da memoria RAM (2 KB, que e o gargalo).

   Nada disso muda o que o programa faz. Sao so cuidados para
   caber na placa.

  ================================================================
*/

#include <Wire.h>              // comunicacao I2C (display e relogio)
#include "RTClib.h"            // relogio de tempo real
#include <U8g2lib.h>           // display OLED
#include <DHT.h>               // sensor de temperatura e umidade
#include <Adafruit_NeoPixel.h> // fita de LED enderecavel


// ================================================================
// SECAO 1 - MEDIDAS BRUTAS DA MAQUETE
// Parede VERTICAL TERREO:        11,5 X 22,5 - 02un
// Parede FUNDO laterais TERREO : 11,5 X 15,5 - 02un
//-----------------------------------------------------------------
// Tetos/compensação: compensar o material utilizado aqui: 
// valor bruto: 15,5 x 22,5 
// Compensação: 4mm acrilico = 15,9 x 22,9 - 3 un
// ================================================================
// MEDIDAS BRUTAS ANDAR_1:
//
// Parede VERTICAL ANDAR_1:        11,5 X 22,5 - 02un
// Parede FUNDO laterais ANDAR_1 : 11,5 X 15,5 - 02un
// Teto na parte compensação
// ================================================================
// Meca com a regua e escreva abaixo. Se o sensor estiver lendo
// um pouco a mais ou a menos que o real, corrija no AJUSTE em vez
// de sair mexendo no resto do programa.

// ---------- SUBSOLO (a garagem) ----------

// Distancia do PISO do subsolo ate o sensor, que fica no teto.
float SUBSOLO_ALTURA_DO_SENSOR = 22.0;

// Distancia do piso ate o teto. Serve so para calcular
// quantos por cento do subsolo ja esta embaixo d'agua.
float SUBSOLO_ALTURA_DO_TETO = 11.5;

// Correcao fina do sensor do subsolo.
// Positivo aumenta a altura de agua calculada, negativo diminui.
float SUBSOLO_AJUSTE = 0.0;

// ---------- GARRAFA (o reservatorio) ----------

// Distancia do FUNDO da garrafa ate o sensor, que fica na tampa.
float GARRAFA_ALTURA_DO_SENSOR = 25.0;

// Quanto de agua cabe na garrafa, em altura e em volume.
// Usados para mostrar o volume em mL e avisar de transbordo.
float GARRAFA_ALTURA_MAXIMA = 13.0;   // cm
int   GARRAFA_VOLUME_MAXIMO = 500;    // mL

// Correcao fina do sensor da garrafa.
float GARRAFA_AJUSTE = 0.0;

// ---------- BOMBA (valor medido na bancada) ----------

// Mini bomba submersa de 5V: tirou 100 mL em 7 segundos.
// 100 dividido por 7 da 14,3 mL por segundo.
// Com isso o programa estima quanto tempo falta para esvaziar.
int BOMBA_ML_POR_SEGUNDO = 14;

// ---------- Faixa de leitura confiavel do ultrassonico ----------

// O sensor HC-SR04 nao consegue medir nada abaixo de 2 cm.
// E, na nossa maquete, qualquer leitura acima de 50 cm so pode
// ser eco perdido ou fio solto. Fora dessa faixa e ERRO.
float DISTANCIA_MINIMA = 2.0;
float DISTANCIA_MAXIMA = 50.0;


// ================================================================
// SECAO 2 - LIMITES DE DECISAO, EM CENTIMETROS DE AGUA
// ================================================================
// Cada limite abaixo tem um motivo escrito do lado. Isso importa:
// numero sem explicacao e chute, numero com explicacao e projeto.

// ---------- SUBSOLO ----------

// Com 1,5 cm a agua ja passa da borda do pneu e molha motores
// baixos. E hora de avisar, mas ainda nao de evacuar.
float SUBSOLO_LIMITE_ALERTA = 1.5;

// Com 2,5 cm a agua chega nas tomadas rentes ao chao e o risco
// deixa de ser so material e passa a ser eletrico. Evacuar.
float SUBSOLO_LIMITE_CRITICO = 2.5;

// ---------- GARRAFA ----------

// A bomba liga cedo, com 2 cm, porque esvaziar devagar e muito
// melhor do que esvaziar com pressa depois que encheu.
float GARRAFA_LIMITE_ALERTA = 2.0;

// Acima de 8 cm a garrafa ja passou da metade. A partir dai a
// bomba passa a pulsar, para limpar a mangueira antes que a
// agua chegue na boca da garrafa.
float GARRAFA_LIMITE_CRITICO = 8.0;

// ---------- Diagnostico da bomba (quando ela deve pulsar) ----------
//
// A bomba nao e magica. Se ela esta LIGADA e mesmo assim a agua
// continua subindo, alguma coisa esta errada: mangueira entupida,
// ou esta entrando mais agua do que a bomba consegue tirar.
//
// Nesse caso ligar e desligar em intervalos (pulsar) ajuda a
// soltar a sujeira presa na mangueira.
//
// Entao o programa vigia a tendencia: de tempos em tempos ele
// compara o nivel de agora com o nivel de antes.
//   - desceu ou ficou parado -> a bomba esta vencendo, segue direto
//   - continuou subindo      -> passa a pulsar

unsigned long TEMPO_ENTRE_COMPARACOES = 5000;  // compara a cada 5 s
float SUBIDA_QUE_CONTA = 0.3;                  // menos que isso e ruido
unsigned long TEMPO_DE_CADA_PULSO = 5000;      // liga 5 s, desliga 5 s

// ---------- Suavizacao das leituras ----------
//
// A agua balanca e a boia sobe e desce sozinha. Se o programa
// olhasse so a ultima leitura, o estado ficaria piscando entre
// NORMAL e ALERTA o tempo todo. Por isso ele guarda as ultimas
// 5 leituras e usa a media delas.
//
// Como le uma vez por segundo, 5 leituras significam reagir em
// cerca de 5 segundos. Rapido o bastante para uma enchente.

const byte QUANTAS_LEITURAS = 5;

// ---------- Sensor de umidade do solo ----------
//
// Na simulacao e um potenciometro, que entrega de 0 a 1023.
// NA MAQUETE FISICA os valores medidos foram 550 (seco) e 250 (molhado).

int SOLO_VALOR_SECO    = 1023;
int SOLO_VALOR_MOLHADO = 0;

// ---------- Previsao do tempo ----------
//
// Solo encharcado nao absorve mais nada: a proxima chuva escorre
// toda para o ponto mais baixo, que e justamente o subsolo.

byte  SOLO_ENCHARCADO = 75;     // em %
float AR_MUITO_UMIDO  = 80.0;   // em %
float TEMPERATURA_DE_CHUVA = 22.0;  // em graus

// ---------- Trava de tempo minimo das bombas ----------
//
// Sem isso a bomba ligaria e desligaria varias vezes por minuto
// toda vez que o nivel ficasse em cima do limite. Isso queima
// motor. A trava obriga a bomba a ficar ligada por um tempo
// minimo. O botao de toque troca entre 5, 10 e 20 segundos.

unsigned long OPCOES_DE_TRAVA[] = {5000, 10000, 20000};
const byte QUANTAS_OPCOES_DE_TRAVA = 3;


// ================================================================
// SECAO 3 - CHAVES LIGA / DESLIGA DE COMPORTAMENTO
// ================================================================
// Mude para true ou false e o programa muda de comportamento,
// sem precisar reescrever nada.

// A bomba de dreno tambem esvazia o subsolo alagado, nao so a garrafa.
bool BOMBA_TAMBEM_DRENA_SUBSOLO = true;

// A fita de LED so acende no CRITICO. Antes disso a agua ainda nao
// oferece risco, e mandar evacuar seria alarme falso.
// Coloque true se quiser que ela acenda tambem no ALERTA.
bool FITA_ACENDE_NO_ALERTA = false;

// Nao encher a garrafa se o sensor dela estiver com erro,
// porque nao da para saber se ela ja esta cheia.
bool SO_ENCHER_COM_SENSOR_OK = false;

// Como o rele liga:
//   false = o rele fecha quando o pino manda HIGH (padrao do Wokwi)
//   true  = o rele fecha quando o pino manda LOW  (maioria dos
//           modulos fisicos, inclusive os da nossa maquete)
//
// Se a bomba acender ao contrario do esperado, e so inverter
// estas duas linhas.
bool RELE1_LIGA_COM_LOW = false;
bool RELE2_LIGA_COM_LOW = false;

// ---------- Fita de LED ----------
#define QUANTOS_LEDS 30
const byte TAMANHO_DA_ONDA = 7;   // a cada 7 LEDs a onda se repete

// ---------- Codigos das situacoes ----------
// Guardar um numero gasta 1 byte. Guardar o texto "CRITICO"
// gastaria 8. Como a RAM e curta, guardamos o numero e so
// escrevemos o texto na hora de mostrar na tela.
const byte SITUACAO_NORMAL  = 0;
const byte SITUACAO_ALERTA  = 1;
const byte SITUACAO_CRITICO = 2;

// ---------- Codigos da previsao do tempo ----------
const byte PREVISAO_ESTAVEL       = 0;
const byte PREVISAO_CHUVA         = 1;
const byte PREVISAO_SOLO_SATURADO = 2;
const byte PREVISAO_SEM_DADOS     = 3;


// ================================================================
// SECAO 4 - LIGACAO DOS PINOS (Arduino Uno R3)
// ================================================================
// Cada sensor ultrassonico tem o SEU proprio pino de disparo.
// Se os dois dividissem o mesmo pino, eles disparariam juntos e o
// eco de um poderia entrar no ouvido do outro, bagunçando a medida.

const byte PINO_DISPARO_GARRAFA = 2;   // Trig do sensor da garrafa
const byte PINO_ECO_GARRAFA     = 3;   // Echo do sensor da garrafa
const byte PINO_DISPARO_SUBSOLO = 11;  // Trig do sensor do subsolo
const byte PINO_ECO_SUBSOLO     = 4;   // Echo do sensor do subsolo

const byte PINO_CHUVA   = 5;   // botao azul (na maquete: sensor de gotas)
const byte PINO_SOM     = 6;   // buzzer
const byte PINO_DHT     = 7;   // sensor de temperatura e umidade do ar
const byte PINO_RELE1   = 8;   // rele da bomba de dreno
const byte PINO_RELE2   = 9;   // rele da bomba de captacao de chuva
const byte PINO_FITA    = 10;  // fita de LED
const byte PINO_SOLO    = A0;  // potenciometro (na maquete: sensor de solo)
const byte PINO_LDR1    = A1;  // sensor de luz 1
const byte PINO_LDR2    = A2;  // sensor de luz 2
const byte PINO_TOQUE   = A3;  // botao verde (na maquete: sensor de toque)

// Os pinos A4 e A5 sao usados pelo display e pelo relogio.
// Os dois conversam pelo mesmo par de fios, que se chama I2C.

#define TIPO_DHT DHT22
DHT sensorDHT(PINO_DHT, TIPO_DHT);

RTC_DS1307 relogio;

// O "_1_" no meio do nome e o modo pagina, que economiza memoria.
U8G2_SSD1306_128X64_NONAME_1_HW_I2C tela(U8G2_R0, U8X8_PIN_NONE);

Adafruit_NeoPixel fita(QUANTOS_LEDS, PINO_FITA, NEO_GRB + NEO_KHZ800);


// ================================================================
// SECAO 5 - VARIAVEIS QUE GUARDAM O ESTADO DO SISTEMA
// ================================================================

// ---------- Marcadores de tempo ----------
// O programa nunca usa delay() para esperar, porque delay() para
// o Arduino inteiro. Em vez disso ele anota em que milissegundo
// cada coisa aconteceu e compara com o relogio interno (millis).

unsigned long momentoDaUltimaMedicao = 0;
unsigned long momentoDoUltimoPassoDaFita = 0;
unsigned long momentoQueARele1Ligou = 0;
unsigned long momentoQueARele2Ligou = 0;
unsigned long momentoQueAFitaAcendeu = 0;
unsigned long momentoDoUltimoPulso = 0;
unsigned long momentoDaUltimaComparacao = 0;
unsigned long momentoQueOBotaoFoiApertado = 0;

// ---------- Botao de toque ----------
unsigned long travaDasBombas = 10000;
byte opcaoDeTravaAtual = 1;
bool estadoAnteriorDoBotao = LOW;
bool acaoDeSegurarJaFeita = false;

// Modo manual, para testar as bombas na hora da apresentacao:
//   0 = desligado
//   1 = so a bomba de captacao (LED amarelo)
//   2 = captacao + dreno (amarelo e vermelho juntos)
byte modoManual = 0;

// ---------- Controle do pulso da bomba ----------
bool bombaEstaPulsando = false;
bool faseAtualDoPulso = true;
float nivelDaComparacaoAnterior = 0;

// ---------- Fita de LED ----------
byte passoDaOnda = 0;

// ---------- Memoria das ultimas leituras (para a media) ----------
float leiturasDaGarrafa[QUANTAS_LEITURAS];
byte quantasLeiturasDaGarrafa = 0;

float leiturasDoSubsolo[QUANTAS_LEITURAS];
byte quantasLeiturasDoSubsolo = 0;

// ---------- Valores medidos, ja convertidos ----------
float aguaNaGarrafa = 0.0;    // cm
float aguaNoSubsolo = 0.0;    // cm
int volumeNaGarrafa = 0;      // mL
byte porcentoAlagado = 0;     // % do subsolo embaixo d'agua

float temperatura = 0.0;
float umidadeDoAr = 0.0;
byte umidadeDoSolo = 0;
byte luzMedia = 0;
bool estaChovendo = false;

// ---------- O que esta ligado agora ----------
bool bombaDeDrenoLigada = false;
bool bombaDeCaptacaoLigada = false;
bool fitaAcesa = false;
bool fitaVermelha = false;

// ---------- Situacao geral e erros ----------
byte situacaoAtual = SITUACAO_NORMAL;
byte previsaoAtual = PREVISAO_ESTAVEL;

bool erroNoDHT = false;
bool erroNoSensorDaGarrafa = false;
bool erroNoSensorDoSubsolo = false;
bool relogioFuncionando = false;

// ---------- Resultado das comparacoes do ciclo ----------
bool garrafaEmAlerta = false;
bool garrafaEmCritico = false;
bool subsoloEmAlerta = false;
bool subsoloEmCritico = false;


// ================================================================
// SECAO 6 - FUNCOES AUXILIARES (as pecinhas que o resto usa)
// ================================================================

// ----------------------------------------------------------------
// Liga ou desliga um rele, respeitando a polaridade do modulo.
// ----------------------------------------------------------------
void mexerNoRele(byte pino, bool ligar, bool ligaComLow) {
  if (ligaComLow) {
    if (ligar) {
      digitalWrite(pino, LOW);
    } else {
      digitalWrite(pino, HIGH);
    }
  } else {
    if (ligar) {
      digitalWrite(pino, HIGH);
    } else {
      digitalWrite(pino, LOW);
    }
  }
}

// ----------------------------------------------------------------
// Dispara o sensor ultrassonico e devolve a distancia em cm.
// Devolve -1 quando a leitura nao serve.
//
// Como funciona: o pino de disparo manda um pulso bem curto, o
// sensor solta um som que a gente nao escuta, e o pino de eco
// fica em HIGH durante o tempo que o som levou para ir e voltar.
// Como o som anda 0,034 cm por microssegundo, basta multiplicar
// e dividir por 2 (porque o som fez o caminho de ida e de volta).
// ----------------------------------------------------------------
float medirDistancia(byte pinoDisparo, byte pinoEco) {
  digitalWrite(pinoDisparo, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoDisparo, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoDisparo, LOW);

  // Espera o eco por no maximo 25 mil microssegundos (25 ms).
  unsigned long tempoDoEco = pulseIn(pinoEco, HIGH, 25000UL);

  // Nenhum eco chegou dentro do prazo.
  if (tempoDoEco == 0) {
    return -1;
  }

  float distancia = (tempoDoEco * 0.034) / 2.0;

  // Leitura fora da faixa confiavel: tratamos como erro.
  if (distancia < DISTANCIA_MINIMA) {
    return -1;
  }
  if (distancia > DISTANCIA_MAXIMA) {
    return -1;
  }

  return distancia;
}

// ----------------------------------------------------------------
// Transforma distancia medida em altura de agua.
// Nunca devolve numero negativo: sem agua e zero, nao menos um.
// ----------------------------------------------------------------
float calcularAlturaDaAgua(float distancia, float alturaDoSensor, float ajuste) {
  float altura = (alturaDoSensor - distancia) + ajuste;

  if (altura < 0) {
    altura = 0;
  }

  return altura;
}

// ----------------------------------------------------------------
// Media das ultimas leituras da GARRAFA.
//
// A lista funciona como uma fila: a leitura nova entra na frente
// e todas as outras andam uma casa para tras. A mais velha cai
// fora. Depois somamos o que sobrou e dividimos pela quantidade.
// ----------------------------------------------------------------
float mediaDaGarrafa(float leituraNova) {
  for (byte i = QUANTAS_LEITURAS - 1; i > 0; i--) {
    leiturasDaGarrafa[i] = leiturasDaGarrafa[i - 1];
  }
  leiturasDaGarrafa[0] = leituraNova;

  if (quantasLeiturasDaGarrafa < QUANTAS_LEITURAS) {
    quantasLeiturasDaGarrafa++;
  }

  float soma = 0;
  for (byte i = 0; i < quantasLeiturasDaGarrafa; i++) {
    soma = soma + leiturasDaGarrafa[i];
  }

  return soma / quantasLeiturasDaGarrafa;
}

// ----------------------------------------------------------------
// Media das ultimas leituras do SUBSOLO. Mesma ideia da de cima.
// ----------------------------------------------------------------
float mediaDoSubsolo(float leituraNova) {
  for (byte i = QUANTAS_LEITURAS - 1; i > 0; i--) {
    leiturasDoSubsolo[i] = leiturasDoSubsolo[i - 1];
  }
  leiturasDoSubsolo[0] = leituraNova;

  if (quantasLeiturasDoSubsolo < QUANTAS_LEITURAS) {
    quantasLeiturasDoSubsolo++;
  }

  float soma = 0;
  for (byte i = 0; i < quantasLeiturasDoSubsolo; i++) {
    soma = soma + leiturasDoSubsolo[i];
  }

  return soma / quantasLeiturasDoSubsolo;
}

// ----------------------------------------------------------------
// Apaga a memoria das medias quando um sensor falha, para a media
// nao misturar valores velhos com os novos quando ele voltar.
// ----------------------------------------------------------------
void esquecerLeiturasDaGarrafa() {
  quantasLeiturasDaGarrafa = 0;
}

void esquecerLeiturasDoSubsolo() {
  quantasLeiturasDoSubsolo = 0;
}

// ----------------------------------------------------------------
// Toca tres bipes agudos quando alguma peca nao responde.
// ----------------------------------------------------------------
void tocarBipeDeErro() {
  for (byte i = 0; i < 3; i++) {
    tone(PINO_SOM, 2500, 120);
    delay(200);
  }
}

// ----------------------------------------------------------------
// Animacao de abertura: um predio enchendo de agua enquanto a
// barra carrega. Serve de enfeite e tambem de teste do display.
// ----------------------------------------------------------------
void animacaoDeAbertura() {
  for (byte porcento = 0; porcento <= 100; porcento = porcento + 4) {

    byte alturaDaAgua = (porcento * 30) / 100;  // agua dentro do predio
    byte tamanhoDaBarra = (porcento * 72) / 100;

    tela.firstPage();
    do {
      // Contorno do predio e as lajes dos andares
      tela.drawFrame(8, 16, 28, 36);
      tela.drawLine(8, 26, 35, 26);
      tela.drawLine(8, 36, 35, 36);

      // Agua subindo de baixo para cima
      if (alturaDaAgua > 0) {
        tela.drawBox(10, 50 - alturaDaAgua, 24, alturaDaAgua);
      }

      tela.setFont(u8g2_font_6x12_tr);
      tela.drawStr(44, 24, "MIXTAPE");

      tela.setFont(u8g2_font_5x7_tr);
      tela.drawStr(44, 36, "Carregando");
      tela.drawStr(44, 45, "sistema IoT");

      // Barra de progresso
      tela.drawFrame(44, 50, 76, 7);
      if (tamanhoDaBarra > 0) {
        tela.drawBox(46, 52, tamanhoDaBarra, 3);
      }
    } while (tela.nextPage());

    // Um tom que vai ficando mais agudo conforme carrega
    if (porcento % 20 == 0) {
      tone(PINO_SOM, 500 + (porcento * 7), 45);
    }

    delay(25);
  }

  tone(PINO_SOM, 1600, 150);
  delay(200);
}


// ================================================================
// SECAO 7 - SETUP (roda uma vez, quando a placa liga)
// ================================================================
void setup() {
  Serial.begin(9600);

  Serial.println(F("================================================"));
  Serial.println(F(" ALERTASUBSOLO - Grupo MIXTAPE"));
  Serial.println(F(" Global Solution 2026.2 - FIAP"));
  Serial.println(F(" Gabriel Souza RM576428 / Rafael Zuccon RM575995"));
  Serial.println(F("================================================"));

  // Sensores ultrassonicos
  pinMode(PINO_DISPARO_GARRAFA, OUTPUT);
  pinMode(PINO_ECO_GARRAFA, INPUT);
  pinMode(PINO_DISPARO_SUBSOLO, OUTPUT);
  pinMode(PINO_ECO_SUBSOLO, INPUT);

  // INPUT_PULLUP liga um resistor interno que segura o pino em HIGH.
  // Assim, sem ninguem apertando, o programa le HIGH = nao esta
  // chovendo. Sem isso o pino ficaria solto, lendo lixo.
  pinMode(PINO_CHUVA, INPUT_PULLUP);

  // Este botao tem um resistor por fora puxando para baixo,
  // entao solto ele le LOW e apertado le HIGH.
  pinMode(PINO_TOQUE, INPUT);

  pinMode(PINO_SOM, OUTPUT);

  // Deixa os dois reles desligados antes de qualquer outra coisa,
  // para nenhuma bomba dar um tranco no momento em que liga a placa.
  pinMode(PINO_RELE1, OUTPUT);
  mexerNoRele(PINO_RELE1, false, RELE1_LIGA_COM_LOW);

  pinMode(PINO_RELE2, OUTPUT);
  mexerNoRele(PINO_RELE2, false, RELE2_LIGA_COM_LOW);

  sensorDHT.begin();

  fita.begin();
  fita.setBrightness(120);
  fita.clear();
  fita.show();

  tela.begin();
  animacaoDeAbertura();

  if (relogio.begin()) {
    relogioFuncionando = true;

    // Se o relogio nunca foi acertado, acerta com a data e a hora
    // em que o programa foi compilado.
    if (!relogio.isrunning()) {
      relogio.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  } else {
    relogioFuncionando = false;
    Serial.println(F("[ERRO] O relogio nao respondeu. A hora nao sera mostrada."));
    tocarBipeDeErro();
  }

  Serial.println(F("Sistema funcionando. Uma leitura por segundo."));
  Serial.println();
}


// ================================================================
// SECAO 8 - LOOP (roda sem parar, milhares de vezes por segundo)
// ================================================================
//
// O loop em si quase nao faz nada: ele so olha o relogio e decide
// de quem e a vez. Tudo que e demorado esta dentro de funcoes
// separadas, e cada uma tem o seu proprio ritmo:
//
//   botao  -> o tempo todo, para nao perder nenhum toque
//   medir  -> uma vez por segundo
//   fita   -> a cada 85 ms, que e a velocidade da animacao
//
void loop() {
  unsigned long agora = millis();

  lerBotaoDeToque(agora);

  if (agora - momentoDaUltimaMedicao >= 1000) {
    momentoDaUltimaMedicao = agora;
    medirEDecidir(agora);
  }

  animarFita(agora);
}


// ================================================================
// SECAO 9 - BOTAO DE TOQUE
// ================================================================
//
//   Toque rapido  -> troca a trava das bombas (5 s, 10 s, 20 s)
//   Segurar 2 s   -> avanca o modo manual de teste:
//                    desligado -> captacao -> captacao + dreno -> desligado
//
// O modo manual existe para a apresentacao: ele liga as bombas
// na marra, sem olhar os sensores, para mostrar que funcionam.
//
void lerBotaoDeToque(unsigned long agora) {
  bool botaoAgora = digitalRead(PINO_TOQUE);

  // Acabou de apertar: anota a hora e zera o controle do "segurar".
  if (botaoAgora == HIGH && estadoAnteriorDoBotao == LOW) {
    momentoQueOBotaoFoiApertado = agora;
    acaoDeSegurarJaFeita = false;
  }

  // Continua apertado: ja passou de 2 segundos?
  else if (botaoAgora == HIGH && !acaoDeSegurarJaFeita) {
    if (agora - momentoQueOBotaoFoiApertado >= 2000) {

      modoManual = modoManual + 1;
      if (modoManual > 2) {
        modoManual = 0;
      }

      acaoDeSegurarJaFeita = true;

      Serial.print(F("[BOTAO] Modo manual "));
      Serial.print(modoManual);

      if (modoManual == 1) {
        tone(PINO_SOM, 1500, 180);
        Serial.println(F(" - bomba de CAPTACAO ligada (LED amarelo)"));
      } else if (modoManual == 2) {
        tone(PINO_SOM, 1900, 180);
        Serial.println(F(" - CAPTACAO e DRENO ligadas (amarelo e vermelho)"));
      } else {
        tone(PINO_SOM, 700, 220);
        Serial.println(F(" - as duas bombas desligadas"));
      }
    }
  }

  // Soltou o botao sem ter segurado: foi um toque rapido.
  else if (botaoAgora == LOW && estadoAnteriorDoBotao == HIGH) {
    if (!acaoDeSegurarJaFeita && (agora - momentoQueOBotaoFoiApertado >= 50)) {

      opcaoDeTravaAtual = opcaoDeTravaAtual + 1;
      if (opcaoDeTravaAtual >= QUANTAS_OPCOES_DE_TRAVA) {
        opcaoDeTravaAtual = 0;
      }

      travaDasBombas = OPCOES_DE_TRAVA[opcaoDeTravaAtual];

      tone(PINO_SOM, 1400, 80);
      Serial.print(F("[BOTAO] Trava das bombas: "));
      Serial.print(travaDasBombas / 1000);
      Serial.println(F(" segundos"));
    }
  }

  estadoAnteriorDoBotao = botaoAgora;
}


// ================================================================
// SECAO 10 - MEDIR, DECIDIR E AGIR (uma vez por segundo)
// ================================================================
void medirEDecidir(unsigned long agora) {

  // ------------------------------------------------------------
  // PARTE 1: ler todos os sensores
  // ------------------------------------------------------------

  erroNoSensorDaGarrafa = false;
  erroNoSensorDoSubsolo = false;
  erroNoDHT = false;

  // --- Sensor da garrafa ---
  float distanciaGarrafa = medirDistancia(PINO_DISPARO_GARRAFA, PINO_ECO_GARRAFA);

  if (distanciaGarrafa < 0) {
    erroNoSensorDaGarrafa = true;
    aguaNaGarrafa = 0;
    volumeNaGarrafa = 0;
    esquecerLeiturasDaGarrafa();
  } else {
    float distanciaSuave = mediaDaGarrafa(distanciaGarrafa);

    aguaNaGarrafa = calcularAlturaDaAgua(distanciaSuave,
                                         GARRAFA_ALTURA_DO_SENSOR,
                                         GARRAFA_AJUSTE);

    // Regra de tres: se 13 cm sao 500 mL, quanto sao X cm?
    volumeNaGarrafa = (aguaNaGarrafa * GARRAFA_VOLUME_MAXIMO) / GARRAFA_ALTURA_MAXIMA;
  }

  // Pequena pausa entre um sensor e outro, para o eco do primeiro
  // sumir antes do segundo disparar.
  delay(30);

  // --- Sensor do subsolo ---
  float distanciaSubsolo = medirDistancia(PINO_DISPARO_SUBSOLO, PINO_ECO_SUBSOLO);

  if (distanciaSubsolo < 0) {
    erroNoSensorDoSubsolo = true;
    aguaNoSubsolo = 0;
    porcentoAlagado = 0;
    esquecerLeiturasDoSubsolo();
  } else {
    float distanciaSuave = mediaDoSubsolo(distanciaSubsolo);

    aguaNoSubsolo = calcularAlturaDaAgua(distanciaSuave,
                                         SUBSOLO_ALTURA_DO_SENSOR,
                                         SUBSOLO_AJUSTE);

    float conta = (aguaNoSubsolo * 100.0) / SUBSOLO_ALTURA_DO_TETO;
    if (conta > 100) {
      conta = 100;
    }
    porcentoAlagado = conta;
  }

  // --- Sensor de chuva ---
  // O botao esta ligado ao GND, entao apertado ele puxa o pino
  // para LOW. LOW significa "esta chovendo".
  estaChovendo = (digitalRead(PINO_CHUVA) == LOW);

  // --- Umidade do solo ---
  int leituraDoSolo = analogRead(PINO_SOLO);
  umidadeDoSolo = map(leituraDoSolo, SOLO_VALOR_SECO, SOLO_VALOR_MOLHADO, 0, 100);
  umidadeDoSolo = constrain(umidadeDoSolo, 0, 100);

  // --- Temperatura e umidade do ar ---
  float temp = sensorDHT.readTemperature();
  float umid = sensorDHT.readHumidity();

  // isnan quer dizer "is not a number": o sensor nao respondeu.
  if (isnan(temp) || isnan(umid)) {
    erroNoDHT = true;
    temperatura = 0;
    umidadeDoAr = 0;
  } else {
    temperatura = temp;
    umidadeDoAr = umid;
  }

  // --- Luz (media dos dois sensores) ---
  int somaDosLDRs = analogRead(PINO_LDR1) + analogRead(PINO_LDR2);
  luzMedia = map(somaDosLDRs / 2, 0, 1023, 0, 100);
  luzMedia = constrain(luzMedia, 0, 100);

  // ------------------------------------------------------------
  // PARTE 2: prever o tempo cruzando os sensores
  // ------------------------------------------------------------

  if (erroNoDHT) {
    previsaoAtual = PREVISAO_SEM_DADOS;
  } else if (umidadeDoSolo >= SOLO_ENCHARCADO && umidadeDoAr >= AR_MUITO_UMIDO) {
    previsaoAtual = PREVISAO_SOLO_SATURADO;
  } else if (umidadeDoAr >= AR_MUITO_UMIDO && temperatura <= TEMPERATURA_DE_CHUVA) {
    previsaoAtual = PREVISAO_CHUVA;
  } else {
    previsaoAtual = PREVISAO_ESTAVEL;
  }

  // ------------------------------------------------------------
  // PARTE 3: decidir em que situacao estamos
  // ------------------------------------------------------------
  //
  // Repare no "!erroNoSensor" em todas as linhas: se o sensor
  // falhou, o programa NAO aciona nada. Na duvida, nao atua.
  // Bomba ligada por engano estraga mais do que bomba parada.

  garrafaEmCritico = (!erroNoSensorDaGarrafa &&
                      aguaNaGarrafa >= GARRAFA_LIMITE_CRITICO);

  garrafaEmAlerta  = (!erroNoSensorDaGarrafa &&
                      !garrafaEmCritico &&
                      aguaNaGarrafa >= GARRAFA_LIMITE_ALERTA);

  subsoloEmCritico = (!erroNoSensorDoSubsolo &&
                      aguaNoSubsolo >= SUBSOLO_LIMITE_CRITICO);

  subsoloEmAlerta  = (!erroNoSensorDoSubsolo &&
                      !subsoloEmCritico &&
                      aguaNoSubsolo >= SUBSOLO_LIMITE_ALERTA);

  if (subsoloEmCritico || garrafaEmCritico) {
    situacaoAtual = SITUACAO_CRITICO;
  } else if (subsoloEmAlerta || garrafaEmAlerta || estaChovendo) {
    situacaoAtual = SITUACAO_ALERTA;
  } else {
    situacaoAtual = SITUACAO_NORMAL;
  }

  // ------------------------------------------------------------
  // PARTE 4: agir
  // ------------------------------------------------------------

  cuidarDaBombaDeDreno(agora);
  cuidarDaBombaDeCaptacao(agora);
  cuidarDaFita(agora);
  cuidarDoAlarme();

  // ------------------------------------------------------------
  // PARTE 5: contar o que aconteceu
  // ------------------------------------------------------------

  atualizarTela();
  escreverNoSerial();
}


// ================================================================
// SECAO 11 - BOMBA DE DRENO (rele 1, pino 8)
// ================================================================
//
// Ela liga quando a garrafa passa de 2 cm, ou quando o subsolo
// alaga, e fica LIGADA DIRETO enquanto a situacao durar.
//
// O modo pulso so entra quando ha motivo:
//   - a garrafa passou dos 8 cm, ou
//   - a agua continuou subindo mesmo com a bomba ligada.
//
void cuidarDaBombaDeDreno(unsigned long agora) {

  // No modo manual 2 a bomba liga direto, sem olhar sensor nenhum.
  if (modoManual == 2) {
    bombaDeDrenoLigada = true;
    bombaEstaPulsando = false;
    mexerNoRele(PINO_RELE1, true, RELE1_LIGA_COM_LOW);
    return;
  }

  // Precisa drenar?
  bool precisaDrenar = false;

  if (garrafaEmAlerta || garrafaEmCritico) {
    precisaDrenar = true;
  }
  if (BOMBA_TAMBEM_DRENA_SUBSOLO && subsoloEmCritico) {
    precisaDrenar = true;
  }

  if (precisaDrenar) {

    // Primeira vez que liga nesta rodada: anota o ponto de partida
    // para depois saber se a agua subiu ou desceu.
    if (!bombaDeDrenoLigada) {
      momentoQueARele1Ligou = agora;
      nivelDaComparacaoAnterior = aguaNaGarrafa;
      momentoDaUltimaComparacao = agora;
      momentoDoUltimoPulso = agora;
      bombaEstaPulsando = false;
      faseAtualDoPulso = true;
    }

    bombaDeDrenoLigada = true;

    // Decide se deve pulsar.
    if (garrafaEmCritico) {
      // Passou de 8 cm: pulsa de qualquer jeito.
      bombaEstaPulsando = true;

    } else if (agora - momentoDaUltimaComparacao >= TEMPO_ENTRE_COMPARACOES) {
      // Hora de comparar: a agua subiu mesmo com a bomba ligada?
      if (aguaNaGarrafa > nivelDaComparacaoAnterior + SUBIDA_QUE_CONTA) {
        bombaEstaPulsando = true;
        Serial.println(F("[DIAGNOSTICO] A agua subiu com a bomba ligada. Pulsando para desentupir."));
      } else {
        bombaEstaPulsando = false;
      }

      nivelDaComparacaoAnterior = aguaNaGarrafa;
      momentoDaUltimaComparacao = agora;
    }

    // Aplica a decisao no rele.
    if (bombaEstaPulsando) {
      if (agora - momentoDoUltimoPulso >= TEMPO_DE_CADA_PULSO) {
        momentoDoUltimoPulso = agora;
        faseAtualDoPulso = !faseAtualDoPulso;   // inverte: liga/desliga
      }
      mexerNoRele(PINO_RELE1, faseAtualDoPulso, RELE1_LIGA_COM_LOW);
    } else {
      mexerNoRele(PINO_RELE1, true, RELE1_LIGA_COM_LOW);
    }

  } else {
    // A situacao passou. So desliga depois de cumprir a trava.
    if (bombaDeDrenoLigada && (agora - momentoQueARele1Ligou >= travaDasBombas)) {
      bombaDeDrenoLigada = false;
      bombaEstaPulsando = false;
    }
    mexerNoRele(PINO_RELE1, bombaDeDrenoLigada, RELE1_LIGA_COM_LOW);
  }
}


// ================================================================
// SECAO 12 - BOMBA DE CAPTACAO DE CHUVA (rele 2, pino 9)
// ================================================================
//
// Ela puxa a agua da chuva para dentro do reservatorio.
// Trava de seguranca: se a garrafa ja esta no critico, para de
// encher na hora, senao transborda.
//
void cuidarDaBombaDeCaptacao(unsigned long agora) {

  // Nos modos manuais 1 e 2 ela liga direto, para teste.
  if (modoManual >= 1) {
    bombaDeCaptacaoLigada = true;
    mexerNoRele(PINO_RELE2, true, RELE2_LIGA_COM_LOW);
    return;
  }

  bool querEncher = estaChovendo;

  if (SO_ENCHER_COM_SENSOR_OK && erroNoSensorDaGarrafa) {
    querEncher = false;
  }

  if (garrafaEmCritico) {
    // Reservatorio cheio: corta na hora, sem esperar trava nenhuma.
    bombaDeCaptacaoLigada = false;

  } else if (querEncher) {
    if (!bombaDeCaptacaoLigada) {
      momentoQueARele2Ligou = agora;
    }
    bombaDeCaptacaoLigada = true;

  } else if (bombaDeCaptacaoLigada &&
             (agora - momentoQueARele2Ligou >= travaDasBombas)) {
    bombaDeCaptacaoLigada = false;
  }

  mexerNoRele(PINO_RELE2, bombaDeCaptacaoLigada, RELE2_LIGA_COM_LOW);
}


// ================================================================
// SECAO 13 - FITA DE LED (guia de evacuacao)
// ================================================================
void cuidarDaFita(unsigned long agora) {

  bool deveAcender = false;

  if (subsoloEmCritico) {
    deveAcender = true;
  }
  if (FITA_ACENDE_NO_ALERTA && subsoloEmAlerta) {
    deveAcender = true;
  }

  if (deveAcender) {
    if (!fitaAcesa) {
      momentoQueAFitaAcendeu = agora;
    }
    fitaAcesa = true;
    fitaVermelha = subsoloEmCritico;

  } else if (fitaAcesa && (agora - momentoQueAFitaAcendeu >= travaDasBombas)) {
    fitaAcesa = false;
  }
}


// ================================================================
// SECAO 14 - ALARME SONORO
// ================================================================
//
// Cada origem tem um som diferente, para quem escuta saber do que
// se trata sem precisar olhar o painel.
//
void cuidarDoAlarme() {

  if (subsoloEmCritico) {
    tone(PINO_SOM, 1400, 400);          // mais agudo: perigo a pessoas

  } else if (garrafaEmCritico) {
    tone(PINO_SOM, 1000, 400);          // mais grave: perigo ao equipamento

  } else if (subsoloEmAlerta || garrafaEmAlerta || estaChovendo) {
    tone(PINO_SOM, 800, 120);           // bipe curto de aviso

  } else {
    noTone(PINO_SOM);                   // silencio
  }
}


// ================================================================
// SECAO 15 - ANIMACAO DA FITA DE LED
// ================================================================
//
// Uma onda de luz corre pela fita na direcao da saida. Vermelha
// no critico, ambar no alerta.
//
// Como funciona: a onda se repete a cada 7 LEDs. Cada LED calcula
// a que distancia esta da cabeca da onda. Os 4 primeiros acendem,
// cada um mais fraco que o anterior, formando um rastro. A cada
// 85 ms a cabeca anda um LED para a frente.
//
void animarFita(unsigned long agora) {

  if (agora - momentoDoUltimoPassoDaFita < 85) {
    return;
  }
  momentoDoUltimoPassoDaFita = agora;

  fita.clear();

  if (fitaAcesa) {

    // Brilho de cada posicao: cabeca forte, rastro fraco.
    byte brilho[4] = {255, 180, 90, 30};

    // Quanto verde misturar no vermelho.
    // Zero da vermelho puro, 110 da ambar.
    byte verde = 110;
    if (fitaVermelha) {
      verde = 0;
    }

    for (byte i = 0; i < QUANTOS_LEDS; i++) {

      byte posicaoNaOnda = (i + TAMANHO_DA_ONDA - passoDaOnda) % TAMANHO_DA_ONDA;

      if (posicaoNaOnda <= 3) {
        int forca = brilho[posicaoNaOnda];
        fita.setPixelColor(i, fita.Color(forca, (verde * forca) / 255, 0));
      }
    }

    passoDaOnda = passoDaOnda + 1;
    if (passoDaOnda >= TAMANHO_DA_ONDA) {
      passoDaOnda = 0;
    }
  }

  fita.show();
}


// ================================================================
// SECAO 16 - DISPLAY OLED
// ================================================================
//
// Como fica a tela:
//
//   MIXTAPE   01:11  CHUVA
//   ----------------------
//   Res: 7.2cm 276mL
//   Sub: 3.1cm 27%
//   T24C H55% S100% L24%
//   CRITICO P1 B2 20s
//
//   B1 = bomba de dreno ligada direto
//   P1 = bomba de dreno pulsando
//   B2 = bomba de captacao ligada
//   20s = trava escolhida no botao
//   ERR = algum sensor com erro
//
// O display esta em modo pagina, entao o bloco do do/while roda
// varias vezes seguidas, uma para cada pedaco da tela. Isso e
// normal e nao precisa mexer.
//
void atualizarTela() {

  tela.firstPage();
  do {

    // ---------- Linha de cima ----------
    tela.setFont(u8g2_font_5x7_tr);
    tela.drawStr(0, 7, "MIXTAPE");

    tela.setCursor(46, 7);
    if (relogioFuncionando) {
      DateTime hora = relogio.now();
      if (hora.hour() < 10) {
        tela.print('0');
      }
      tela.print(hora.hour());
      tela.print(':');
      if (hora.minute() < 10) {
        tela.print('0');
      }
      tela.print(hora.minute());
    } else {
      tela.print(F("--:--"));
    }

    if (estaChovendo) {
      tela.drawStr(84, 7, "CHUVA");
    }
    if (modoManual > 0) {
      tela.drawStr(120, 7, "M");
    }

    tela.drawLine(0, 10, 128, 10);

    // ---------- Reservatorio ----------
    tela.setFont(u8g2_font_6x12_tr);
    tela.setCursor(0, 23);
    tela.print(F("Res: "));
    if (erroNoSensorDaGarrafa) {
      tela.print(F("ERRO"));
    } else {
      tela.print(aguaNaGarrafa, 1);
      tela.print(F("cm "));
      tela.print(volumeNaGarrafa);
      tela.print(F("mL"));
    }

    // ---------- Subsolo ----------
    tela.setCursor(0, 35);
    tela.print(F("Sub: "));
    if (erroNoSensorDoSubsolo) {
      tela.print(F("ERRO"));
    } else {
      tela.print(aguaNoSubsolo, 1);
      tela.print(F("cm "));
      tela.print(porcentoAlagado);
      tela.print('%');
    }

    // ---------- Ambiente ----------
    tela.setFont(u8g2_font_5x7_tr);
    tela.setCursor(0, 46);
    tela.print('T');
    tela.print((int)temperatura);
    tela.print(F("C H"));
    tela.print((int)umidadeDoAr);
    tela.print(F("% S"));
    tela.print(umidadeDoSolo);
    tela.print(F("% L"));
    tela.print(luzMedia);
    tela.print('%');

    // ---------- Linha de baixo ----------
    tela.setFont(u8g2_font_6x12_tr);
    tela.setCursor(0, 62);
    if (situacaoAtual == SITUACAO_CRITICO) {
      tela.print(F("CRITICO"));
    } else if (situacaoAtual == SITUACAO_ALERTA) {
      tela.print(F("ALERTA"));
    } else {
      tela.print(F("NORMAL"));
    }

    tela.setFont(u8g2_font_5x7_tr);

    if (bombaDeDrenoLigada) {
      tela.setCursor(46, 62);
      if (bombaEstaPulsando) {
        tela.print(F("P1"));
      } else {
        tela.print(F("B1"));
      }
    }

    if (bombaDeCaptacaoLigada) {
      tela.drawStr(62, 62, "B2");
    }

    tela.setCursor(78, 62);
    tela.print(travaDasBombas / 1000);
    tela.print('s');

    if (erroNoDHT || erroNoSensorDaGarrafa || erroNoSensorDoSubsolo) {
      tela.drawStr(104, 62, "ERR");
    }

  } while (tela.nextPage());
}


// ================================================================
// SECAO 17 - RELATORIO NO SERIAL MONITOR
// ================================================================
//
// Uma linha por segundo com tudo que o sistema sabe naquele
// instante. E por aqui que se prova, no video, que a decisao do
// programa bate com o que esta acontecendo na tela.
//
void escreverNoSerial() {

  // ---------- Hora ----------
  Serial.print('[');
  if (relogioFuncionando) {
    DateTime hora = relogio.now();
    if (hora.hour() < 10)   Serial.print('0');
    Serial.print(hora.hour());
    Serial.print(':');
    if (hora.minute() < 10) Serial.print('0');
    Serial.print(hora.minute());
    Serial.print(':');
    if (hora.second() < 10) Serial.print('0');
    Serial.print(hora.second());
  } else {
    Serial.print(F("--:--:--"));
  }
  Serial.print(F("] "));

  // ---------- Situacao ----------
  if (situacaoAtual == SITUACAO_CRITICO) {
    Serial.print(F("CRITICO"));
  } else if (situacaoAtual == SITUACAO_ALERTA) {
    Serial.print(F("ALERTA"));
  } else {
    Serial.print(F("NORMAL"));
  }

  // ---------- Reservatorio ----------
  Serial.print(F(" | Reservatorio: "));
  if (erroNoSensorDaGarrafa) {
    Serial.print(F("ERRO"));
  } else {
    Serial.print(aguaNaGarrafa, 1);
    Serial.print(F(" cm / "));
    Serial.print(volumeNaGarrafa);
    Serial.print(F(" mL"));

    // Quanto tempo falta para esvaziar, usando a vazao medida.
    if (bombaDeDrenoLigada && volumeNaGarrafa > 0) {
      int segundos = volumeNaGarrafa / BOMBA_ML_POR_SEGUNDO;

      // Pulsando, a bomba so trabalha metade do tempo: demora o dobro.
      if (bombaEstaPulsando) {
        segundos = segundos * 2;
      }

      Serial.print(F(" (esvazia em ~"));
      Serial.print(segundos);
      Serial.print(F("s)"));
    }
  }

  // ---------- Subsolo ----------
  Serial.print(F(" | Subsolo: "));
  if (erroNoSensorDoSubsolo) {
    Serial.print(F("ERRO"));
  } else {
    Serial.print(aguaNoSubsolo, 1);
    Serial.print(F(" cm ("));
    Serial.print(porcentoAlagado);
    Serial.print(F("% alagado)"));
  }

  // ---------- Ambiente ----------
  Serial.print(F(" | Temp: "));
  if (erroNoDHT) {
    Serial.print(F("ERRO"));
  } else {
    Serial.print(temperatura, 1);
    Serial.print('C');
  }

  Serial.print(F(" | Umid ar: "));
  Serial.print(umidadeDoAr, 0);
  Serial.print(F("% | Solo: "));
  Serial.print(umidadeDoSolo);
  Serial.print(F("% | Luz: "));
  Serial.print(luzMedia);
  Serial.print('%');

  Serial.print(F(" | Chuva: "));
  if (estaChovendo) {
    Serial.print(F("SIM"));
  } else {
    Serial.print(F("NAO"));
  }

  // ---------- O que esta ligado ----------
  Serial.print(F(" | Dreno: "));
  if (!bombaDeDrenoLigada) {
    Serial.print(F("OFF"));
  } else if (bombaEstaPulsando) {
    Serial.print(F("PULSANDO"));
  } else {
    Serial.print(F("LIGADA"));
  }

  Serial.print(F(" | Captacao: "));
  if (bombaDeCaptacaoLigada) {
    Serial.print(F("ON"));
  } else {
    Serial.print(F("OFF"));
  }

  Serial.print(F(" | Fita: "));
  if (!fitaAcesa) {
    Serial.print(F("OFF"));
  } else if (fitaVermelha) {
    Serial.print(F("VERMELHA"));
  } else {
    Serial.print(F("AMBAR"));
  }

  // ---------- Ajustes e previsao ----------
  Serial.print(F(" | Manual: "));
  Serial.print(modoManual);

  Serial.print(F(" | Trava: "));
  Serial.print(travaDasBombas / 1000);
  Serial.print('s');

  Serial.print(F(" | Previsao: "));
  if (previsaoAtual == PREVISAO_SEM_DADOS) {
    Serial.println(F("SEM DADOS"));
  } else if (previsaoAtual == PREVISAO_SOLO_SATURADO) {
    Serial.println(F("SOLO SATURADO"));
  } else if (previsaoAtual == PREVISAO_CHUVA) {
    Serial.println(F("CHUVA A CAMINHO"));
  } else {
    Serial.println(F("ESTAVEL"));
  }
}