#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include "helperMethods.h"

#define MAX_INPUT_LENGTH 15
#define MAX_ARGUMENTS 10
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
    printf("Recebi um SIGUSR1");
    interrupt = true;
  }
}


int main(void)
{
  char command[MAX_INPUT_LENGTH + 1]; //stores command string
  char fullCommand[MAX_INPUT_LENGTH + 1]; //stores full command string
  char tempArgument[MAX_ARGUMENT_LENGTH + 2];
  char** arguments;//[MAX_INPUT_LENGTH + 2]; //stores arguments
  //MAX_ARGUMENTS/10 + 3 should have enough space to store the chars, EOS and
  char nArgRaw[MAX_ARGUMENTS/10 + 3] = {0}; //stores number of arguments
  char * pEnd; //array to store end pointer in strtol function

  while(1)
  {
    printf("Caminho: %s \n",PROGRAMS_PATH);
    interrupt = false;
    //STEP 1: COMMAND
    //TODO: find way to send interrupt variable to signal_handler without
    //global variable
    if (signal(SIGUSR1, signal_handler) == SIG_ERR)
    {
      printf("Não é possível SIGUSR1");
    }

    printf("Qual comando quer executar?\n");

    if(interrupt)
      continue;

    //STEP 2: COMMAND INPUT
    if(fgets(command,sizeof(command),stdin) != NULL)
    {
      removeNewLine(command);
    }

    //Checks if command exists
    if(!isValidCommand(command,PROGRAMS_PATH))
    {
      printf("Comando %s não encontrado no diretório %s\n",
             command,
             PROGRAMS_PATH);
    }
    //Makes sure command string is not too long
    if(strlen(command) > MAX_INPUT_LENGTH)
    {
      printf("O comando excede o comprimento máximo de %i caracteres\n",
             MAX_INPUT_LENGTH);
      exit(-1);
    }
    strcpy(fullCommand,PROGRAMS_PATH);
    strcat(fullCommand,command);


    ///STEP 3: ARGUMENTS
    //MAX_ARGUMENTS/10 + 3 should have enough space to store the chars, EOS and
    //new line character

    printf("Quantos argumentos voce quer digitar?");
    fgets(nArgRaw,sizeof(nArgRaw),stdin);
    if(interrupt)
      continue;

    int nArg = strtol(nArgRaw,&pEnd,10);
    if(pEnd == nArgRaw )
    {
      printf("Número de argumentos inválido: %s\n",nArgRaw);
      interrupt = true;
    }
    if(nArg > MAX_ARGUMENTS)
    {
      printf("Não é possível utilizar mais do que máximo %i argumentos\n",MAX_ARGUMENTS);
      interrupt = true;
    }
    arguments = (char**) malloc(sizeof(char*)*(nArg+2));
    arguments[0] = command;
    for(int i = 1; i < nArg + 1; i++)
    {
      printf("Digite o argumento %i:\n",i);
      fgets(tempArgument,sizeof(tempArgument),stdin);
      removeNewLine(tempArgument);
      printf("tempArgument: %s\n",tempArgument);
      arguments[i] = (char*) malloc(sizeof(char)*strlen(tempArgument));
      //maybe check if malloc worked?
      strcpy(arguments[i],tempArgument);
    }
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

  		wait(NULL);
  		printf("Task is done\n");
  		return 0;
  	}
  }
}
