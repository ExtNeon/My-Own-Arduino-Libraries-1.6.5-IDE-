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

#include "Arduino.h"
#include "SevenSegmentsIndicator.h"

SevenSegmentsIndicator::SevenSegmentsIndicator(byte *segmentPins, byte countOfDigits, byte *digitsPins, boolean digitPinType) {
	_segmentPins = segmentPins;
	_digitPins = digitsPins;
	_countOfDigits = countOfDigits;
	_digitPinType = digitPinType;
	_indicatorMemory = new byte[countOfDigits];
	for (int i = 0; i < countOfDigits; i++) {
		_indicatorMemory[i] = SSI_DIGIT_EIGHT | SSI_ADDITIVE_DOTPOINT;
	}
	for (int i = 0; i < 8; i++) {
		pinMode(_segmentPins[i], OUTPUT);
	}
	insolateDigitPins();
}

SevenSegmentsIndicator::SevenSegmentsIndicator() {}

void SevenSegmentsIndicator::print(String str, boolean shiftToRight) {
	//Serial.println("LAST DIGIT VALUE IS " + String(_indicatorMemory[_countOfDigits - 1]));
	//delay(1000);
	while (str.length() < _countOfDigits) {
		if (shiftToRight) {
			str = ' ' + str;
		} else {
			str = str + ' ';
		}
	}
	word usedDigits = 0;
	int lastStrIndex;
	for (lastStrIndex = 0; lastStrIndex <= str.length() && usedDigits <= _countOfDigits; lastStrIndex++) {
		if (str.charAt(lastStrIndex) != '.') usedDigits++;
	}
	String cuttedStr = str.substring(0, lastStrIndex);
	cuttedStr.toUpperCase();
	//Serial.println("CUTTED STR = " + cuttedStr);
	for (int i = 0; i < _countOfDigits && cuttedStr.length() > 0; i++) {
		_indicatorMemory[i] = interpretateSymbolToActiveSegments(cuttedStr.charAt(0));
		//Serial.println(String(i) + " print cycle iteration. Processing " + cuttedStr.charAt(0) + ". Tranlsated into: " + String(interpretateSymbolToActiveSegments(String(cuttedStr.charAt(0)))));
		
		boolean manyDotsFlag = cuttedStr.charAt(0) == '.';
		cuttedStr = cuttedStr.substring(1, cuttedStr.length());
		if (cuttedStr.length() > 0 && !manyDotsFlag) {
			if (cuttedStr.charAt(0) == '.') {
				_indicatorMemory[i] = _indicatorMemory[i] | SSI_ADDITIVE_DOTPOINT; //Binary OR
				cuttedStr = cuttedStr.substring(1, cuttedStr.length());
			}
		}
	}
	
}

void SevenSegmentsIndicator::displayCustomSymbols(byte* symbols, byte countOfSymbols) {
	if (countOfSymbols > _countOfDigits) {
		countOfSymbols = _countOfDigits;
	}
	byte i;
	for (i = 0; i < _countOfDigits; i++) {
		if (i < countOfSymbols) {
			setDigitValue(i, symbols[i]);
		} else {
			setDigitValue(i, 0);
		}
	}
}

