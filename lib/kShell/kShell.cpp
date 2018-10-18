/*
 * Shell.cpp
 *
 *  Created on: 24. Mai 2016
 *      Author: a.poelzleithner
 */

#include "kShell.h"

#include "Arduino.h"
#include "WString.h"

#define MYARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))


static char line[CLI_INBUFFER_SIZE];
static char* pLineP;
static char *argv[TOKEN_IN_LINE];
static uint8_t argc;

cmd_t* customcmdtable = 0;
uint16_t customcount = 0;


// Private functions
void cli_ParseLine(char* line);
static void tokenize(char *line);
void cmd_help(uint8_t argc, char *argv[]);




cmd_t help = {"help", 	cmd_help, 	"List of possible functions"};


void cli_RegisterCmdList(cmd_t* cmdtable, uint16_t count){
	customcmdtable = cmdtable;
	customcount = count;
}


void cli_init(void){
	pLineP = line;
	Serial.println( CLI_PROMPT);
}


void cli_PutChar(char c){
	switch (c){
		case '\n':
			/* ignore carriage return */
			break;
		case '\r':
			Serial.println(c);
			*pLineP = '\0';
			cli_ParseLine(line);
			pLineP = line;
			Serial.print(CLI_PROMPT);
			break;
		default:
			Serial.print(c);
			*pLineP++ = c;
			break;
	}
}


// Local functions
static void tokenize(char *line){
	char* pActC;
	uint8_t token = 0;
	uint8_t quotes = 0;
	memset(&argv[0], 0, sizeof(argv)/sizeof(argv[0]));
	argc = 0;
	for(pActC = line; *pActC != '\0'; pActC++){
		if(!token){
			/* search for token start */
			if(*pActC == ' '){
				*pActC = '\0';
			}
			else if(*pActC == '"'){
				quotes = 1;
			}
			else{
				token = 1;
				argv[argc++] = pActC;
			}
		}
		else{
			/* search for token end. if quotes are "open" ignore spaces */
			if(quotes){
				if(*pActC == '"'){
					quotes = 0;
					*pActC = '\0';
				}
			}
			else{
				if(*pActC == ' '){
					token = 0;
					*pActC = '\0';
				}
			}
		}
	}
}


void cli_ParseLine(char* line)
{
	uint8_t tblidx = 0;
	uint8_t found = 0;

	tokenize(line);

	if(argc > 0){
		if(strcmp(help.pCmd, argv[0]) == 0){
			help.pCmdClbk(argc, argv);
			return;
		}

		if((customcmdtable != 0) && (customcount != 0)){
			for(tblidx = 0; tblidx < customcount; tblidx++){
				if(strcmp(customcmdtable[tblidx].pCmd, argv[0]) == 0){
					customcmdtable[tblidx].pCmdClbk(argc, argv);
					found = 1;
					continue;
				}
			}
		}
	}
	else{
		return;
	}
	if(!found){
		Serial.print(argv[0]);
		Serial.print(" not found \r\n");
	}
}



void cmd_help(uint8_t argc, char *argv[]){
	int c;
	Serial.print(help.pCmd);
	Serial.print(" - ");
	Serial.println(help.pDescription);

	if((customcmdtable != 0) && (customcount != 0)){
		for(c = 0; c < customcount; c++){
			Serial.print(customcmdtable[c].pCmd);
			Serial.print(" - ");
			Serial.println(customcmdtable[c].pDescription);
		}
	}
}