/*Taylor Madden
CIS 415 Fall 2019

Project 2: MCP v2.0
Launches workload and gets all
processes running together AND implemented way
for MCP to stop all forked child processes
right before they call exec() and a mechanism
to signal a running process to stop

Work is all my own (with guidance of course materials of course)*/
//./part2 -ts=5 input.txt

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include "header.h"
#include <signal.h>
#include <sys/time.h>

int *pidList;

int main(int argc, char *argv[]){

	initQueue();

	//timeslot needed for part3 and part4
	//not neccessarily req for part2 but helps error handle
	int ts = getTimeslot(argc, argv);
	if (ts < 0){
		printf("Please include a timeslot for error handling \nfor example, type the command: ./p2 -ts=5 input.txt\n");
		exit(1);
	}

	//process array from commandline or stdin
	getProcesses(argc, argv);
	int numPrograms = processQueue->initialSize;
	if(isQueueEmpty()){
		perror("getProcesses(argc, argv)");
		exit(1);
	}

	//run and execute all programs
	forkAll();//returns pid's of children

	//PART2 ADDITIONS
	startProcesses(numPrograms);
	stopProcesses(numPrograms);
	startProcesses(numPrograms);
	waitProcesses(numPrograms);

	//free and exit
	deallocQueue();
	free(pidList);
	exit(0);
}
