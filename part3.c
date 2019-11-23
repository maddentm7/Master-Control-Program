/*Taylor Madden
CIS 415 Fall 2019

Project 2: MCP v3.0
Launches workload and gets all
processes running together AND MCP schedules proceses

Work is all my own
*/
//./part3 -ts=1 input.txt

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "header.h"
#include <signal.h>

#define UNUSED __attribute__((unused))

Process *firstProc;
int *childrenPid;

static void usr1Sig(UNUSED int sig){
	//wake up!
	switch(sig){
		case(SIGUSR1):
			USR1_received++;
			break;
		default:
			break;
	}
}

static void alarmSig(UNUSED int sig) {
	//alarm on schedule given input

	signal(SIGINT, SIG_IGN);

	//stop firstProc
	kill(firstProc->pid, SIGSTOP);
	enqueue(firstProc);
	int found = 0;
	while(!isQueueEmpty() && !found){
		//get new proc
		firstProc = dequeue();

		//if stat 0, destroy
		if (firstProc->stat == 0){
			deallocateEndProcess(firstProc);
		}

		//else, success
		else{
			found = 1;
		}
	}
	if (isP4){
	//	displayKnowledge(firstProc); wont display in p3
	}
	if (firstProc->stat == 2){
		firstProc->stat = 1;
		kill(firstProc->pid, SIGUSR1);
	}
	else if(firstProc->stat == 1){
		kill(firstProc->pid, SIGCONT);
	}
	signal(SIGINT, SIG_DFL);
}

static void childSig(UNUSED int sig){
	//causes interrupts to be ignored, while
	signal(SIGINT, SIG_IGN);

	//set stat to 0
	pid_t pid;
	int stat;
	isP4 = 0; //DO NOT SHOW INFO FOR PART 3
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		if (WIFEXITED(stat) || WIFSIGNALED(stat)) {
			livingProcesses--;

			if(firstProc->pid == pid ){
				firstProc->stat = 0;
			}
			else{
				//iterate through processQueue and set process stat of pid to 0.
				ProcessNode *cur = processQueue->h;
				while(cur != NULL){
					if (cur->process->pid == pid){
						cur->process->stat = 0;
						break;
					}
					cur = cur->next;
				}
			}

		}
	}
	signal(SIGINT, SIG_DFL);
}


int main(int argc, char *argv[]){
	initQueue();
	isP4=1;

	//get time quantum to run each process for (in seconds)
	int seconds = getTimeslot(argc, argv);
	if (seconds< 0){
		perror("getTimeslot(argc, argv)");
		exit(1);
	}

	//make process array from commandline or stdin
	getProcesses(argc, argv);
	int n = processQueue->initialSize;
	livingProcesses = n;

	if(isQueueEmpty()){
	     perror("initQueue()");
             exit(1);
	}

	//set signal handlers
	if (signal(SIGUSR1, usr1Sig) == SIG_ERR) {
    	    perror("signal(SIGUSR1,usr1Sig)");
    	    exit(1);
        }
        if (signal(SIGALRM, alarmSig) == SIG_ERR) {
    	    perror("signal(SIGALRM, alarmSig)");
            exit(1);
        }
        if (signal(SIGCHLD, childSig) == SIG_ERR) {
    	    perror("signal(SIGCHILD, childSig)");
            exit(1);
        }

	forkAll();
	setTimer(seconds*1000);//convert to MS
	
	//start first process
	firstProc = dequeue();
	kill(firstProc->pid, SIGUSR1);
	struct timespec frame = {0, 10000000};
	while (livingProcesses){
		(void)nanosleep(&frame, NULL);
	}

	//free and exit
	free(childrenPid);
	deallocQueue();
	deallocateEndProcess(firstProc); //
	exit(0);

}




