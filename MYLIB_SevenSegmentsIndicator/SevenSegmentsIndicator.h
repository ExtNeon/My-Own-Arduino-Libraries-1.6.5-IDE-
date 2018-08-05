/**
	SevenSegmentsIndicator_h - библиотека, позволяющая взаимодействовать с семисегментным индикатором, как с ООП - объектом.
	Позволяет подключать семисегментные индикаторы с любым количеством цифр и любой полярностью сегментов.
	Умеет выводить цифры, в том числе и дробные, а также некоторые буквы (ABCDEFGHIJLNOPQRSTUVY) и символы (-; [; ] и .). Также, имеется возможность вывода собственных символов.
	Индикация осуществляется динамическим способом. Для нормальной работы индикатора нужно периодически вызывать метод refreshNext(). Он переключает активный разряд и выводит на 
	него символ, находящийся в позиции данного разряда в памяти. В зависимости от периода вызова данного метода будет изменяться скорость смены разрядов индикатора.
	Не рекомендуется вызывать его реже, чем в каждые 20 миллисекунд, так как при увеличении интервала станет заметным процесс переключения разрядов.
	
	При создании указывается:
		* Массив байтов, каждый из которых является номером пина определённого сегмента. Последовательность пинов: A, B, C, D, E, F, G, dp.
		* Количество разрядов
		* Массив байтов, каждый из которых является номером пина определённого разряда.
		* Необязательный параметр: Тип выводов разряда индикатора. Принимает булево значение. Если к выводу разряда подключены катоды светодиодов сегментов, 
			то нужно указать SSI_DGPIN_KATHODE. По умолчанию установлено значение, при котором работают индикаторы, с объединёнными анодами светодиодов сегментов.
			Если вы подключили всё правильно, но индикатор не светится, попробуйте указать это значение.
	
	Для вывода информации на дисплей можно использовать следующие методы:
		* Метод print(String str, boolean shiftToRight). Принимает на вход строку, интерпретирует её и записывает в память для вывода. Необязательный параметр - режим выравнивания.
			Если указано false, то выводимый текст будет выровнен слева, по умолчанию справа.
		* Метод setDigitValue(byte digitIndex, byte value). Принимает на вход индекс разряда и устанавливоемое значение. Устанавливает определённое значение разряда по индексу.
			Значение разряда вводится в форме байта, в котором каждый бит соотносится с определённым сегментом. 
			Последовательность (начиная со старшего бита): A, B, C, D, E, F, G, dp.
		* Метод displayCustomSymbols(byte* symbols, byte countOfSymbols). Позволяет вывести последовательность пользовательских символов с выравниванием слева. Принимает на вход
			массив байтов, каждый байт из которого соотносится с определённым разрядом, а каждый бит с определённым сегментом в данном разряде; и его длину.
			Последовательность (начиная со старшего бита): A, B, C, D, E, F, G, dp.
		
	Можно выключать и опять включать индикацию  методом setPowerState(), останавливать смену разрядов методом stopRefreshing() и возобновлять её методом resumeRefreshing().
	Также, можно физически отключить индикацию точки при помощи метода setPointShow(), который принимает на вход булево значение.
	Написано за один вечер. ExtNeon. 06.06.2018
	
	
	Карта сегментов:

		|--A--|
		F	  B
		|--G--|
		E	  C
		|--D--|
				dp
		
*/

// проверка, что библиотека еще не подключена
#ifndef SevenSegmentsIndicator_h // если библиотека не подключена
#define SevenSegmentsIndicator_h // тогда подключаем ее
#include "Arduino.h"

const byte SSI_DIGIT_ZERO = 0b11111100;
const byte SSI_DIGIT_ONE = 0b01100000;
const byte SSI_DIGIT_TWO = 0b11011010;
const byte SSI_DIGIT_THREE = 0b11110010;
const byte SSI_DIGIT_FOUR = 0b01100110;
const byte SSI_DIGIT_FIVE = 0b10110110;
const byte SSI_DIGIT_SIX = 0b10111110;
const byte SSI_DIGIT_SEVEN = 0b11100000;
const byte SSI_DIGIT_EIGHT = 0b11111110;
const byte SSI_DIGIT_NINE = 0b11110110;

const byte SSI_LETTER_A = 0b11101110;
const byte SSI_LETTER_B = 0b00111110;
const byte SSI_LETTER_C = 0b10011100;
const byte SSI_LETTER_D = 0b01111010;
const byte SSI_LETTER_E = 0b10011110;
const byte SSI_LETTER_F = 0b10001110;
const byte SSI_LETTER_G = 0b11011110;
const byte SSI_LETTER_H = 0b01101110;
const byte SSI_LETTER_I = SSI_DIGIT_ONE;
const byte SSI_LETTER_J = 0b01111000;
const byte SSI_LETTER_L = 0b00011100;
const byte SSI_LETTER_N = 0b11101100;
const byte SSI_LETTER_O = SSI_DIGIT_ZERO;
const byte SSI_LETTER_P = 0b11001110;
const byte SSI_LETTER_Q = 0b11100110;
const byte SSI_LETTER_R = 0b10001100;
const byte SSI_LETTER_S = SSI_DIGIT_FIVE;
const byte SSI_LETTER_T = 0b00011110;
const byte SSI_LETTER_U = 0b01111100;
const byte SSI_LETTER_V = SSI_LETTER_U;
const byte SSI_LETTER_Y = 0b01110110;

const byte SSI_SYMBOLS_MINUS = 0b00000010;
const byte SSI_SYMBOLS_OPEN_BRACKET = 0b10011100;
const byte SSI_SYMBOLS_CLOSED_BRACKET = 0b11110000;
const byte SSI_ADDITIVE_DOTPOINT = 0b00000001;


#define SSI_DGPIN_ANODE true
#define SSI_DGPIN_KATHODE false


class SevenSegmentsIndicator {
	public:
		SevenSegmentsIndicator(byte *segmentPins, byte countOfDigits, byte *digitsPins, boolean digitPinType = SSI_DGPIN_ANODE); //+
		SevenSegmentsIndicator();
		void refreshNext(); //+
		void print(String str, boolean shiftToRight = true); //+
		void displayCustomSymbols(byte* symbols, byte countOfSymbols); //++
		void setPowerState(boolean enabled); //+
		boolean getPowerState(); //+
		void stopRefreshing(); //+
		void resumeRefreshing(); //+
		void setDigitValue(byte digitIndex, byte value); //+
		void setPointShow(boolean enabled); //+
	private:
		byte *_segmentPins;
		byte *_digitPins;
		byte _currentActiveDigit = 0;
		byte _countOfDigits;
		byte *_indicatorMemory;
		boolean _enabled = true;
		boolean _refreshing = true;
		boolean _showPoint = true;
		boolean _digitPinType;
		void setSegmentsState(byte value); //+
		void insolateDigitPins(); //+
		byte interpretateSymbolToActiveSegments(char __inputSymbol ); //+
		boolean getBitState(byte input, byte bitIndex); //+
};

#endif