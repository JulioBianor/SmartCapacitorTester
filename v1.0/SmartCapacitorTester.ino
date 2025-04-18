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

const float commercialValues[] = { 
  //can be expanded
  //  1.0 1.2 1.5 1.8 2.2 2.7 3.3 3.9 4.7 5.6 6.8 8.2
  //  10  12  15  18  22  27  33  39  47  56  68  82
  //  100 120 150 180 220 270 330 390 470 560 680 820

//pF
  1e-12,   2.2e-12, 3.3e-12, 4.7e-12,
  10e-12,  22e-12,  33e-12,  47e-12,
  100e-12, 220e-12, 330e-12, 470e-12,

//nF
  1e-9,    2.2e-9,  3.3e-9,  4.7e-9,
  10e-9,   22e-9,   33e-9,   47e-9,
  100e-9,  220e-9,  330e-9,  470e-9,

//uF
  1e-6,    2.2e-6,  3.3e-6,  4.7e-6,
  10e-6,   22e-6,   33e-6,   47e-6,
  100e-6   220e-6,  330e-6,  470e-6,
};

unsigned long trocaTimestamp = 0;
bool using10K = false;
float capacitancia = 0.00;

// Calc C = t / R (t in us → s)
float calcularCapacitancia(unsigned long time, float resistence) {
  const float fatorCalib = 4.464;
  return (time / (resistence * 1e6)) * fatorCalib;
}


float valorComercialMaisProximo(float valorMedido) {
  float maisProximo = commercialValues[0];
  float minorError = absFloat(valorMedido - maisProximo);

  const int NUM_VALORES = sizeof(commercialValues) / sizeof(commercialValues[0]);

  for (int i = 1; i < NUM_VALORES; i++) {
    float erro = absFloat(valorMedido - commercialValues[i]);
    if (erro < minorError) {
      minorError = erro;
      maisProximo = commercialValues[i];
    }
  }
  return maisProximo;
}




void setup() {
  analogReference(INTERNAL);  // 1.1V, needed in Pro Mini Analog Read
  delay(50);
  Serial.begin(115200);
  u8g2.begin();

  pinMode(PIN_100K, INPUT);
  pinMode(PIN_10K, INPUT);
  pinMode(PIN_ADC, INPUT);

  u8g2.setFont(u8g2_font_6x13_tr);
  u8g2.setContrast(35); //optional
  u8g2.clearBuffer();
  u8g2.drawStr(0, 16, "Smart Capacitor Tester v1.0");
  u8g2.sendBuffer();
  delay(1000);
}

void loop() {
  capacitancia = medirCapacitancia();
  exibeDisplay();
  
  Serial.print("C: ");
  Serial.println(capacitancia, 12); // for terminal debug
  delay(1000);

}

float medirCapacitancia() {
  float resultado = -1;
  using10K = false;

  // 1ª tentativa com 100k
  configurarCarga(PIN_100K);
  delayEstabilizacao();
  unsigned long time = medirtimeCarga();
  if (time > 0 && time < TIMEOUT_US) {
    resultado = calcularCapacitancia(time, 100000.0);
    desligarTodos();
    return resultado;
  }

  // 2ª tentativa com 10k
  configurarCarga(PIN_10K);
  delayEstabilizacao();
  time = medirtimeCarga();
  if (time > 0 && time < TIMEOUT_US) {
    resultado = calcularCapacitancia(time, 10000.0);
    using10K = true;
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

unsigned long medirtimeCarga() {
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
    u8g2.drawStr(0, 16, "Unable to measure");
    Serial.println("[!] Unable to measure");
  } else {
    //char buffer[20];
    float valorRef = valorComercialMaisProximo(capacitancia);
    u8g2.setCursor(0,12);
    if (capacitancia < 1e-9){
      u8g2.print( capacitancia * 1e12); u8g2.print("pF "); u8g2.print("  Ref: "); u8g2.print(valorRef * 1e12); u8g2.print("pF");
    }else if (capacitancia < 1e-6){
      u8g2.print( capacitancia * 1e9); u8g2.print("nF "); u8g2.print("  Ref: "); u8g2.print(valorRef * 1e9); u8g2.print("nF");
    }else{
      u8g2.print( capacitancia * 1e6); u8g2.print("uF "); u8g2.print("  Ref: "); u8g2.print(valorRef * 1e6); u8g2.print("uF");
    }
  }
  u8g2.sendBuffer();
  //Serial.println(bufferComercial);

}
