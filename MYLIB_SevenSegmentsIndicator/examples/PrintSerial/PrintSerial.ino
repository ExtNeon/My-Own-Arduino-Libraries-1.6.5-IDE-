#include <SevenSegmentsIndicator.h>

SevenSegmentsIndicator indicator;

  byte segmentsPins[8] = {2, 3, 4, 5, 6, 7, 8, 14};
  byte digitsPins[6] = {10, 11, 12, 17, 18, 19};

void setup()
{
  // Костыльно создаём экземпляр индикатора. При этом пины A, B, C, D, E, F, G, dp подключаем к ногам 2 - 9 соответственно, а пины разрядов к 10 - 12.
  Serial.begin(115200);
  indicator = SevenSegmentsIndicator(segmentsPins, 6, digitsPins);
    // Timer0 уже используется millis() - мы создаем прерывание где-то
  // в середине и вызываем ниже функцию "Compare A"
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
}

String buf = "";

void loop()
{ 
  // Выводим на дисплей то, что ввели в порт
  while (Serial.available()) {
    char a = (char) Serial.read();
    if (a == '\n') {
      indicator.print(buf, false);
      buf = "";
    } else {
      buf += a;
    }
  }
  //indicator.print(String(millis() / 1000));
  
}

SIGNAL(TIMER0_COMPA_vect) {
  indicator.refreshNext(); 
}
