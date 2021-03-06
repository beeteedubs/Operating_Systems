// File:	rpthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "rpthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
rpthread_t tid = 0;
// YOUR CODE HERE

///////////////////////////////////
//Linked List Helper Methods
qNode* newNode(tcb* data){
	qNode* temp = (qNode*)malloc(sizeof(qNode)); 
    temp->data = data; 
    temp->next = NULL; 
    return temp;
}

Queue* createQueue(){
	Queue* q = (Queue*)malloc(sizeof(Queue)); 
    q->front = q->rear = NULL;
	q->size = 0;
    return q; 
} 

void enQueue(Queue* q, tcb* data){ 
    qNode* temp = newNode(data); 
    if (q->rear == NULL) { 
        q->front = q->rear = temp; 
        return; 
    }  
    q->rear->next = temp; 
    q->rear = temp; 
}

qNode* deQueue(Queue* q){
    if (q->front == NULL) 
        return NULL; 
    qNode* temp = q->front; 
    q->front = q->front->next; 
    if (q->front == NULL) 
        q->rear = NULL; 
    return temp; //remember to FREE later
}

//intitalize the runqueue
Queue* runQueue = NULL;
//initialize current thread TCB
//tcb *currentThreadTCB = NULL; we dont need this anymore...i think
///////////////////////////////////


/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
    	// create Thread Control Block
       // create and initialize the context of this thread
       // allocate space of stack for this thread to run
       // after everything is all set, push this thread int
       // YOUR CODE HERE
	printf("pleas work\n");
	//first initialize the TCB - (change) need to malloc space for this tcb struct
	tcb* thread_control_block = (tcb*) malloc(sizeof(tcb));

	//initialize the thread ID
	*thread = tid;
	tid = tid+ 1; // increment GLOBAL thread ID so no 2 threads have same thread ID
	thread_control_block->rpthread_id = *thread;//set thread id in TCB
	thread_control_block->thread_status = READY;//set status in TCB
	printf("about to get context\n");
	if (getcontext(&thread_control_block->context) < 0){//init context, need this or will segfault
		perror("getcontext");
		exit(1);
	}
	//allocate space for the stack
	printf("about ot malloc\n");
	void *stack = malloc(STACK_SIZE); //remember to FREE later
	if(stack == NULL){
		perror("Failed to allocate stack");
		exit(1);
	}

	//setup context to be used
	thread_control_block->context.uc_link = NULL; //ask TA abt this too
	thread_control_block->context.uc_stack.ss_sp = stack;
	thread_control_block->context.uc_stack.ss_size = STACK_SIZE;
	thread_control_block->context.uc_stack.ss_flags = 0;
	printf("About to make context\n");
	makecontext(&(thread_control_block->context),(void*)function,0);//prob wrong to put 0, can't figure out how to put void * args into here, ask TA
	thread_control_block->priority = 0; // prob wrong: default highest
	printf("about to set contest\n");
	//following lines added by Ritvik to address no initial thread being run! Check with Bryan and TA
	//check if runQueue is NULL
	printf("about to do the runqueue stuff for 'create'...\n");
	if(runQueue == NULL){//if runQueue is NULL, no thread is running at the moment, so set the currentThreadTCB to this thread's TCB and set the context to that thread
		printf("Made a runqueue...");
		runQueue = createQueue();
	}
	printf("about to enqueue tcb\n");
	enQueue(runQueue,thread_control_block);
	printf("Done creating the thread and added to runqueue...");
	setcontext(&(deQueue(runQueue)->data->context));

    return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
	
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// wwitch from thread context to scheduler context

	// YOUR CODE HERE
	/********************
	 * (1)  We need to get the current running thread's context and set it's state from RUNNING to READY
	 * (2)  Dequeue a new tcb, enqueue the old tcb, use swapcontext() to swap the current and new contexts (I'm pretty sure swap context saves the stack for us...)
	 * 
	 * But should we do this in this method or call the scheduler to do so.
	 *
	 * I think that the swap should be done in the scheduler and that I should simply just switch to that context
	 */
	
	// get curr context
	qNode* temp_node = deQueue(runQueue);
	temp_node->data->thread_status = READY;

	enQueue(runQueue,temp_node->data);


	return 0;
};

/* terminate a thread */
void rpthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

	// dequeue
	qNode* temp_node = deQueue(runQueue);
	
	// free
	free(temp_node->data->context.uc_stack.ss_sp);
	free(temp_node->data);
	// YOUR CODE HERE
	/********************
	 * So here we need to exit out of the thread
	 * Clean up everything malloced...
	 * - FREE the malloced stack in the tcb context
	 * - FREE the qNode that was going in the runqueue everytime
	 *
	 * So far this is all I could think of...
	 *
	 * The actual switching context, should be done by the scheduler itself - need to figure that out
	 */

};


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	
	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	// check thread is still in queue, if so add this thread

	return 0;
};

/* initialize the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //  
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in rpthread_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// Every time when timer interrup happens, your thread library 
	// should be contexted switched from thread context to this 
	// schedule function

	// Invoke different actual scheduling algorithms
	// according to policy (RR or MLFQ)

	// if (sched == RR)
	//		sched_rr();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// schedule policy
#ifndef MLFQ
	// Choose RR
     // CODE 1
#else 
	// Choose MLFQ
     // CODE 2
#endif

}

/* Round Robin (RR) scheduling algorithm */
static void sched_rr() {
	// Your own implementation of RR
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
	/****************
	 * I'm going to start with a simple FCFS scheduler to test my threads...
	 *
	 *
	 */
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE

