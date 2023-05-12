#ifndef _NIXIE_H_
#define _NIXIE_H_

#include "Arduino.h"

class Nixie
{
	public:
		Nixie(uint8_t A_pin, uint8_t B_pin, uint8_t C_pin, uint8_t D_pin);
		void setup();
        void write(uint8_t number);

	private:
		uint8_t _A_pin;
		uint8_t _B_pin;
		uint8_t _C_pin;
		uint8_t _D_pin;
};

#endif