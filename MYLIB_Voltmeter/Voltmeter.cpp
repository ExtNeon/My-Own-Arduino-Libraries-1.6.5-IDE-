/**
	Voltmeter_h - библиотека, позволяющая представить отдельный аналоговый пин, как вольтметр с делителем.
	Имеется внутренний фильтр значений с усреднением, позволяющий получить точный усреднённый результат даже тогда, когда производится измерение нестабильного
	источника напряжения с пульсациями.
	Плюс именно данной библиотеки в том, что вследствие прямой работы с регистрами микроконтроллера, его скорость даже с использованием усреднения весьма высока, 
	а размер при этом сильно уменьшается. Используется функция фонового преобразования, что позволяет измерять напряжение за примерно 116 микросекунд даже при высоком
	(порядка 20 - 30) количестве выборок. Однако учтите, что основные операции с фильтрацией производятся в методе getVoltage, поэтому большое количество выборок не только
	сильно увеличивает размер занимаемой оперативной памяти, но и негативно сказывается на скорости получения напряжения и, отчасти, на скорости его измерения.
	При создании указывается:
		* Пин, на котором будет висеть вольтметр.
		* Необязательный параметр - опорное напряжение АЦП
		* Необязательный параметр - сопротивление верхнего резистора делителя
		* Необязательный параметр - сопротивление нижнего резистора делителя
		* Необязательный параметр - количество выборок для стабилизации/фильтрации значений
		* Необязательный параметр - пороговый разброс между выборками. Если 0 - то будет искать одинаковые

	Если при создании указать только пин, вольтметр будет измерять текущее фактическое напряжение на указанном выводе, используя в качестве опорного напряжения 5 вольт.
	Если дополнительно указать и опорное напряжение, то результат будет рассчитываться исходя из него.
	Для проведения измерений напряжения большего, чем опорное, необходимо сделать резистивный делитель, а в конструкторе вводятся параметры верхнего и нижнего резистора.
	Можно дополнительно изменить количество выборок. Если необходимо отключить фильтрацию - укажите 1. В дополнение, можно изменить пороговый разброс при фильтрации.
	Также, изменение количества выборок и разброса возможно методом setFilterSamplesCount(byte countOfSamples, byte filterRange). 
	В качестве опорного напряжения лучше устанавливать внутреннее напряжение контроллера (1.1v), так как оно более стабильно.
	Для того, чтобы узнать верхнюю границу вольтметра с определёнными параметрами, используйте эту формулу:
	|-------------------------------|
	|	REF / (R2 / ( R1 + R2 ) )	|
	|-------------------------------|
	Где REF - опорное напряжение, а R1 и R2 - параметры верхнего и нижнего резисторов делителя соответственно.
	Для проведения очередного измерения и учёта его результата в усреднённом напряжении, в цикле/таймере/отдельном потоке вызывается метод processMeasurement()
	Для того, чтобы узнать текущий уровень напряжения на этом выводе, вызывается метод getVoltage().
	Написано за один вечер. ExtNeon. 05.11.2017
*/

#include "Arduino.h"
#include "Voltmeter.h"

#ifndef CyberLib_H

void delay_us(uint16_t tic_us) {
	tic_us *= 4; //1us = 4 цикла
	__asm__ volatile 
		  (	
			"1: sbiw %0,1" "\n\t" //; вычесть из регистра значение N
			"brne 1b"				
			: "=w" (tic_us)
			: "0" (tic_us)
		  );
}

//**************Поиск макс повторяющегося элемента в массиве****************************
uint16_t find_similar(uint16_t *buf, uint8_t size_buff, uint8_t range) 
{
 uint8_t maxcomp=0; //счётчик максимального колличества совпадений
 uint16_t mcn=0;	//максимально часто встречающийся элемент массива
 uint16_t comp;	//временная переменная
 range++;	//допустимое отклонение

	for (uint8_t i=0; i<size_buff; i++) 
	{
		comp=buf[i];	//кладем элемент массива в comp
		uint8_t n=0;	//счётчик совпадении
		for (uint8_t j=0; j<size_buff; j++)	{ if (buf[j]>comp-range && buf[j]<comp+range) n++;} // ищем повторения элемента comp в массиве buf	
		if (n > maxcomp) //если число повторов больше чем было найдено ранее
		{
			maxcomp=n; //сохраняем счетяик повторов
			mcn=comp; //сохраняем повторяемый элемент
		}		
	}
 return mcn;
}
#endif



