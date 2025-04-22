#define PIN_MEASURE A0
#define TIMEOUT_US 2000000
#define ADC_THRESHOLD 648

const uint8_t resistorPins[] = {12, 11, 10};  // 1M, 100k, 10k
const float resistorValues[] = {1000000.0, 100000.0, 10000.0};

float calibrationFactor = 1.0;
const float CALIB_FACTOR_LOWVCC = 4.64;
bool lowVoltage = false;

void setup() {
  Serial.begin(9600);
  delay(100);

  long vcc = readVcc();  // Detecta tensão real

  Serial.println(F("\n== SmartCapacitorTester v1.0 =="));
  Serial.print(F("VCC detectado: "));
  Serial.print(vcc);
  Serial.println(F(" mV"));

  if (vcc < 4700) {
    analogReference(INTERNAL);
    calibrationFactor = CALIB_FACTOR_LOWVCC;
    lowVoltage = true;
    Serial.println(F(">> Modo bateria: analogReference(INTERNAL), fator 4.64"));
  } else {
    analogReference(DEFAULT);
    calibrationFactor = 1.0;
    lowVoltage = false;
    Serial.println(F(">> Modo USB/5V: analogReference(DEFAULT), fator 1.0"));
  }

  pinMode(PIN_MEASURE, INPUT);
  for (uint8_t i = 0; i < 3; i++) {
    pinMode(resistorPins[i], INPUT);
  }
}

void loop() {
  float capacitance = -1;
  unsigned long tempoMedido = 0;
  uint8_t faixaUsada = 255;

  for (uint8_t i = 0; i < 3; i++) {
    dischargeCapacitor();
    activateResistor(resistorPins[i]);

    tempoMedido = measureChargeTime();
    deactivateAllResistors();

    if (tempoMedido > 5 && tempoMedido < TIMEOUT_US) {
      capacitance = (tempoMedido / (resistorValues[i] * 1e6)) * calibrationFactor;
      faixaUsada = i;
      break;
    }
  }

  Serial.println(F("\n------------------------------"));
  if (capacitance > 0 && faixaUsada < 3) {
    Serial.print(F("Resistor usado: "));
    Serial.print(resistorValues[faixaUsada] / 1000);
    Serial.print(F("kΩ | Tempo: "));
    Serial.print(tempoMedido);
    Serial.print(F(" us | Capacitância: "));

    if (capacitance < 1e-9) {
      Serial.print(capacitance * 1e12, 2);
      Serial.println(F(" pF"));
    } else if (capacitance < 1e-6) {
      Serial.print(capacitance * 1e9, 2);
      Serial.println(F(" nF"));
    } else {
      Serial.print(capacitance * 1e6, 2);
      Serial.println(F(" uF"));
    }
  } else {
    Serial.println(F("✖️ Medição inválida ou capacitor ausente."));
  }

  delay(1000);
}

// === Funções auxiliares ===

void dischargeCapacitor() {
  deactivateAllResistors();
  pinMode(PIN_MEASURE, OUTPUT);
  digitalWrite(PIN_MEASURE, LOW);
  delayMicroseconds(100);
  pinMode(PIN_MEASURE, INPUT);
}

void activateResistor(uint8_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void deactivateAllResistors() {
  for (uint8_t i = 0; i < 3; i++) {
    pinMode(resistorPins[i], INPUT);
  }
}

unsigned long measureChargeTime() {
  if (analogRead(PIN_MEASURE) >= ADC_THRESHOLD) return -1;

  unsigned long start = micros();
  while (analogRead(PIN_MEASURE) < ADC_THRESHOLD) {
    if ((micros() - start) > TIMEOUT_US) return -1;
  }
  return micros() - start;
}

long readVcc() {
  // Lê a Vref interna de 1.1V usando VCC como referência
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  int adc = ADC;
  return (1100L * 1024L) / adc;  // Retorna em milivolts
}