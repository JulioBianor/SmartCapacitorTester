#include <U8g2lib.h>

// Display ST7567 128x32 via SPI
U8G2_ST7567_OS12864_F_4W_SW_SPI u8g2(U8G2_R2, /* clock=*/ 7, /* data=*/ 4, /* cs=*/ 2, /* dc=*/ 3, /* reset=*/ 8);

// Pinos
#define PIN_100K A0
#define PIN_10K  A2
#define PIN_ADC  A1

#define THRESHOLD_ADC 648
#define TIMEOUT_US    3000000UL

float absFloat(float x){
  return x < 0 ? -x : x;
}

// Tabela de valores comerciais de capacitância (em Farads)
const float valoresComerciais[] = {
  1e-12, 2.2e-12, 3.3e-12, 4.7e-12, 10e-12, 22e-12, 33e-12, 47e-12, 100e-12, 220e-12, 330e-12, 470e-12, 1e-9, 2.2e-9, 3.3e-9, 4.7e-9,
  10e-9, 22e-9, 33e-9, 47e-9, 100e-9, 220e-9, 330e-9, 470e-9, 1e-6, 2.2e-6, 3.3e-6, 4.7e-6, 10e-6, 22e-6, 33e-6, 47e-6, 100e-6
};

unsigned long trocaTimestamp = 0;
bool usando10K = false;
float capacitancia = 0.00;

// Calcula C = t / R (t em us → s)
float calcularCapacitancia(unsigned long tempo, float resistencia) {
  const float fatorCalibracao = 4.464;
  return (tempo / (resistencia * 1e6)) * fatorCalibracao;
  //return (float)tempo / (resistencia * 1e6); // resultado em Farads
}


float valorComercialMaisProximo(float valorMedido) {
  float maisProximo = valoresComerciais[0];
  float menorErro = absFloat(valorMedido - maisProximo);

  const int NUM_VALORES = sizeof(valoresComerciais) / sizeof(valoresComerciais[0]);

  for (int i = 1; i < NUM_VALORES; i++) {
    float erro = absFloat(valorMedido - valoresComerciais[i]);
    if (erro < menorErro) {
      menorErro = erro;
      maisProximo = valoresComerciais[i];
    }
  }
  return maisProximo;
}




void setup() {
  analogReference(INTERNAL);  // 1.1V
  delay(50);
  Serial.begin(115200);
  u8g2.begin();

  pinMode(PIN_100K, INPUT);
  pinMode(PIN_10K, INPUT);
  pinMode(PIN_ADC, INPUT);

  u8g2.setFont(u8g2_font_6x13_tr);
  u8g2.setContrast(35);
  u8g2.clearBuffer();
  u8g2.drawStr(0, 16, "Capacimetro Automatico");
  u8g2.sendBuffer();
  delay(1000);
}

void loop() {
  capacitancia = medirCapacitancia();
  exibeDisplay();
  Serial.print("Capacitancia calculada: ");
  Serial.println(capacitancia, 12); // Mostra até 12 casas decimais para investigar

  delay(1000);

}

float medirCapacitancia() {
  float resultado = -1;
  usando10K = false;

  // 1ª tentativa com 100k
  configurarCarga(PIN_100K);
  delayEstabilizacao();
  unsigned long tempo = medirTempoCarga();
  if (tempo > 0 && tempo < TIMEOUT_US) {
    resultado = calcularCapacitancia(tempo, 100000.0);
    desligarTodos();
    return resultado;
  }

  // 2ª tentativa com 10k
  configurarCarga(PIN_10K);
  delayEstabilizacao();
  tempo = medirTempoCarga();
  if (tempo > 0 && tempo < TIMEOUT_US) {
    resultado = calcularCapacitancia(tempo, 10000.0);
    usando10K = true;
  }

  desligarTodos();
  return resultado;
}

void configurarCarga(uint8_t pino) {
  desligarTodos();
  pinMode(pino, OUTPUT);
  digitalWrite(pino, HIGH);
  trocaTimestamp = millis();
}

void desligarTodos() {
  pinMode(PIN_100K, INPUT);
  pinMode(PIN_10K, INPUT);
}

void delayEstabilizacao() {
  while (millis() - trocaTimestamp < 3);
}

unsigned long medirTempoCarga() {
  // Descarrega capacitor
  pinMode(PIN_ADC, OUTPUT);
  digitalWrite(PIN_ADC, LOW);
  delay(50);
  pinMode(PIN_ADC, INPUT);

  // Verifica se já está carregado
  if (analogRead(PIN_ADC) >= THRESHOLD_ADC) return -1;

  unsigned long start = micros();
  while (analogRead(PIN_ADC) < THRESHOLD_ADC) {
    if (micros() - start > TIMEOUT_US) return -1;
  }
  return micros() - start;
}

void exibeDisplay(){

  u8g2.clearBuffer();
  if (capacitancia < 0) {
    u8g2.drawStr(0, 16, "Falha na medicao");
    Serial.println("[!] Nao foi possivel medir");
  } else {
    //char buffer[20];
    float valorRef = valorComercialMaisProximo(capacitancia);
    u8g2.setCursor(0,12);
    if (capacitancia < 1e-9){
      u8g2.print( capacitancia * 1e12); u8g2.print("pF "); u8g2.print("  Ref: "); u8g2.print(valorRef * 1e12); u8g2.print(" pF");
    }else if (capacitancia < 1e-6){
      u8g2.print( capacitancia * 1e9); u8g2.print("nF "); u8g2.print("  Ref: "); u8g2.print(valorRef * 1e9); u8g2.print(" nF");
    }else{
      u8g2.print( capacitancia * 1e6); u8g2.print("uF "); u8g2.print("  Ref: "); u8g2.print(valorRef * 1e6); u8g2.print(" uF");
      u8g2.drawStr(0, 28, usando10K ? "Faixa: 10K" : "Faixa: 100K");
    }
  }
  u8g2.sendBuffer();
  //Serial.println(bufferComercial);

}
