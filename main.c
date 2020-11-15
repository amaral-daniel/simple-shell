#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <errno.h>
#include "helperMethods.h"

#define MAX_INPUT_LENGTH 15
#define MAX_ARGUMENTS 20
#define MAX_ARGUMENT_LENGTH 15
#define DEBUG_MODE 0

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#   error "Cannot compile for windows"
#elif __APPLE__
//This code was developed using a Macbook Pro 2017, executables seem to be
//divided in /sbin and /bin. For this assignement /sbin will be used.
    #define PROGRAMS_PATH "/sbin/"
#elif __linux__
    #define PROGRAMS_PATH "/bin/"
#elif __unix__
    #define PROGRAMS_PATH "/bin/"
#else
#   error "Unknown compiler"
#endif


bool interrupt;
void signal_handler (int sigNumber)
{
  if (sigNumber == SIGUSR1)
  {
    printf("Recebi um SIGUSR1, pressione Enter para retornar ao início...\n");
    interrupt = true;
  }
}


int main(void)
{
  char command[MAX_INPUT_LENGTH + 1]; // command string
  char fullCommand[MAX_INPUT_LENGTH + 1]; // full command string
  char tempArgument[MAX_ARGUMENT_LENGTH + 2]; //argument input
  char** arguments;//[MAX_INPUT_LENGTH + 2]; // arguments
  char nArgRaw[MAX_ARGUMENTS/10 + 2]; // number of arguments
  int nArg = 0;//number of arguments already converted
  char * pEnd; //pointer in strtol function

  while(1)
  {
    //Make sure interrupt state is false when the shell goes back to step 1
    interrupt = false;
    //STEP 1: GET COMMAND
    //It would be nice to find a way to interrupt without using a global
    //variable....
    if (signal(SIGUSR1, signal_handler) == SIG_ERR)
    {
      printf("Não é possível ligar SIGUSR1\n");
      exit(-1);
    }

    while(1)
    {
      printf("Qual comando quer executar?\n");

      if(interrupt)
        break;

      //STEP 2: COMMAND INPUT
      if(fgets(command,sizeof(command),stdin) != NULL)
        removeNewLine(command);

      //Checks if command exists
      if(!isValidCommand(command,PROGRAMS_PATH))
      {
        printf("Comando %s não encontrado no diretório %s\n",
               command,
               PROGRAMS_PATH);
               continue;
      }
      //Makes sure command string is not too long
      if(strlen(command) > MAX_INPUT_LENGTH)
      {
        printf("O comando excede o comprimento máximo de %i caracteres\n",
               MAX_INPUT_LENGTH);
        continue;
      }
      strcpy(fullCommand,PROGRAMS_PATH);
      strcat(fullCommand,command);

      //input seems valid, we can move on to next step
      break;
    }
    if(interrupt)
      continue;


    ///STEP 3: ARGUMENTS

    while(1)
    {
      printf("Quantos argumentos voce quer digitar?\n");
      fgets(nArgRaw,sizeof(nArgRaw),stdin);
      removeNewLine(nArgRaw);
      printf("Número de argumentos: %s\n",nArgRaw);
      removeNewLine(nArgRaw);

      nArg = strtol(nArgRaw,&pEnd,10);
      if(pEnd != (nArgRaw + strlen(nArgRaw)*sizeof(char)))
      {
        printf("Número %s inválido \n",nArgRaw);
        continue;
      }
      if(nArg > MAX_ARGUMENTS)
      {
        printf("Não é possível utilizar mais do que %i argumentos\n",MAX_ARGUMENTS);
        continue;
      }
      //input seems valid, continue to next step
      break;
    }
    if(interrupt)
      continue;

    //allocate enough space for the pointers
    arguments = (char**) malloc(sizeof(char*)*(nArg+2));
    arguments[0] = command;

    //Get arguments one by one
    for(int i = 1; i < nArg + 1; i++)
    {
      printf("Digite o argumento %i:\n",i);
      fgets(tempArgument,sizeof(tempArgument),stdin);
      removeNewLine(tempArgument);
      arguments[i] = (char*) malloc(sizeof(char)*strlen(tempArgument));
      //maybe check if malloc worked?
      strcpy(arguments[i],tempArgument);
      if(interrupt)
        break;
    }
    //signal received, return to step 1
    if(interrupt)
      continue;

    //last element in argument array has to be null for execv call
    arguments[nArg + 1] = NULL;

  	pid_t childPid = fork();

  	if (childPid == 0)
    {
      execv(fullCommand,
            arguments);
  		return 0;
  	}
  	else
    {
      //wait for child process to finish
  		wait(NULL);
      //After the command is finished the shell can be terminated
  		printf("Task is done\n");
  		return 0;
  	}
  }
}