byte SevenSegmentsIndicator::interpretateSymbolToActiveSegments(char __inputSymbol) {
	/*Serial.println("INTERPRETATING " + __inputSymbol);
	delay(200);*/
	if (__inputSymbol == '0') {
		return SSI_DIGIT_ZERO;
	} else if (__inputSymbol == '1') {
		return SSI_DIGIT_ONE;
	} else if (__inputSymbol == '2') {
		return SSI_DIGIT_TWO;
	} else if (__inputSymbol == '3') {
		return SSI_DIGIT_THREE;
	} else if (__inputSymbol == '4') {
		return SSI_DIGIT_FOUR;
	} else if (__inputSymbol == '5') {
		return SSI_DIGIT_FIVE;
	} else if (__inputSymbol == '6') {
		return SSI_DIGIT_SIX;
	} else if (__inputSymbol == '7') {
		return SSI_DIGIT_SEVEN;
	} else if (__inputSymbol == '8') {
		return SSI_DIGIT_EIGHT;
	} else if (__inputSymbol == '9') {
		return SSI_DIGIT_NINE;
	} else if (__inputSymbol == 'A') {
		return SSI_LETTER_A;
	} else if (__inputSymbol == 'B') {
		return SSI_LETTER_B;
	} else if (__inputSymbol == 'C') {
		return SSI_LETTER_C;
	} else if (__inputSymbol == 'D') {
		return SSI_LETTER_D;
	} else if (__inputSymbol == 'E') {
		return SSI_LETTER_E;
	} else if (__inputSymbol == 'F') {
		return SSI_LETTER_F;
	} else if (__inputSymbol == 'G') {
		return SSI_LETTER_G;
	} else if (__inputSymbol == 'H') {
		return SSI_LETTER_H;
	} else if (__inputSymbol == 'I') {
		return SSI_LETTER_I;
	} else if (__inputSymbol == 'J') {
		return SSI_LETTER_J;
	} else if (__inputSymbol == 'L') {
		return SSI_LETTER_L;
	} else if (__inputSymbol == 'N') {
		return SSI_LETTER_N;
	} else if (__inputSymbol == 'O') {
		return SSI_LETTER_O;
	} else if (__inputSymbol == 'P') {
		return SSI_LETTER_P;
	} else if (__inputSymbol == 'Q') {
		return SSI_LETTER_Q;
	} else if (__inputSymbol == 'R') {
		return SSI_LETTER_R;
	} else if (__inputSymbol == 'S') {
		return SSI_LETTER_S;
	} else if (__inputSymbol == 'T') {
		return SSI_LETTER_T;
	} else if (__inputSymbol == 'U') {
		return SSI_LETTER_U;
	} else if (__inputSymbol == 'V') {
		return SSI_LETTER_V;
	} else if (__inputSymbol == 'Y') {
		return SSI_LETTER_Y;
	} else if (__inputSymbol == '-') {
		return SSI_SYMBOLS_MINUS;
	} else if (__inputSymbol == '[') {
		return SSI_SYMBOLS_OPEN_BRACKET;
	} else if (__inputSymbol == ']') {
		return SSI_SYMBOLS_CLOSED_BRACKET;
	} else if (__inputSymbol == '.') {
		return SSI_ADDITIVE_DOTPOINT;
	} else return 0;
}

void SevenSegmentsIndicator::insolateDigitPins() {
	for (int i = 0; i < _countOfDigits; i++) {
		pinMode(_digitPins[i], INPUT);
	}
}

void SevenSegmentsIndicator::setPowerState(boolean enabled) {
	_enabled = enabled;
	if (!enabled) {
		insolateDigitPins();
	}
}

void SevenSegmentsIndicator::setPointShow(boolean enabled) {
	_showPoint = enabled;
}

void SevenSegmentsIndicator::stopRefreshing() {
	_refreshing = false;
}

void SevenSegmentsIndicator::resumeRefreshing() {
	_refreshing = true;
}


boolean SevenSegmentsIndicator::getPowerState() {
	return _enabled;
}

void SevenSegmentsIndicator::setDigitValue(byte digitIndex, byte value) {
	if (digitIndex < _countOfDigits) {
		_indicatorMemory[digitIndex] = value;
	}
}

void SevenSegmentsIndicator::refreshNext() {
	if (_enabled) {
		if (_refreshing){
			if (_currentActiveDigit >= 0) {
				pinMode(_digitPins[_currentActiveDigit], INPUT);
			}
			if (++_currentActiveDigit >= _countOfDigits) _currentActiveDigit = 0;
		}
		pinMode(_digitPins[_currentActiveDigit], OUTPUT);
		setSegmentsState(_indicatorMemory[_currentActiveDigit]);
		digitalWrite(_digitPins[_currentActiveDigit], _digitPinType ? HIGH : LOW);
		//Serial.println("REFRESH. Current active digit = " + String(_digitPins[_currentActiveDigit]) + " writing " + String(_digitPinType));
	}
}

void SevenSegmentsIndicator::setSegmentsState(byte value) {
	for (int currentSegmentPin = 0; currentSegmentPin < 8; currentSegmentPin++) {
		boolean segmentState = getBitState(value, 7 - currentSegmentPin);

		if (_digitPinType) {
			segmentState = !segmentState;
		}
		//Serial.println("SETTING " + String(currentSegmentPin) + " segment state to " + String(segmentState));
		if (currentSegmentPin < 7) {
			digitalWrite(_segmentPins[currentSegmentPin], segmentState ? HIGH : LOW);
		} else {
			if (_showPoint) {		
				digitalWrite(_segmentPins[7], segmentState ? HIGH :LOW);
			} else {
				digitalWrite(_segmentPins[7], _digitPinType ? HIGH : LOW);
			}
		}
	}
}

boolean SevenSegmentsIndicator::getBitState(byte input, byte bitIndex) {
	if (bitIndex >= 8) bitIndex = 7;
	return (input >> bitIndex) & 1u;
}