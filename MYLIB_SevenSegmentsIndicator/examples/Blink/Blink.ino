#include <SevenSegmentsIndicator.h>

SevenSegmentsIndicator indicator;

  byte segmentsPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};
  byte digitsPins[3] = {10, 11, 12};

void setup()
{
  // Костыльно создаём экземпляр индикатора. При этом пины A, B, C, D, E, F, G, dp подключаем к ногам 2 - 9 соответственно, а пины разрядов к 10 - 12.

  indicator = SevenSegmentsIndicator(segmentsPins, 3, digitsPins);
}

void loop()
{ 
  // Выводим на дисплей количество секунд, которые контроллер работает. Каждые 10 миллисекунд меняем разряд.
  indicator.print(String(millis() / 1000));
  if (millis() % 10 == 0) {
    indicator.refreshNext(); 
  }
}
