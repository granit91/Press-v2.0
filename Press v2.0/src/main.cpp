#include <Arduino.h>
#include "button.h"

#define RELAY_VALVE 2 	// подключение клапана
#define RELAY_DRV_1 3 	// подключение мотора открытие сбросного вентиля
#define RELAY_DRV_2 4 	// подключение мотора открытие сбросного вентиля
#define LS_DRV_CLS 5 	// подключение концевого выключателя положение закрыто 
#define LS_DRV_OPN 6 	// подключение концевого выключателя положение открыто
#define LS_UP 7 		// подключение концевого выключателя верхнее положение
#define LS_DWN 8		// подключение концевого выключателя нижнее положение 
#define BTN_STRT 9    	// подключение кнопки старт
#define BTN_DRV_CLS	10	// кнопка закрытие домкрата
#define BTN_DRV_OPN	11	// кнопка открытие домкрата
#define MANUAL_MODE 12  // ручной режим
#define lamp 13

bool up_posit; // верхняя позиция
bool drv_opn = false; // верхняя позиция
bool drv_cls; // верхняя позиция
bool cond_strtpress; // условие для запуска процедуры прессования
bool press_run; // пресс запущен
bool press_compl; // пресс выполнен
bool flag; // 
bool manual_cls; // ручной режим
bool manual_opn; // ручной режим
bool manual;

int State = 0;
uint32_t tmr2;
uint32_t pause;
uint32_t pause2;

void setup() {
	pinMode(RELAY_VALVE, OUTPUT);
	pinMode(RELAY_DRV_1, OUTPUT);
	pinMode(RELAY_DRV_2, OUTPUT);
	digitalWrite(RELAY_DRV_1, LOW);
	digitalWrite(RELAY_DRV_2, LOW);
	digitalWrite(RELAY_VALVE, LOW);
	pinMode(BTN_STRT, INPUT);
	pinMode(LS_DRV_CLS, INPUT);
	pinMode(LS_DRV_OPN, INPUT);
	pinMode(LS_UP, INPUT);
	pinMode(LS_DWN, INPUT);
	pinMode(BTN_DRV_CLS, INPUT);
	pinMode(BTN_DRV_OPN, INPUT);
	pinMode(MANUAL_MODE, INPUT);
	pinMode(lamp, OUTPUT);
	Serial.begin(115200);
}

bool btn (byte pin) { //обработка кнопок
	uint32_t tmr;
	if (digitalRead(pin) && millis() - tmr >= 200) {
        tmr = millis();
        return true;
      }
	  	return false;
}

void compressor(bool value) { //обработка клапана домкрата 1 - включаем, 0 - отключаем
	if (value) {
		digitalWrite(RELAY_VALVE, HIGH);
	}
	else {
		digitalWrite(RELAY_VALVE, LOW);
	} 
}

void motor(int direct) { // 0 - стоит на месте, 1 - открывается, 2 - закрывается
	if (direct == 1) {
		digitalWrite(RELAY_DRV_1, HIGH);
		digitalWrite(RELAY_DRV_2, LOW);
	}
	else if(direct == 2) {
		digitalWrite(RELAY_DRV_2, HIGH);
		digitalWrite(RELAY_DRV_1, LOW);
	}
	else {
		digitalWrite(RELAY_DRV_1, LOW);
		digitalWrite(RELAY_DRV_2, LOW);
	}
}

void loop() {

	if (btn(MANUAL_MODE)) { //ручной режим
		press_run = false;
		manual = true;
		compressor(0); 
		State = 0; //сброс последовательности
	}
	else { 
		manual = false;
	}

	if (manual) {
		if (btn(BTN_DRV_CLS) && !btn(LS_DRV_CLS)) {
			motor(2);
	}
		else  if (btn(BTN_DRV_OPN) && !btn(LS_DRV_OPN)) {
			motor(1);
	}
		else {
			motor(0);
		}
	}
	
	else {
		switch (State)
		{
		case 0:
			if (btn(LS_DRV_CLS) && btn(LS_DWN) && millis() - pause2 >= 10) {
				State = 1;
			}
			break;

		case 1:

			if (btn(BTN_STRT)) {
				State = 2;
			}
			break;

		case 2:
			compressor(1);
			if (btn(LS_UP)) {
				compressor(0);
				State = 3;
			}
			break;

		case 3:
			motor(1);
			if (btn(LS_DRV_OPN) || btn(LS_DWN)) {
				motor(0);
				pause = millis();
				State = 4;
			}
			break;

		case 4:
			if (btn(LS_DWN) && millis() - pause >= 800) {
				motor(2);
				State = 5;
			}
			break;

		case 5:
			if (btn(LS_DRV_CLS)) {
				motor(0);
				State = 0;
				Serial.print("PRESS_COMPL:");
				pause2 = millis();
			}
			break;
		}
	}
		if(State == 1) digitalWrite(lamp, HIGH); else digitalWrite(lamp, LOW);

		if (millis() - tmr2 >= 500) {
        tmr2 = millis();
		Serial.print("State:");
		Serial.println(State);
		Serial.print("Btn start:");
		Serial.println(btn(BTN_STRT));
		Serial.print("LS UP:");
		Serial.println(btn(LS_UP));
		Serial.print("LS DWN:");
		Serial.println(btn(LS_DWN));
		Serial.print("LS CLOSE:");
		Serial.println(btn(LS_DRV_CLS));
		Serial.print("LS OPEN:");
		Serial.println(btn(LS_DRV_OPN));
		Serial.print("BTN OPN:");
		Serial.println(btn(BTN_DRV_OPN));
		Serial.print("BTN CLS:");
		Serial.println(btn(BTN_DRV_CLS));
		Serial.print("MANUAL:");
		Serial.println(btn(MANUAL_MODE));
		Serial.print("RELAY VALVE:");
		Serial.println(digitalRead(RELAY_VALVE));
		Serial.print("RELAY DRV1:");
		Serial.println(digitalRead(RELAY_DRV_1));
		Serial.print("RELAY DRV2:");
		Serial.println(digitalRead(RELAY_DRV_2));
		Serial.println(" ");
	}
}