void AnReadStart_250KHz(uint8_t An_pin) {
  ADMUX=An_pin;   
  delay_us(10);	  
  ADCSRA=B11000110;	//B11000111-125kHz B11000110-250kHz  - PRESCALER
}

uint16_t AnReadEnd() {
  uint8_t __ADC_RESULT_LOW_BYTE = ADCL;
  uint16_t __ADC_RESULT_HIGH_BYTE = ADCH; 
  return (__ADC_RESULT_HIGH_BYTE<<8) + __ADC_RESULT_LOW_BYTE;
}

bool isADCReadInProcess() {
	return ADCSRA & (1 << ADSC);
}

Voltmeter::Voltmeter(byte measurement_Pin, float ctrl_ref_voltage, float rdiv_TopResistance, float rdiv_BottomResistance, byte filterCountOfSamples, byte filterRange) {
	setDividerParams(rdiv_TopResistance, rdiv_BottomResistance, ctrl_ref_voltage);
	setFilterSamplesCount(filterCountOfSamples, filterRange);
	byte inputModeByte = B11111110; //14 pin input mode
	byte targetBit = measurement_Pin - 14; //Which bit is need to turn to 0
	while (targetBit-- > 0) { //Shifting bitmask
		shl(inputModeByte); //To the left with cyclic mode
	}
	DDRC &= inputModeByte; //Applying pin mode
	_pin = measurement_Pin + 50; //We need to get a byte with base B01000000 - this is 14 pin inp or DDRC0 in ADMUX mode
	
}

void Voltmeter::setDividerParams(float rdiv_TopResistance, float rdiv_BottomResistance, float ctrl_ref_voltage) {
	dev_transferCoeff = (ctrl_ref_voltage / 1024.) / (rdiv_BottomResistance / (rdiv_TopResistance + rdiv_BottomResistance));
}

void Voltmeter::setFilterSamplesCount(byte countOfSamples, byte filterRange) {
	dev_maxFilterSamplesCount = countOfSamples;
	_filterRange = filterRange;
	dev_maxFilterSamplesCount = dev_maxFilterSamplesCount < 1 ? 1 : dev_maxFilterSamplesCount; //Не допускаем подобных вещей. Хотя...
	samples = realloc(samples, dev_maxFilterSamplesCount * sizeof(word));
	for (byte i = 0; i < dev_maxFilterSamplesCount; i++) {
		samples[i] = 0;
	}
}

void Voltmeter::processMeasurement() {
	/*for (int i = dev_maxFilterSamplesCount - 2; i >= 0; i--) {
		dev_vlmSumValue[i] = dev_vlmSumValue[i+1];
	}*/
	//dev_vlmSumValue[dev_maxFilterSamplesCount-1] = analogRead(_pin);
	AnReadStart_250KHz(_pin);
	for (word i = 1; i < dev_maxFilterSamplesCount; i++) {
		samples[i] = samples[i - 1];
	}
	while (isADCReadInProcess());
	samples[0] = AnReadEnd();
	dev_changed = true;
	
	/*
	if (++dev_countOfMeasures >= dev_maxFilterSamplesCount) {
		short averageADCValue = (dev_measuresSum / dev_countOfMeasures);
		if (dev_maxFilterSamplesCount > 1) {
			dev_sum = (dev_sum + averageADCValue) / 2;
		} else {
			dev_sum = averageADCValue;
		}
		dev_measuresSum = 0;
		dev_countOfMeasures = 0;
		dev_changed = true;
	}*/
}

float Voltmeter::getVoltage() {
	if (dev_changed) {
		dev_lastResult = dev_maxFilterSamplesCount > 1 ? find_similar(samples, dev_maxFilterSamplesCount, _filterRange) : samples[0];
		dev_changed = false;
	}
	/*for (int i = 0; i < dev_maxFilterSamplesCount; i++) {
		_sum += dev_vlmSumValue[i];
	}*/
	//dev_lastResult = dev_sum * dev_transferCoeff;
	return dev_lastResult * dev_transferCoeff;
}