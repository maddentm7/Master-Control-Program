#ifndef _P1FXNS_H_
#define _P1FXNS_H_
//FUNCTION AND STRUCTURE DECLARATION FOR PROJECT 2
//GLOBAL VARS AND STRUCTURE INITIALIZATION
struct queue;
typedef struct queue Queue;
typedef struct process Process;
extern Queue *processQueue;
extern volatile int USR1_received;
extern volatile int livingProcesses;
extern int isP4;

//STRUCTS
struct process{
	char *command;
	char **args;
	int pid;
	int stat;
	int numArgs;
};

typedef struct pNode {
        Process *process;
        struct pNode *next;
} ProcessNode;

struct queue {
	int initialSize;
	ProcessNode *h;
	ProcessNode *t;
};

//FUNCTIONS

//used in part 1 implemented and used as needed throughout project
void initQueue();
void enqueue(Process *p);
Process *dequeue();
unsigned int isQueueEmpty();
void deallocQueue();
int getLineIndex(int file, char buf[], int maxSize);
int indexOccursAt(char buf[], char ch);
int getWordIndex(char buf[], int i, char word[]);
int stringLength(char *s);
char *duplicateString(char *s);
int stringToInt(char *s);
void intToString(int number, char *buf);
void moveString(char *str1, char *str2);
void concatString(char *str1, char *str2);
int equalSubstring(const char *s1, const char *s2, int n);
void stripNewLine(char word[]);
Process *allocateAssignProcess(int numArgs);
void deallocateEndProcess(Process *p);
void loadProcessesIntoQueue(int file);
void getProcesses(int argc, char *argv[]);
int *forkAll();

//solely for part2
void startProcesses(int numPrograms);
void stopProcesses(int numPorgrams);
void continueProcesses(int numPrograms);
void waitProcesses(int numPrograms);

//implemented and used in part3 and part4 (also see p3.c and p4.c for management of signals
int getTimeslot(int argc, char *argv[]);
void displayKnowledge(Process *p);
void setTimer(int quantum);

//implemented for part4
void displayUsage(Process *p);
#endif	/* _P1FXNS_H_ */
