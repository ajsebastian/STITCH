//Dustin Lapierre
//10.14.2016
//Shell in C
//A shell that creates processes and executes commands

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>

//defining constants
const int TRUE = 1;
const int FALSE = 0;

char *getUserInput();
char **parseUserInput(char* line);
int executeCommand(char** args);
void main_loop();

int main(int argc,char* argv[])
{
	//starts the main loop
	main_loop();

	return EXIT_SUCCESS;
}

void main_loop()
{
	//initialize shared memory address
	//this all needs error checking
	int shm_id;
	key_t key;
	key = 9876;
	shm_id = shmget(key, 128*sizeof(char), IPC_CREAT | 0666);
	char *path = shmat(shm_id, NULL, 0);
	strcpy(path, "/home");

	//used for exit condition
	int status = TRUE;

	//stores user input
	char* line;

	//argument list
	char **args;

	do
	{
		//1. Prompt for input
		printf("> ");

		//2. Read input
		line = getUserInput();

		//3. Parse input
		args = parseUserInput(line);

		//4. Execute command
		status = executeCommand(args);

		free(line);
		free(args);
	}
	while(status);

	int del_shm = shmdt(path);
	shmctl(shm_id, IPC_RMID, NULL);
}

char **parseUserInput(char* line)
{
	//buffer size
	int bufsize = 24;
	//holds arg array position
	int position = 0;
	//list of possible deiliminators
	char *delim = " \t\r\n";
	//holds each single token
	char *token;

	//allocating buffer
	char **args = malloc(bufsize * sizeof(char*));
	if (!args)
	{
		perror("Error allocating space\n");
		exit(EXIT_FAILURE);
	}

	//separating each segment of the line into tokens based on deliminators
	token = strtok(line, delim);
	while (token != NULL)
	{
		args[position] = token;
		position++;

		//if buffer size is exceeded the buffer size is increased
		if (position >= bufsize)
		{
			bufsize *= 2;
			args = realloc(args, bufsize * sizeof(char*));
			if (!args)
			{
				perror("Error allocating space\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, delim);
	}

	//setting the last position after arguments to NULL
	args[position] = NULL;
	return args;


}

char *getUserInput()
{
	//stores user input
	char *line = NULL;
	//used to test how many bytes getline reads in
	int bytes_read;

	//Decalaring buffer variable
	size_t bufsize = 0;

	//Getting user input
	bytes_read = getline(&line, &bufsize, stdin);
	if(bytes_read == -1)
	{
		perror("Error reading input");
	}

	//returning input
	return line;
}

//string concatenation
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

//change directory
int cd(const char *arg)
{

    return 0;
}

//Uses arguments to create a process and load a program into memory
//Notifies parent when program ends
int executeCommand(char** args)
{

	//builtin functions
	if(args[0] == NULL)
	{
		//nothing is entered
		return TRUE;
	}
	else if(strcmp(args[0], "exit") == 0)
	{
		//returns FALSE to tell status to quit
		return FALSE;
	}
	else if(strcmp(args[0], "lilo") == 0)
	{
		//command that prints about info
		printf("STITCH 2.0\n");
		printf("Created by Dustin Lapierre and Albert Sebastian\n");
		printf("Shell That Is Thrilling and Considerably Helpful\n");
		printf("Copyright 10/14/2016\n");
		return TRUE;
	}
	//creating and executing a new process
	int pid = fork();
	if(pid == -1)
	{
		perror("Error creating process\n");
	}
	else if(pid == 0)
	{
		if(execvp(concat("./commands/", args[0]), args) == -1)
		{
			perror("Error loading program into memory");
			exit(EXIT_FAILURE);
		}
	}

	//waiting until child is finished running
	pid_t waitpid;
	waitpid = wait(NULL);
	if(waitpid == -1)
	{
		perror("Error on wait");
		exit(0);
	}

	return TRUE;
}
