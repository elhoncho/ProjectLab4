/*
 * terminal.c
 *
 *  Created on: Feb 12, 2018
 *      Author: owner
 *
 *   HowTo:  -put the terminalOpen() where it will execute when the terminal is opened
 *           -put the terminalMain() function in the main loop
 *           -make newCmd true when a command has been entered (usually when recieved a cr and or nl)
 */

#include<stdint.h>
#include<string.h>
#include "MyInc\terminal.h"
#include "MyInc\terminalHAL.h"

uint8_t volatile newCmd = 0;
uint8_t debug = 0;
typedef void(*functionPointerType)(char *arg1, char *arg2);
struct commandStruct{
    const char *name;
    functionPointerType execute;
    const char *help;
};

static void CmdClear(char *arg1, char *arg2);
static void ListCommands(char *arg1, char *arg2);
static void ToggelDebug(char *arg1, char *arg2);

const struct commandStruct commands[] ={
    {"clear", &CmdClear, "Clears the screen"},
    {"ls", &ListCommands, "Run Help Function"},
    {"help", &ListCommands, "Run Help Function"},
    {"debug", &ToggelDebug, "Toggels Debug Mode"},
    {"",0,""} //End of commands indicator. Must be last.
};

static void ToggelDebug(char *arg1, char *arg2){
    if(debug){
        debug = 0;
    }
    else{
        debug = 1;
    }
}

static void ListCommands(char *arg1, char *arg2){
    char tmpStr[40];
    uint8_t i = 0;
    while(commands[i].execute){
        strcpy(tmpStr, commands[i].name);
        terminalWrite(tmpStr);
        strcpy(tmpStr, " - ");
        terminalWrite(tmpStr);
        strcpy(tmpStr, commands[i].help);
        terminalWrite(tmpStr);
        strcpy(tmpStr,"\r\n");
        terminalWrite(tmpStr);
        i++;
    }
}
static void CmdClear(char *arg1, char *arg2){
    char tmpStr[25];
    strcpy(tmpStr, "\033[2J\033[;H");
    terminalWrite(tmpStr);
}

void terminalOpen(){
	//TODO: Get this to run when the termial is opened
    char tmpStr[25];
    strcpy(tmpStr, "\033[2J\033[;H");
    terminalWrite(tmpStr);
    strcpy(tmpStr,"Battle Control Online:");
    terminalWrite(tmpStr);
    strcpy(tmpStr,"\r\n>");
    terminalWrite(tmpStr);
}

void termianlClose(){

}

void terminalRead(){
	//TODO: Need to find a better way to control this than the newCmd switch
    if(newCmd){
        char tmpStr[25];
        static uint8_t i = 0;
        char tmpChar;

        char arg[3][22];

        tmpChar = terminalReadRXCharHAL();

        arg[0][0] = '\0';
        arg[1][0] = '\0';
        arg[2][0] = '\0';

        i = 0;
        while(tmpChar != 255){
            uint8_t len = strlen(arg[i]);

            //Dont store \r or \n or space or .
            if(tmpChar != 0x0D && tmpChar != 0x0A && tmpChar != 0x20 && tmpChar != 0x2E){
                arg[i][len] = tmpChar;
                arg[i][len+1] = '\0';
            }
            //space or . => new argument
            else if(tmpChar == 0x20 ||  tmpChar == 0x2E){
                i++;
            }

            tmpChar = terminalReadRXCharHAL();
        }

        if(debug){
            strcpy(tmpStr, "\r\nArg0: \r\n");
            terminalWrite(tmpStr);
            terminalWrite(arg[0]);

            strcpy(tmpStr, "\r\nArg1: \r\n");
            terminalWrite(tmpStr);
            terminalWrite(arg[1]);

            strcpy(tmpStr, "\r\nArg2: \r\n");
            terminalWrite(tmpStr);
            terminalWrite(arg[2]);
        }

        i = 0;
        if(strlen(arg[0]) >= 1){
            while(commands[i].execute){
                if(strcmp(arg[0], commands[i].name) == 0){
                    strcpy(tmpStr, "\r\n");
                    terminalWrite(tmpStr);

                    commands[i].execute(arg[1], arg[2]);
                    strcpy(tmpStr, ">");
                    terminalWrite(tmpStr);
                    i = 0;
                    break;
                }
                i++;
            }
            //i is set to 0 if a command is found
            if(i != 0){
                strcpy(tmpStr, "\r\n");
                terminalWrite(tmpStr);

                strcpy(tmpStr, "No Command Found \r\n>");
                terminalWrite(tmpStr);
            }
        }

        newCmd = 0;
    }
}

void terminalWrite(char *txStr){
	terminalWriteHAL(txStr);
}

void terminalMain(){
    terminalRead();
}
