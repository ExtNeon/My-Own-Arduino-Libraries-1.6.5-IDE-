/**
	HandledEventTimer_h - библиотека, позволяющая создавать параллельные события с определённым временным интервалом
	В параллельном потоке нужно циклически вызывать метод processStep().
	Для обработки вызовов обработчиков (каламбур), нужно вызывать метод processHandlers(). Вызывайте его в loop().
	При создании указывается:
		* Интервал между вызовами метода processStep().
	После, нужно добавить события. Для этого вызывается метод createEvent(), параметром которого является интервал вызова события и функция - обработчик. Он возвращает ID события.
	Для создания повторяющегося события, необходимо вызывать createRepeatedEvent(). Также, можно изменять режим события методом setRepeatability();
	Текущее состояние события можно проверить с помощью метода getEventState(), которому нужно передать ID события. Он возвращает булевое значение.
	При его вызове, флаг события сбрасывается и вызов обработчика будет пропущен.
	Можно прямо на лету менять интервал, деактивировать определённые события, останавливать весь таймер вообще.
	Написано за один вечер. ExtNeon. 08.11.2017
*/

// проверка, что библиотека еще не подключена
#ifndef EventTimer_h // если библиотека не подключена
#define EventTimer_h // тогда подключаем ее

#include "Arduino.h"

class HandledEvent {
	public:
		boolean flag;
		unsigned long interval;
		unsigned long counter;
		boolean active;
		boolean repeated;
		word maxRepeatCount;
		word repeatationCounter;
		void (*handleProcedure)();
};

class HandledEventTimer {
  private:
	word dev_eventCount;
	boolean enabled;
	HandledEvent *events;
	word dev_timerInterval;
  public:
	HandledEventTimer(word timerInterval);
	word createEvent(unsigned long eventInterval, void (*eventHandler)()); 
	word createRepeatedEvent(unsigned long eventInterval, void (*eventHandler)(), word repeatationCount); //Создать повторяющееся событие.
	void reset(); //Сбрасывает все счётчики событий 
	void stop(); 
	void start();
	void disableEvent(word eventId);
	void enableEvent(word eventId);
	boolean isEventActive(word eventId); 
	boolean isEnabled();
	void setEventInterval(word eventId, unsigned long interval); 
	void setRepeatability(word eventId, boolean repeatIt, word repeatationCount);
	boolean getEventState(word eventId); //Возвращает состояние события, и сбрасывает его. 
	void resetEvent(word eventId); //Сбрасывает текущий таймер события а также его состояние 
	void processStep();
	void processMcsStep(word stepWidthMicros);
	void processStepOptimized();
	void processHandlers(); //Вызывается в лупе.
};

#endif