#include "Arduino.h"
#include "Small_Cycle.h"
/*Small_Cycle::Small_Cycle(const byte duration) : _duration ( duration)
{
	//_duration = duration;
	started   = false;
	completed = false;
}*/
Small_Cycle::Small_Cycle()
{
	_started   = false;
	_completed = false;
}

byte Small_Cycle::stage()
{
	return _stage;
}
void Small_Cycle::stage(byte stage)
{
	_stage = stage;
}

byte Small_Cycle::duration() {
	return _duration;
}
void Small_Cycle::duration(byte duration) {
	_duration = duration;
}

String Small_Cycle::display() {
	return _display;
}
void Small_Cycle::display(String display) {
	_display = display;
}

bool Small_Cycle::started() {
	return _started;
}
void Small_Cycle::started(bool started) {
	_started = started;
}

bool Small_Cycle::completed() {
	return _completed;
}
void Small_Cycle::completed(bool completed) {
	_completed = completed;
}

unsigned long Small_Cycle::endtime() {
	return _endtime;
}
void Small_Cycle::endtime(unsigned long endtime) {
	_endtime = endtime;
}

void Small_Cycle::setEndtime(unsigned long nowSeconds) {
	_endtime = _duration + nowSeconds;
}
