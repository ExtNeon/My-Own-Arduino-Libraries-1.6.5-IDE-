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
#include "Arduino.h"
#include "HandledEventTimer.h"



HandledEventTimer::HandledEventTimer(word timerInterval) {
  events = NULL;
  enabled = false;
  dev_eventCount = 0;
  dev_timerInterval = timerInterval;
}

word HandledEventTimer::createEvent(unsigned long eventInterval, void (*eventHandler)()) {
  boolean lastEnabledState = enabled;
  enabled = false;
  dev_eventCount++;
  events = (HandledEvent *)realloc(events, sizeof(HandledEvent) * dev_eventCount);
  events[dev_eventCount-1].flag = false;
  events[dev_eventCount-1].interval = eventInterval;
  events[dev_eventCount-1].counter = 0;
  events[dev_eventCount-1].active = true;
  events[dev_eventCount-1].repeated = false;
  events[dev_eventCount-1].maxRepeatCount = 0;
  events[dev_eventCount-1].repeatationCounter = 0;
  events[dev_eventCount-1].handleProcedure = eventHandler;
  enabled = lastEnabledState;
  return dev_eventCount - 1;
}

word HandledEventTimer::createRepeatedEvent(unsigned long eventInterval, void (*eventHandler)(), word repeatationCount) {
	word dev_createdEvent = createEvent(eventInterval, eventHandler);
	setRepeatability(dev_createdEvent, true, repeatationCount);
	return dev_createdEvent;
}

void HandledEventTimer::reset() {
  boolean lastEnabledState = enabled;
  enabled = false;
  for (word i = 0; i < dev_eventCount; i++) {
	resetEvent(i);
  }
  enabled = lastEnabledState;
}

void HandledEventTimer::stop() {
  enabled = false;
}

void HandledEventTimer::start() {
  enabled = true;
}

boolean HandledEventTimer::isEnabled() {
  return enabled;
}

void HandledEventTimer::setEventInterval(word eventId, unsigned long interval) {
  if (eventId >= dev_eventCount) return;
  events[eventId].interval = interval;
}

void HandledEventTimer::resetEvent(word eventId) {
  if (eventId >= dev_eventCount) return;
  events[eventId].active = true;
  events[eventId].counter = 0;
  events[eventId].flag = false;
  events[eventId].repeatationCounter = 0;
}

void HandledEventTimer::disableEvent(word eventId) {
	if (eventId >= dev_eventCount) return;
	events[eventId].active = false;
}

void HandledEventTimer::enableEvent(word eventId) {
	if (eventId >= dev_eventCount) return;
	events[eventId].active = true;
}

void HandledEventTimer::setRepeatability(word eventId, boolean repeatIt, word repeatationCount) {
	if (eventId >= dev_eventCount) return;
	events[eventId].repeated = repeatIt;
	events[eventId].maxRepeatCount = repeatationCount;
}

boolean HandledEventTimer::getEventState(word eventId) {
  if (eventId >= dev_eventCount) return false;
  boolean temp_flag = events[eventId].flag;
  events[eventId].flag = false;
  return temp_flag;
}

boolean HandledEventTimer::isEventActive(word eventId) {
  if (eventId >= dev_eventCount) return false;
  return events[eventId].active;
}

void HandledEventTimer::processStepOptimized() {
  if (!enabled) return;
  for (word i = 0; i < dev_eventCount; i++) {
	if (events[i].active) {
	  if ((events[i].counter += dev_timerInterval) >= events[i].interval ) {
		events[i].counter = 0;
		events[i].flag = true;
		if (!events[i].repeated) {
			events[i].active = false;
		} else {
			if (events[i].maxRepeatCount != 0) {
				if (++events[i].repeatationCounter >= events[i].maxRepeatCount) {
					events[i].active = false;
				}
			}
		}
	  }
	}
  }
}

void HandledEventTimer::processStep() {
  if (!enabled) return;
  for (word i = 0; i < dev_eventCount; i++) {
	if (events[i].active) {
	  if ((events[i].counter += dev_timerInterval) >= events[i].interval) {
		events[i].counter -= events[i].interval;
		events[i].flag = true;
		if (!events[i].repeated) {
			events[i].active = false;
		} else {
			if (events[i].maxRepeatCount != 0) {
				if (++events[i].repeatationCounter >= events[i].maxRepeatCount) {
					events[i].active = false;
				}
			}
		}
	  }
	}
  }
}

void HandledEventTimer::processMcsStep(word stepWidthMicros) {
  if (!enabled) return;
  for (word i = 0; i < dev_eventCount; i++) {
	if (events[i].active) {
	  if ((events[i].counter += stepWidthMicros) >= events[i].interval) {
		events[i].counter -= events[i].interval;
		events[i].flag = true;
		if (!events[i].repeated) {
			events[i].active = false;
		} else {
			if (events[i].maxRepeatCount != 0) {
				if (++events[i].repeatationCounter >= events[i].maxRepeatCount) {
					events[i].active = false;
				}
			}
		}
	  }
	}
  }
}

void HandledEventTimer::processHandlers() {
	for (int i = 0; i < dev_eventCount; i++) {
		if (getEventState(i)) {
			events[i].handleProcedure();
		}
	}
}