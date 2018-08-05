/**
	PatternPlayer_h - библиотека, позволяющая воспроизводить различные заранее заданные последовательности частот и интервалов.
	Для работы проигрывателя, требуются заранее заданные последовательности (мелодии). Можно воспроизводить мелодии циклично, либо задавать 
	максимальное время, в течение которого может быть воспроизведена мелодия.
	О способе создания и настройки последовательности написано в файле SignalPattern.h
	Данная библиотека использует первый таймер для задачи частоты ШИМ на 9 и 10 ноге. На всех остальных ногах частота изменяться не будет.
	При создании указывается:
		* Метод с двумя параметрами целочисленного типа. Первый параметр - частота, второй - продолжительность воспроизведения. Этот метод вызывается тогда, когда необходимо
			воспроизвести ноту.
		* Метод без параметров, котрый будет выключать воспроизведение звука.
		* Интервал между вызовами метода processStep().
	В параллельном потоке нужно циклически вызывать метод processStep().
	Для воспроизведения можно использовать несколько способов:
		* Вызов метода play, в параметры которого нужно обязательно передать целочисленный массив с частотами, целочисленный массив с длительностями и их длину.
			Остальные два необязательных параметра - количество повторов и максимальная длительность воспроизведения. Если количество повторов = 0, то сигнал не воспроизводится.
			Если максимальная длительность = 0, то сигнал воспроизводится, пока закончится паттерн и количество повторов.
		* Вызов метода play c передачей ему в параметры отформатированную строку с данными. Её, в свою очередь, можно получить с помощью метода convertInpMelodyToStr, 
			передав ему два массива с частотами нот и их длительностью и их длину.
		Если частота ноты = 0, то будет воспроизводиться тишина.
	Можно остановить воспроизведение, вызвав метод stop(). После этого можно запустить мелодию методом play(). При этом воспроизведение начнётся с нуля.
	Приостановить воспроизведение можно методом pause(). При этом, его можно будет продолжить с той же точки, используя метод play().
	Написано за один вечер. ExtNeon. 08.11.2017
*/

// проверка, что библиотека еще не подключена
#ifndef PatternPlayer_h // если библиотека не подключена
#define PatternPlayer_h // тогда подключаем ее

#include "Arduino.h"
//#include "SignalPattern.h"


#define PLAYING 1
#define PAUSED 2
#define STOPPED 0
#define DEV_PLAYER_STOPPED_PWM_VAL 0

class PatternPlayer {
	public:
		PatternPlayer(void (*toneFunc) (int frequency, int duration), void (*noToneFunc) (), unsigned int timerInterval);
		void play(int freqArray[], int durationArray[], int arrayLength, int repeatationCount = 1, unsigned int duration = 0);
		void play(String inputMelody, int repeatationCount = 1, unsigned int duration = 0);
		void play();
		void pause();
		void stop();
		byte getState();
		void processStep();
		void processStepMs(unsigned int ms);
	    static String convertInpMelodyToStr(int *freqArr, int *durationArr, int arrLength);
	private:
		unsigned int dev_stepCounter;
		unsigned int dev_currentStepIndex;
		unsigned int dev_timerInterval;
		unsigned int max_duration;
		unsigned int max_duration_timer;
		int dev_repeatationCount;
		int dev_repeatCounter;
		byte dev_currentState;
		int *dev_freqArray;
		int *dev_durationArray;
		int dev_arrayLength;
		boolean dev_notInitialized;
		void (*_toneFunc) (int frequency, int duration);
		void (*_noToneFunc) ();
		void dev_processStepOnly();
		void moveStep(unsigned int moveToMillis);
		void freeArrays();
		boolean newStep;
};

#endif