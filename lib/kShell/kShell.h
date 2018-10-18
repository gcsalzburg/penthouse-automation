/*
 * Shell.h
 *
 *  Created on: 24. Mai 2016
 *      Author: a.poelzleithner
 */

#ifndef SHELL_H_
#define SHELL_H_

#include "Arduino.h"
#include "WString.h"


#define CLI_INBUFFER_SIZE 30
#define CLI_PROMPT "KISKA>"
#define TOKEN_IN_LINE 5


typedef void(*clbk)(uint8_t argc, char *argv[]);
typedef struct
{
	char *pCmd;
	clbk pCmdClbk;
	char *pDescription;
}cmd_t;

void cli_init(void);
void cli_PutChar(char c);
void cli_RegisterCmdList(cmd_t* cmdtable, uint16_t count);


#define REGISTER_COMMANDLIST(c) (cli_RegisterCmdList(c, (sizeof(c)/sizeof(c[0]))))

#endif /* SHELL_H_ */