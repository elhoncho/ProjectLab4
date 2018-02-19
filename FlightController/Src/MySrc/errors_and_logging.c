/*
 * logging.c
 *
 *  Created on: Feb 18, 2018
 *      Author: owner
 */

#include <stdio.h>
#include "MyInc/errors_and_logging.h"
#include "MyInc/terminal.h"

#define MIN_LOG_LVL LOG_DEBUG

void LOG(LOG_LVL lvl, char * message){
	if(lvl > MIN_LOG_LVL){
		TerminalWrite(message);
	}
}

void AssertError(char * fileName, int lineNumber){
	char tmpStr[32];
	sprintf(tmpStr, "Error in file :");
	LOG(LOG_ERROR, tmpStr);
	LOG(LOG_ERROR, fileName);
	sprintf(tmpStr, "\r\nOn line: %d\r\n", lineNumber);
	LOG(LOG_ERROR, tmpStr);
}
