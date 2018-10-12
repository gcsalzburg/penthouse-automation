/*
 * kTasks.h
 *
 *  Created on: 4 Jun 2018
 *      Author: g.cave
 */

#ifndef KISKA_KTASKS_H_
#define KISKA_KTASKS_H_

#include "Arduino.h"

// Loop timer
typedef struct LoopTimer{
	uint32_t last;		// Last time the function was fired
	uint32_t delay;		// Interval between each call to this task
	uint8_t fire;		// Flag for task handler to determine whether to call or not
	void (*callback)(struct LoopTimer* t);	// Callback function to fire
}LoopTimer_t;


void tasks_init(void);
void tasks_update(void);
void tasks_RegisterTaskList(LoopTimer_t* tasklist, uint16_t count);

#define tasks_register(c) (tasks_RegisterTaskList(c, (sizeof(c)/sizeof(c[0]))))

#endif /* LIB_KISKA_KTASKS_H_ */