#pragma once
#include <Arduino.h>

enum State : uint8_t { OFF = 0, ON = 1 };

extern State fanInState, fanExState;
extern State pumpState;
extern State window1State, window2State;

void initRelays();
void relayOn(uint8_t pin);
void relayOff(uint8_t pin);

void setFanIn(State s);
void setFanEx(State s);
void setPump(State s);
void setWindow1(State s);
void setWindow2(State s);
