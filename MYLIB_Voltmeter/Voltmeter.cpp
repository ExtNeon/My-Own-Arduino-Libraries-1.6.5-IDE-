/**
	Voltmeter_h - библиотека, позволяющая представить отдельный аналоговый пин, как вольтметр с делителем.
	Имеется внутренний фильтр значений с усреднением, позволяющий получить точный усреднённый результат даже тогда, когда производится измерение нестабильного
	источника напряжения с пульсациями.
	Плюс именно данной библиотеки в том, что вследствие прямой работы с регистрами микроконтроллера, его скорость даже с использованием усреднения весьма высока, 
	а размер при этом сильно уменьшается. Используется функция фонового преобразования, что позволяет измерять напряжение за 104 микросекунды даже при большом
	(до 80) количестве выборок, а при использовании тактовой частоты АЦП 4МГц снижается до 12 микросекунд! При измерении на одну выборку тратится 1,44 микросекунды.
	Однако учтите, что основные операции с фильтрацией производятся в методе getVoltage, поэтому большое количество выборок не только сильно увеличивает размер занимаемой 
	оперативной памяти, но и негативно сказывается на скорости получения напряжения и, отчасти, на скорости его измерения.
	
	При создании указывается:
		* Пин, на котором будет висеть вольтметр.
		* Необязательный параметр - опорное напряжение АЦП
		* Необязательный параметр - сопротивление верхнего резистора делителя
		* Необязательный параметр - сопротивление нижнего резистора делителя
		* Необязательный параметр - количество выборок для стабилизации/фильтрации значений
		* Необязательный параметр - пороговый разброс между выборками. Если 0 - то будет искать одинаковые

	Если при создании указать только пин, вольтметр будет измерять текущее фактическое напряжение на указанном выводе, используя в качестве опорного напряжения 5 вольт.
	Если дополнительно указать и опорное напряжение, то результат будет рассчитываться исходя из него.
	Внимание: переключение регистра выбора опорного напряжения АЦП зависит от выбранного опорного напряжения. Если вы указали 1.1 или 5 вольт, то будут использоваться:
		В первом  случае - внутренний источник опорного напряжения 1.1 вольт
		Во втором случае - напряжение на пине AVcc, который на ардуино по умолчанию подключён к напряжению питания 5 вольт.
		Во всех остальных случаях будет использовано напряжение на пине ARef
	В качестве опорного напряжения лучше устанавливать внутреннее напряжение контроллера (1.1v), так как оно более стабильно.
	СТАНДАРТНАЯ ФУНКЦИЯ АРДУИНО analogReference() НЕ ВЛИЯЕТ НА ДАННУЮ БИБЛИОТЕКУ!!! Это позволяет устанавливать разные опорные напряжения для разных пинов, делая вольтметр ещё более 
	гибким в использовании! Однако, тут есть и минусы:
	Если вы используете несколько вольтметров в одной программе, и у них разные источники опорного напряжения, то в силу особенностей АЦП, первые измерения (порядка 30 раз) после 
	смены источника будут совершенно неточными. Для исправления данного недостатка, в библиотеке реализован автоматический калибратор, но он выключен по умолчанию, так как 
	измерение займёт в 30 раз больше времени!!! Для его включения, вызовите один раз функцию enableREFcalibrationPass(). Вы также можете передать ей количество 
	проходов для калибрации. По умолчанию: 30.
	Включение данной функции сильно замедлит работу вольтметра. Для компенсации потери скорости вы можете увеличить тактовую частоту АЦП.
	
	Если вы хотите изменить скорость преобразования/включить прерывания по завершению преобразования, либо произвести иные манипуляции с регистром ADCSRA, используйте метод
	set_CTRL_STAT_REG_VAL(byte new_ADCSRA_val), принимающий байтовое значение, которое будет записываться в регистр каждый раз при произведении измерения.
	Для удобства, данная библиотека имеет шаблонные значения, позволяющие задать определённую тактовую частоту АЦП:
		ADC_RATE_125KHz - по умолчанию используемое стандартными библиотеками. Время измерения: 208 микросекунд
		ADC_RATE_250KHz - по умолчанию используемое данной библиотекой. Время измерения: 104 микросекунды
		ADC_RATE_500KHz - Время измерения: 56 микросекунд
		ADC_RATE_1MHz - Время измерения: 28 микросекунд
		ADC_RATE_2MHz - Время измерения: 16 микросекунд
		ADC_RATE_4MHz - Время измерения: 12 микросекунд
		ADC_RATE_8MHz - Возможно, не работает. АЦП всё время возвращает 1023. Время измерения: 8 микросекунд
		
	Данные частоты будет верны, если используется 16MHz кварцевый резонатор. При использовании другой тактовой частоты, частота АЦП изменяется пропорционально.
	Примечание: при использовании более высокой тактовой частоты, точность АЦП будет уменьшаться!
	
	Для проведения измерений напряжения большего, чем опорное, необходимо сделать резистивный делитель, а в конструкторе вводятся параметры верхнего и нижнего резистора.
	Можно дополнительно изменить количество выборок. Если необходимо отключить фильтрацию - укажите 1. В дополнение, можно изменить пороговый разброс при фильтрации.
	Также, изменение количества выборок и разброса возможно методом setFilterSamplesCount(byte countOfSamples, byte filterRange). 
	
	Для того, чтобы узнать верхнюю границу вольтметра с определёнными параметрами, используйте эту формулу:
	|-------------------------------|
	|	REF / (R2 / ( R1 + R2 ) )	|
	|-------------------------------|
	Где REF - опорное напряжение, а R1 и R2 - параметры верхнего и нижнего резисторов делителя соответственно.
	Для проведения очередного измерения и учёта его результата в усреднённом напряжении, в цикле/таймере/отдельном потоке вызывается метод processMeasurement()
	Для того, чтобы узнать текущий уровень напряжения на этом выводе, вызывается метод getVoltage().
	Начало положено by ExtNeon. 05.11.2017
*/

