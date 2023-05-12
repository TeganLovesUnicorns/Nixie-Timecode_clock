/*
  NIXIE.h - Library for driving a Soviet Era Nixie Driver.
  Created by Tegan Rehbein, May, 2023.
*/
#include <Arduino.h>
#include <NIXIE.h>

#define NIXIE_VERSION           1     // software version of this library


Nixie::Nixie(uint8_t A_pin, uint8_t B_pin, uint8_t C_pin, uint8_t D_pin)
{
    _A_pin = A_pin;
    _B_pin = B_pin;
    _C_pin = C_pin;
    _D_pin = D_pin;
};


void Nixie::setup() {
    pinMode(_A_pin, OUTPUT);
    pinMode(_B_pin, OUTPUT);
    pinMode(_C_pin, OUTPUT);
    pinMode(_D_pin, OUTPUT);
};

void Nixie::write(uint8_t number) {
    int digits [10][4] {
        {0,0,0,0},
        {0,0,0,1},
        {0,0,1,0},
        {0,0,1,1},
        {0,1,0,0},
        {0,1,0,1},
        {0,1,1,0},
        {0,1,1,1},
        {1,0,0,0},
        {1,0,0,1}
    };
};
