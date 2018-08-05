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

#define BTN_ACTIVE_LOW 0 //Если кнопка считается нажатой тогда, когда уровень сигнала = 0
#define BTN_ACTIVE_HIGH 1 //Если кнопка считается нажатой тогда, когда уровень сигнала = 1
#define BTN_ACTIVE_HIGH_INVERTED 2 //Если кнопка считается нажатой тогда, когда уровень сигнала = 1, но нужен подтягивающий резистор к +5v

#define BTN_DEFAULT_HOLD_TIME 30 //Минимальное время для удержания контактов = 30 миллисекунд.

#define DEV_BTN_MAX_COUNTER_VALUE 960000 //Максимальное время для таймера удержания. Защищает от переполнения.

// проверка, что библиотека еще не подключена
#ifndef HandledButton_h // если библиотека не подключена
#define HandledButton_h // тогда подключаем ее

#include "Arduino.h"

class HandledButton {
	public:
		HandledButton(byte pin, word timerInterval, unsigned long minimalHoldTime = BTN_DEFAULT_HOLD_TIME, byte buttonActiveState = BTN_ACTIVE_LOW);
		void attachHandlerToPushDown(void (*interruptFunc)());
		void attachHandlerToPullUp(void (*interruptFunc)());
		void attachHandlerToChange(void (*interruptFunc)());
		unsigned long getTimeInCurrentState();
		unsigned long getTimeInLastState();
		boolean isPressed();
		boolean isClicked();
		void processStep();
		void processHandlers();
	private:
		void (*pushDownHandler)();
		void (*pullUpHandler)();
		void (*changedHandler)();
		boolean dev_changed;
		boolean dev_lastReadedState;
		boolean dev_currentStableState;
		boolean dev_pushedDown;
		boolean dev_clicked;
		boolean dev_pulledUp;
		boolean dev_buttonActiveState;
		unsigned long dev_minimalHoldTime;
		word dev_timerInterval;
		unsigned long dev_holdStateCounter;
		unsigned long dev_timeInLastState;
		byte _pin;
};

#endif