#include "Arduino.h"
#include "Voltmeter.h"

#ifndef CyberLib_H

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

void Voltmeter::enableREFcalibrationPass(byte amountOfPasses) {
	_EXP_DEV_ENABLE_CALIBRATION_PASS_AMNT = amountOfPasses;
}

void Voltmeter::AnReadStart() {	  
  ADMUX=_pin;
  if (_EXP_DEV_ENABLE_CALIBRATION_PASS_AMNT) {
	  for (byte i = 0; i < _EXP_DEV_ENABLE_CALIBRATION_PASS_AMNT; i++) {
		  ADCSRA= dev_ctrl_stat_reg;
		  while(isADCReadInProcess());
		  AnReadEnd();
	  }
  }
  ADCSRA= dev_ctrl_stat_reg;
}

uint16_t Voltmeter::AnReadEnd() {
  uint8_t __ADC_RESULT_LOW_BYTE = ADCL;
  uint16_t __ADC_RESULT_HIGH_BYTE = ADCH; 
  return (__ADC_RESULT_HIGH_BYTE<<8) + __ADC_RESULT_LOW_BYTE;
}

bool Voltmeter::isADCReadInProcess() {
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
	byte referenceMask = 0; //Selecting the ref source, mask is two high bits.
	if (ctrl_ref_voltage == 5.) {
		referenceMask = B01000000; //If AVCC, then 01
	} else if (ctrl_ref_voltage == 1.1) {
		referenceMask = B11000000; //If vInt, then 11
	} //Else AREF, 00
	_pin = measurement_Pin - 14 + referenceMask ;
	
}

void Voltmeter::set_CTRL_STAT_REG_VAL(byte new_ADCSRA_val) {
	dev_ctrl_stat_reg = new_ADCSRA_val;
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
	AnReadStart();
	for (word i = 1; i < dev_maxFilterSamplesCount; i++) {
		samples[i] = samples[i - 1];
	}
	while (isADCReadInProcess());
	samples[0] = AnReadEnd();
	dev_changed = true;
}

float Voltmeter::getVoltage() {
	if (dev_changed) {
		dev_lastResult = dev_maxFilterSamplesCount > 1 ? find_similar(samples, dev_maxFilterSamplesCount, _filterRange) : samples[0];
		dev_changed = false;
	}
	return dev_lastResult * dev_transferCoeff;
}