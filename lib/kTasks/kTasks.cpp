/*
 * kTasks.cpp
 *
 *  Created on: 4 Jun 2018
 *      Author: g.cave
 */


#include "kTasks.h"
#include "Arduino.h"

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

LoopTimer_t* tasks = 0;
uint16_t numtasks = 0;

// Private functions
void SampleTask(struct LoopTimer* t);

LoopTimer_t sample = {0, 50, 0, SampleTask};


void tasks_init(void){
	// Init here
}

void tasks_RegisterTaskList(LoopTimer_t* tasktable, uint16_t count){
	tasks = tasktable;
	numtasks = count;
}

void tasks_update(){
	// Task handling
	uint32_t curr_ms = millis();
	for(uint16_t i = 0; i < numtasks; i++){
		if(curr_ms - tasks[i].last > tasks[i].delay){
			tasks[i].fire = 1;
			tasks[i].last = curr_ms;
		}
	}
	for(uint16_t i = 0; i < numtasks; i++){
		if(tasks[i].fire){
			tasks[i].callback(&tasks[i]);
			tasks[i].fire = 0;
		}
	}
}


void SampleTask(struct LoopTimer* t){
	Serial.println("Sample task trigger");
}