/*Taylor Madden
CIS 415 Fall 2019

Project 2: MCP v1.0
Launches workload and gets all
processes running together.

Work is all my own, code is inspired
by psuedo-code for launching processes
provided in project description.
*/
//./part1 -ts=5 input.txt

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "header.h"


int main(int argc, char *argv[]){
	initQueue();

	//timeslot needed for part3 and part4
	//not neccessarily req for part1 but helps error handle
	int ts = getTimeslot(argc, argv);
	if (ts < 0){
		printf("Please include a timeslot for error handling \nFor example, type the command: ./p1 -ts=5 input.txt\n");
		exit(1);
	}


	//printf("START\n");
	//process array from commandline or stdin
	getProcesses(argc, argv);
	if(isQueueEmpty()){
		perror("getWorkload(argc, argv)");
		exit(1);
	}
	

	//run each program 	and wait until they are all done
	//printf("before fork all\n");
	forkAll(); //also waits to terminate
	//printf("after fork all\n");

	//free and exit
	deallocQueue();
	//printf("all done\n");
	//exit when done
	exit(0);
}

