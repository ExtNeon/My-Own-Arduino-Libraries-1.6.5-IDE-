/**
	Voltmeter_h - библиотека, позволяющая представить отдельный аналоговый пин, как вольтметр с делителем.
	Имеется внутренний фильтр значений с усреднением, позволяющий получить точный усреднённый результат даже тогда, когда производится измерение нестабильного
	источника напряжения с пульсациями.
	При создании указывается:
		* Пин, на котором будет висеть вольтметр.
		* Необязательный параметр - опорное напряжение АЦП
		* Необязательный параметр - сопротивление верхнего резистора делителя
		* Необязательный параметр - сопротивление нижнего резистора делителя
		* Необязательный параметр - количество выборок для усреднения значения

	Если при создании указать только пин, вольтметр будет измерять текущее фактическое напряжение на указанном выводе, используя в качестве опорного напряжения 5 вольт.
	Если дополнительно указать и опорное напряжение, то результат будет рассчитываться исходя из него.
	Для проведения измерений напряжения большего, чем опорное, необходимо сделать резистивный делитель, а в конструкторе вводятся параметры верхнего и нижнего резистора.
	Можно дополнительно изменить количество выборок. Если необходимо отключить фильтрацию - укажите 1.
	Также, изменение количества выборок возможно методом setFilterSamplesCount(int countOfSamples). 
	В качестве опорного напряжения лучше устанавливать внутреннее напряжение контроллера, так как оно более стабильно.
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

Voltmeter::Voltmeter(byte measurement_Pin, float ctrl_ref_voltage, float rdiv_TopResistance, float rdiv_BottomResistance, int filterCountOfSamples) {
	setDividerParams(rdiv_TopResistance, rdiv_BottomResistance, ctrl_ref_voltage);
	//dev_vlmSumValue = (short *) malloc(sizeof(short) * midMeasuresCount);
	/*for (int i = 0; i < midMeasuresCount; i++) {
		dev_vlmSumValue[i] = 0;
	}*/
	setFilterSamplesCount(filterCountOfSamples);
	_pin = measurement_Pin;
	pinMode(measurement_Pin, INPUT);
}

void Voltmeter::setDividerParams(float rdiv_TopResistance, float rdiv_BottomResistance, float ctrl_ref_voltage) {
	dev_transferCoeff = (ctrl_ref_voltage / 1024.) / (rdiv_BottomResistance / (rdiv_TopResistance + rdiv_BottomResistance));
}

void Voltmeter::setFilterSamplesCount(int countOfSamples) {
	dev_maxFilterSamplesCount = countOfSamples / 2; //Ускоряем стабилизацию
	dev_maxFilterSamplesCount = dev_maxFilterSamplesCount < 1 ? 1 : dev_maxFilterSamplesCount; //Не допускаем подобных вещей. Хотя...
}

void Voltmeter::processMeasurement() {
	/*for (int i = dev_maxFilterSamplesCount - 2; i >= 0; i--) {
		dev_vlmSumValue[i] = dev_vlmSumValue[i+1];
	}*/
	//dev_vlmSumValue[dev_maxFilterSamplesCount-1] = analogRead(_pin);
	dev_measuresSum += analogRead(_pin);
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
	}
}

float Voltmeter::getVoltage() {
	if (!dev_changed) {
		return dev_lastResult;
	}
	/*for (int i = 0; i < dev_maxFilterSamplesCount; i++) {
		_sum += dev_vlmSumValue[i];
	}*/
	dev_lastResult = dev_sum * dev_transferCoeff;
	dev_changed = false;
	return dev_lastResult;
}