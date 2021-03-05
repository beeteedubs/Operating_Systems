// File:	rpthread_t.h

// List all group member's name: Ritvik Biswas, Bryan Zhu
// username of iLab:
// iLab Server:

#ifndef RTHREAD_T_H
#define RTHREAD_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_RTHREAD macro */
#define USE_RTHREAD 1

#ifndef TIMESLICE
/* defined timeslice to 5 ms, feel free to change this while testing your code
 * it can be done directly in the Makefile*/
#define TIMESLICE 5
#endif

#define READY 0
#define SCHEDULED 1
#define BLOCKED 2

/*RITVIK is defining a stack size here. However, I am unsure about this. I referenced makecontext.c for this understanding. Need to confirm with TA...*/
#define STACK_SIZE SIGSTKSZ

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
//I am adding the following libraries
#include <ucontext.h>
#include <signal.h> //stack_t uc_stack is defined here

typedef uint rpthread_t;

typedef struct threadControlBlock {
	/* add important states in a thread control block */
	// thread Id
	// thread status
	// thread context
	// thread stack
	// thread priority
	// And more ...

	// YOUR CODE HERE
	rpthread_t rpthread_id; //following recommendation on proj pdf
	int thread_status; //status set to READY, SCHEDULED, or BLOCKED
	ucontext_t context; //for the context 
	void* stack; // maybe wrong, following examples for now
	//^note from Ritvik: I think we have to save the stack in the context itself. I referenced makecontext.c for this understanding. Need to confirm with TA...
	int priority; //probably wrong
} tcb; 

/* mutex struct definition */
typedef struct rpthread_mutex_t {
	/* add something here */

	// YOUR CODE HERE
} rpthread_mutex_t;

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE
///////////////////////////////////
//Linked List Implementation
//struct for node for the queue
typedef struct qNode
{
	tcb data;
	struct qNode *next;
}qNode;

//struct for the queue itself
typedef struct Queue{
	qNode *front, *rear;
	int size;
}Queue;

//function declrs.
qNode* newNode(tcb data); //creates a new qNode
Queue* createQueue(); //creates empty queue 
void enQueue(Queue* q, tcb data);
qNode* deQueue(Queue* q);

///////////////////////////////////



/* Function Declarations: */

/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int rpthread_yield();

/* terminate a thread */
void rpthread_exit(void *value_ptr);

/* wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr);

/* initial the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex);

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex);

/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex);

#ifdef USE_RTHREAD
#define pthread_t rpthread_t
#define pthread_mutex_t rpthread_mutex_t
#define pthread_create rpthread_create
#define pthread_exit rpthread_exit
#define pthread_join rpthread_join
#define pthread_mutex_init rpthread_mutex_init
#define pthread_mutex_lock rpthread_mutex_lock
#define pthread_mutex_unlock rpthread_mutex_unlock
#define pthread_mutex_destroy rpthread_mutex_destroy
#endif

#endif
