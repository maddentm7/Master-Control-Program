#include "header.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

//GLOBAL VARS
#define UNUSED __attribute__((unused))
volatile int USR1_received = 0;
volatile int livingProcesses = 0;
Process *firstProc;
int *childrenPid;
int isP4;
Queue *processQueue;

//QUEUE FUNCTIONALITY LINK FOR QUEUE IMPLEMENTATION INSPIRATION IN REPORT
//initialize queue
void initQueue() {
	processQueue = malloc(sizeof(Queue));//allocate and make sure not empty
	if (processQueue == NULL) {
		perror("initQueue()");
		exit(1);
	}
	processQueue->initialSize = 0; //empty
	processQueue->h = NULL;
	processQueue->t = NULL;
}

void enqueue(Process *p) {
	ProcessNode *new = malloc(sizeof(ProcessNode));
	if (new == NULL) {
		perror("enqueue()");
		exit(1);
	}
	new->process = p;
	new->next = NULL;
	if (processQueue->h == NULL) {
		processQueue->h = new;
		processQueue->t = new;
	}else {
		processQueue->t->next = new;
		processQueue->t = new;
	}
}

Process *dequeue() {
	if (processQueue->h == NULL) {
		perror("dequeue(Queue)");
		exit(1);
	}
	Process *ret = processQueue->h->process;
	ProcessNode *tmp = processQueue->h;
	processQueue->h = processQueue->h->next;
	tmp->next = NULL;
	free(tmp);
	return ret;
}

unsigned int isQueueEmpty(){
	if (processQueue->h == NULL){
		return 1;
	}
	else{
		return 0;
	}
}
int d = 0;
//free queue and processes associated with it (be careful if you popped any off!)
void deallocQueue() {
	while(!isQueueEmpty()) {
		deallocateEndProcess(dequeue());
		d++;
	}
	free(processQueue);
	processQueue = NULL;
}



//******* FUNCTIONS INTRODUCED IN PART 1 + MAY BE USED IN OTHER PARTS AS NEEDED 
//returns index following each \n indicating a start of a new line
int getLineIndex(int file, char buf[], int maxSize) {
    int i;
    char ch;
    for (i = 0; i < maxSize-1; i++) {
        if (read(file, &ch, 1) == 0)
            break;
        buf[i] = ch;
        if (ch == '\n') {
            i++;
            break;
        }
    }
    buf[i] = '\0';
    return i;
}

//returns first occuring index in a string of characters or -1 if not in string
int indexOccursAt(char buf[], char ch) {
    int i=0;
    while (buf[i] != '\0'){
        if (buf[i] == ch){
            return i; //found target ch at index i
	}
        i++;
    }
    return -1; //not found
}

//returns starting of index of a word indicated by being sepereated by spacesOrTabs

//use static variables as alluded to in lab to look up for characters that seperate words
static char *spacesOrTabs = " \t";
static char *apostrophe = "'";
static char *quotation = "\"";
int getWordIndex(char buf[], int i, char word[]) {
    char *temp;
    char *loc;
    //skip any blannks at beginning of buf
    while(indexOccursAt(spacesOrTabs, buf[i]) != -1){
        i++;
    }
    //check to make sure None or only blanks were inputted
    if (buf[i] == '\0'){
        return -1;
    }
    loc = word; //update location
    switch(buf[i]) { //if encounter apostophe quotation or blank break because word has ended
    case '\'': temp = apostrophe; i++; break;
    case '"': temp = quotation; i++; break;
    default: temp = spacesOrTabs; break;
    }
    while (buf[i] != '\0') {
        if (indexOccursAt(temp, buf[i]) != -1)
            break;
        *loc++ = buf[i];
        i++;
    }
    //have broken out or reached end of string buf
    if (buf[i] != '\0' && temp != spacesOrTabs) {
        i++;//skip paste placeholder chars
    }
    *loc = '\0';//pointer at end of string
    return i; 
}

//returns length of a string
int stringLength(char *str) {
    char *index = str;
    while (*index != '\0'){
        index++;
    }
    return index-str;
}

//Duplicate string (allocate new memory) returning pointer to start of new string
char *duplicateString(char *str) {
    int len = stringLength(str);
    char *loc = (char *)malloc(len+1);
    int i;
    if (loc != NULL) {
        for (i = 0; i < len+1; i++)
            loc[i] = str[i];
    }
    return loc;
}

//string to int
int stringToInt(char *str) {
    int ret;
    for (ret = 0;*str<='9' && *str >= '0'; str++){
        ret = ret*10+(int)(*str-'0');
    }
    return ret;
}

