#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Ultrasonic.h>

SoftwareSerial mySerial(8, 9); // RX (D8 <- TX do DFPlayer), TX (D9 -> RX do DFPlayer)
DFRobotDFPlayerMini player;

Ultrasonic ultrasonic(11, 12); // TRIG = 11, ECHO = 12

// Botões
const int BTN1 = 13;       // áudio 2, fio amarelo
const int BTN2 = 4;        // áudio 3 , fio branco
const int BTN_IDIOMA = 5;  // troca de idioma, fio roxo
const int pinoFone = 6
const int pinoFone_2 = 7

// Arquivos dentro de cada pasta de idioma
const int ARQ_APROXIMACAO = 1;
const int ARQ_BOTAO1 = 2;
const int ARQ_BOTAO2 = 3;
const int ARQ_IDIOMA = 4;

const int NUM_IDIOMAS = 3;   // 1 = PT, 2 = EN, 3 = ES
int idioma = 1;

// Distâncias e tempos
const int DIST_ATIVAR = 20;                // cm: começa o áudio 1
const int DIST_PARAR = DIST_ATIVAR + 10;   // cm: acima disso a pessoa "se afastou"
const unsigned long TEMPO_PARAR = 500;     // ms longe antes de parar o áudio 1
const unsigned long TEMPO_AUSENTE = 2000;  // ms longe antes de rearmar o sensor

bool pessoaPerto = false;      // trava: impede repetir o áudio 1
bool audio1Tocando = false;    // true só antes de qualquer botão ser apertado

unsigned long ultimaLeitura = 0;
unsigned long ultimaPresenca = 0;

bool ant1 = HIGH, ant2 = HIGH, antIdioma = HIGH;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN_IDIOMA, INPUT_PULLUP);

  Serial.println("Iniciando DFPlayer...");
  if (!player.begin(mySerial)) {
    Serial.println("Erro: verifique as ligacoes e o cartao SD.");
    while (true);
  }

  player.volume(30); // 0 a 30
  Serial.println("Pronto!");
}

// Retorna true só no instante do clique
bool clicou(int pino, bool &anterior) {
  bool atual = digitalRead(pino);
  bool evento = (anterior == HIGH && atual == LOW);
  anterior = atual;
  return evento;
}

void loop() {
  // ---------- SENSOR DE PROXIMIDADE (lê a cada 100 ms) ----------
  if (millis() - ultimaLeitura >= 100) {
    ultimaLeitura = millis();
    int distance = ultrasonic.read(); // cm

    // Pessoa dentro da zona de presença (1 a 30 cm)
    if (distance > 0 && distance <= DIST_PARAR) {
      ultimaPresenca = millis();
    }

    // Toca o áudio 1 só na primeira detecção (a 20 cm ou menos)
    if (!pessoaPerto && distance > 0 && distance <= DIST_ATIVAR) {
      pessoaPerto = true;
      audio1Tocando = true;
      Serial.println("Pessoa detectada: audio 1");
      player.playFolder(idioma, ARQ_APROXIMACAO);
    }

    // Pessoa se afastou e nenhum botão foi apertado: para o áudio 1
    if (audio1Tocando && millis() - ultimaPresenca > TEMPO_PARAR) {
      audio1Tocando = false;
      player.stop();
      Serial.println("Pessoa se afastou: audio 1 parado");
    }

    // Rearma o sensor depois de 2 s longe
    if (pessoaPerto && millis() - ultimaPresenca > TEMPO_AUSENTE) {
      pessoaPerto = false;
      Serial.println("Sensor rearmado");
    }
  }

  // ---------- BOTÕES ----------
  if (clicou(BTN1, ant1)) {
    audio1Tocando = false;
    Serial.println("Botao 1: audio 2");
    player.playFolder(idioma, ARQ_BOTAO1);
    delay(50);
  }

  if (clicou(BTN2, ant2)) {
    audio1Tocando = false;
    Serial.println("Botao 2: audio 3");
    player.playFolder(idioma, ARQ_BOTAO2);
    delay(50);
  }

  if (clicou(BTN_IDIOMA, antIdioma)) {
    audio1Tocando = false;
    idioma++;
    if (idioma > NUM_IDIOMAS) idioma = 1;
    Serial.print("Idioma: ");
    Serial.println(idioma);
    player.playFolder(idioma, ARQ_IDIOMA);
    delay(50);
  }
}
