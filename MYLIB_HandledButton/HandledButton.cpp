/**
	HandledButton_h - библиотека, позволяющая представлять кнопки, подключённые к определённым пинам, как объекты. 
	К определённым событиям можно подключить обработчики для реакции на них. 
	Для обработки подключённых обработчиков, нужно вызывать метод processHandlers(). Вызывайте его в loop().
	В параллельном потоке нужно циклически вызывать метод processStep(). Например, в обработчике прерывания второго таймера.	
	При создании указывается:
		* Пин, на котором висит кнопка
		* Интервал между вызовами метода processStep().
		* Время, в течение которого кнопка должна оставаться стабильной для смены флага. По умолчанию равен BTN_DEFAULT_HOLD_TIME = 30 миллисекунд (опционально)
		* Тип кнопки, а точнее, в каком состоянии должен находиться выход при активной кнопке. (опционально)
			1) BTN_ACTIVE_LOW - если нормально разомкнутая кнопка подключена к выводу и к земле - по умолчанию, рекомендуется для снижения уровня помех
			2) BTN_ACTIVE_HIGH - если нормально разомкнутая кнопка подключена к выводу и к +5v. Необходимо учесть подтягивающий резистор на землю.
			3) BTN_ACTIVE_HIGH_INVERTED - если нормально ЗАМКНУТАЯ кнопка подключена к выводу и к земле.
	Обработчики могут быть подключены к следующим событиям:
		* Кнопка нажата - attachHandlerToPushDown(void (*interruptFunc)());
		* Кнопка отжата - attachHandlerToPullUp(void (*interruptFunc)());
		* Кнопка поменяла своё состояние - attachHandlerToChange(void (*interruptFunc)());
	В параметры к этим методам нужно передать функцию, которая будет вызываться при выполнении события. Вызов обработчика будет осуществляться из того места, 
	где вызывается метод processHandlers(). 
	Также, можно узнать текущее состояние кнопки методом isPressed(). 
	Если вы хотите узнать, была ли нажата кнопка без использования обработчиков, то используйте метод isClicked(). Он сбрасывает флаг клика, так что вам необходимо сохранять
	результат, если вы хотите использовать его в дальнейшем.
	Если вы хотите расширить функционал (сделать длительные нажатия и прочее), то используйте метод getTimeInCurrentState(). Он вернёт количество времени, в течении
	которого кнопка находится в стабильном состоянии. Также, если вы хотите узнать, сколько времени кнопка была в предыдущем состоянии, используйте метот getTimeInLastState().
	Дописано за один вечер. ExtNeon. 07.12.2017
*/

#include "Arduino.h"
#include "HandledButton.h"



HandledButton::HandledButton(byte pin, word timerInterval, unsigned long minimalHoldTime, byte buttonActiveState) {
	_pin = pin;
	pinMode(_pin, buttonActiveState == 1 ? INPUT : INPUT_PULLUP);
	pushDownHandler = NULL;
	pullUpHandler = NULL;
	changedHandler = NULL;
	dev_changed = false;
	dev_pushedDown = false;
	dev_pulledUp = false;
	dev_lastReadedState = false;
	dev_currentStableState = false;
	dev_clicked = false;
	dev_buttonActiveState = buttonActiveState > 0;
	dev_minimalHoldTime = minimalHoldTime;
	dev_timerInterval = timerInterval;
	dev_holdStateCounter = 0;
	dev_timeInLastState = 0;
}

boolean HandledButton::isPressed() {
	return dev_currentStableState;
}

boolean HandledButton::isClicked() {
	boolean temp = dev_clicked;
	dev_clicked = false;
	return temp;
}

void HandledButton::attachHandlerToPushDown(void (*interruptFunc)()) {
	pushDownHandler = interruptFunc;
}

void HandledButton::attachHandlerToPullUp(void (*interruptFunc)()) {
	pullUpHandler = interruptFunc;
}

void HandledButton::attachHandlerToChange(void (*interruptFunc)()) {
	changedHandler = interruptFunc;
}

void HandledButton::processHandlers() {
	if (dev_pushedDown && pushDownHandler != NULL) {
		pushDownHandler();
		dev_pushedDown = false;
	}
	
	if (dev_pulledUp && pullUpHandler != NULL) {
		pullUpHandler();
		dev_pulledUp = false;
	}
	
	if (dev_changed && changedHandler != NULL) {
		changedHandler();
		dev_changed = false;
	}
}

void HandledButton::processStep() {
	boolean dev_gettedState = digitalRead(_pin);
	dev_gettedState = dev_buttonActiveState ? dev_gettedState : !dev_gettedState;
	
	if (dev_gettedState == dev_lastReadedState) {
		dev_holdStateCounter += dev_timerInterval;
	} else {
		if (dev_holdStateCounter > dev_minimalHoldTime) {
			dev_timeInLastState = dev_holdStateCounter;
		}
		dev_holdStateCounter = 0;
		dev_lastReadedState = dev_gettedState;
	}
	
	if (dev_holdStateCounter >= dev_minimalHoldTime && dev_currentStableState != dev_lastReadedState) {
		dev_changed = true;
		dev_currentStableState = dev_lastReadedState;
		if (dev_currentStableState) {
			dev_pushedDown = true;
		} else {
			dev_pulledUp = true;
			dev_clicked = true;
		}
	}
	
	if (dev_holdStateCounter >= DEV_BTN_MAX_COUNTER_VALUE) {
		dev_holdStateCounter = DEV_BTN_MAX_COUNTER_VALUE;
	}
}

unsigned long HandledButton::getTimeInCurrentState() {
	return dev_holdStateCounter > dev_minimalHoldTime ? dev_holdStateCounter - dev_minimalHoldTime : 0;
}

unsigned long HandledButton::getTimeInLastState() {
	return dev_timeInLastState;
}