//int to string
void intToString(int original, char *buf) {
    char temp[100];
    int n;
    int i;
    int neg;
    static char dig[] = "0123456789";

    if (original == 0) {
        temp[0] = '0';
        i = 1;
    } else {
        if ((n = original) < 0) {
            neg = 1;
            n = -n;
        } else
            neg = 0;
        for (i = 0; n != 0; i++) {
            temp[i] = dig[n % 10];
	    n /= 10;
        }
        if (neg) {
            temp[i] = '-';
            i++;
        }
    }
    while (i-- >= 0){
       *buf++ = temp[i];
    }
    *buf = '\0';
}

//move str1 contents to str2
void moveString(char *str1, char *str2) {
    while ((*str1++ = *str2++) != '\0');
}

//concat str
void concatString(char *str1, char *str2) {
    char *loc;
    for (loc = str1; *loc != '\0'; loc++);
    moveString(loc, str2);
}

//returns 1 if slice up to index i of s1 and s2 are equals otherwise returns 0
int equalSubstring(const char *s1, const char *s2, int n){
    int i;

    for (i = 0; i < n; i++) {
        if (s1[i] != s2[i])
            return 0;	//found unmatching character
    }
    return 1;
}


//in a line or a word remove new line char
void stripNewLine(char word[]){
	int i = 0;
	while (word[i] != '\0'){
		if(word[i] == '\n'){
			word[i] = '\0';
			break;
		}
		i++;
	}
}

//allocate space for process in memory and assign args, numArgs, command, pid, stat
Process *allocateAssignProcess(int numArgs){
	Process *processStruct = (Process *)malloc(sizeof(Process));

	if (processStruct != NULL){
		processStruct->args = (char **) malloc((numArgs+1) * sizeof(char *));

		if (processStruct->args == NULL){//make sure not empty
			free(processStruct);
			return processStruct = NULL;
		}
		processStruct->numArgs = numArgs;
		processStruct->command = NULL;
		processStruct->pid = -1; //no pid yet;
		processStruct->stat = 2;
		//stat 2 == waiting for usr1
		//stat 1 == waiting for cont
		//stat 0 == process is dead	
         }

	return processStruct;
}

int deal = 0;
//enforces deallocation of a process. Used to stop first process in main
void deallocateEndProcess(Process *p){
	deal++;
	//free command
	free(p->command);

	//free args
	int i = 0;
	char *arg;
	while(i <= p->numArgs){
		arg = p->args[i++];
		free(arg);
	}
	free(p->args);
	free(p);
}

//returns a list of child processes
int *forkAll(){
	int n = processQueue->initialSize;
	childrenPid = (int *) malloc(n * sizeof(int));
	if (childrenPid == NULL){
		perror("(int *) malloc(n * sizeof(int))");
		exit(1);
	}

	int i = 0;
	struct timespec frame = {0, 20000000};
	ProcessNode *node = processQueue->h;
	while(node != NULL){
		childrenPid[i] = fork();
		//printf("after fork %d\n", childrenPid[i]);
			if (childrenPid[i] == 0){ //is child
				//wait for sigusr1
				while (! USR1_received){
					(void)nanosleep(&frame, NULL);
				}

				char *program = node->process->command;
				char **args = node->process->args;
				//printf("about to execvp!\n");
				execvp(program, args);

				perror("execvp(program,args)");
				exit(1);
			}
			else if (childrenPid[i] > 0){
				node->process->pid = childrenPid[i];
			}
			else{
				perror("fork()");
				exit(1);
			}
		node = node->next;
		i++;
	}
	return childrenPid;
}
//puts processes into procecess queue sepereating words by whitespace
void loadProcessesIntoQueue(int file){
	int n;
	char buff[512];
	Process *currProcess = NULL;

	//while lines remaining in workfile
	while((n = getLineIndex(file, buff, sizeof(buff))) > 0){

		int numArgs = 0;
		char wordBuff[100];
		char tempBuff[512];
		int i = 0;
		moveString(tempBuff, buff);
		while ((i = getWordIndex(tempBuff, i, wordBuff)) > 0){
			numArgs++;
		} //put number of arguments in buff
		currProcess = allocateAssignProcess(numArgs);//allocate process space
		if (currProcess == NULL){
			perror("allocateAssignProcess(numArgs");
			exit(1);
		}
		char word[64];
		int j = 0;

		getWordIndex(buff, 0, word);
		stripNewLine(word);
		currProcess->command = duplicateString(word);//set command as word
		//get args
		int index = 0;
		while ((j = getWordIndex(buff, j, word)) > 0){
			stripNewLine(word);
			currProcess->args[index++] = duplicateString(word);
		}
		currProcess->args[index] = NULL;

		processQueue->initialSize++;
		enqueue(currProcess);
	}
}
void getProcesses(int argc, char *argv[]){
	char* fname = NULL;
	int f;

	if(argc > 1){//inputted a file
		if (equalSubstring(argv[1], "-", 1)){
			fname = argv[2];
		}
		else if (equalSubstring(argv[2], "-", 1)){
			fname = argv[2];
		}
	}
	if (fname != NULL){
		f = open(fname, 0);
		loadProcessesIntoQueue(f);
	}
	else{
		loadProcessesIntoQueue(0); //stdin
	}
}


//********PART 2 ADDITIONAL IMPLEMENTATION IN PART2.C*************
void startProcesses(int numPrograms){
	//sends USR1 sig to all children
	int i;
	for(i=0; i < numPrograms; i++){
		//printf("START %d\n", childrenPid[i]);
		kill(childrenPid[i], SIGUSR1);
	}
}

void stopProcesses(int numPrograms){
	//suspends all programs
	int i;
	for(i=0; i < numPrograms; i++){
		//printf("STOP %d\n", childrenPid[i]);
		kill(childrenPid[i], SIGSTOP);
	}
}

void continueProcesses(int numPrograms){
	//continues all programs
	int i;
	for(i=0; i < numPrograms; i++){
		//printf("CONTINUE %d\n", childrenPid[i]);
		kill(childrenPid[i], SIGCONT);
	}
}

void waitProcesses(int numPrograms){
	//waits until all programs have completed
	int i;
	for(i=0; i < numPrograms; i++){
		//printf("WAIT %d\n", childrenPid[i]);
		waitpid(childrenPid[i], 0 ,0);
	}
}

//*********FUNCTIONS IMPLEMENTED IN PART 3**********
int getTimeslot(int argc, char *argv[]){
	char *loc = NULL;
	int slot = -1;
	char *argName = "-ts=";
	int argLen = stringLength(argName);

	if ((loc = getenv("USPS_QUANTUM_MSEC")) != NULL){
		slot = stringToInt(loc);
	}
	if (argc > 1) {
		if(equalSubstring(argName, argv[1], argLen)){
			slot = stringToInt(&(argv[1][argLen]));
		}
	}
	else if (argc > 2){
		if(equalSubstring(argName, argv[2], argLen)){
			slot = stringToInt(&(argv[2][argLen]));
		}
	}
	return slot;
}

//sets a timer such that alarm goes off in given timeslot
void setTimer(int seconds){
	struct itimerval v;
	v.it_value.tv_sec = seconds/1000;
	v.it_value.tv_usec = (seconds*1000) % 1000000;
	v.it_interval = v.it_value;
	if (setitimer(ITIMER_REAL, &v, NULL) == -1) {
		perror("setTimer()");
		exit(1);
	}
}

//FUNCTION IMPLEMENTED FOR PART4
void displayKnowledge(Process *p){
	//display information about process about to be scheduled
	int file;
	int n;
	int pid = p->pid;
	char filePath[1024];
	char printval[1024];
	int i= 0;
	char buff1[64];
	char buff2[1024];


	printf("\nPID:\t\t     %d\n", pid); //pretty easy just point to our processes pid (bulit-in from struct)
	char buf[512];
	intToString(pid, buf); //change to string for compatible printf

	//retrieve current command 
	moveString(filePath, "/proc/");
	concatString(filePath, buf); //include entire command
	concatString(filePath, "/cmdline");

	file = open(filePath, 0);
	if((n = getLineIndex(file, printval, sizeof(printval))) > 0){
		printf("COMMAND EXECUTING:  %s\n", printval);
	}

	
	//memory used
	moveString(filePath, "/proc/");
	concatString(filePath, buf);
	concatString(filePath, "/statm");

	file = open(filePath, 0);

	//and I/O
	moveString(filePath, "/proc/");
	concatString(filePath, buf);
	concatString(filePath, "/io");

	file = open(filePath, 0);

	if((n = getLineIndex(file, printval, sizeof(printval))) > 0){
		moveString(buff2, printval);
		if((i = getWordIndex(buff2, 0, buff1)) > 0){
			if((i = getWordIndex(buff2, i, buff1)) > 0){
				stripNewLine(buff1);
				printf("BYTES READ:\t    %s\n", buff1);
			}
		}
	}
	if((n = getLineIndex(file, printval, sizeof(printval))) > 0){
		moveString(buff2, printval);
		if((i = getWordIndex(buff2, 0, buff1)) > 0){
			if((i = getWordIndex(buff2, i, buff1)) > 0){
				stripNewLine(buff1);
				printf("BYTES WRITTEN:      %s\n", buff1);
			}
		}
	}
}


