// File:	rpthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "rpthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
rpthread_t tid = 0; // thread IDs, increment w/ each pthread create call
ucontext_t schedule_context;//(ucontext_t*)malloc(sizeof(ucontext_t)); //scheduler context to init w/ first pthread create call

int check_sch_ctx = 0; //GLOBAL to check if schedule_context has no context (I found out we can't initialize to NULL) -> if 0 then empty
Queue* runQueue = NULL; //GLOBAL for the runQueue that scheduler will grab contexts from
Queue* blockQueue = NULL; // queue of blocked contexts from mutex lock
struct itimerval start; //GLOBAL for timer start value for scheduling
struct itimerval stop;  //GLOABL for timer stop value for scheduling
//initialize current thread TCB
tcb *currentThreadTCB = NULL; //we dont need this anymore...i think..sike we do

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

int isQueueEmpty(Queue* q){
	if(q->front == NULL && q->rear == NULL){
		return 1; //it is empty
	}
	return 0; //it isn't empty
}

qNode* isThread(rpthread_t t, Queue* q){
	printf("in isThread...\n");
	qNode* temp = q->front;
	if(temp == NULL){
		printf("Queue is NULL...\n");
		return NULL;
	}
	do{
		if(temp->data->rpthread_id == t){//then we found the TCB for that thread
			printf("Found thread...\n");
			return temp;
		}else{//else go to next node
			temp = temp->next;
		}
	}while(temp != q->front || temp != NULL);
	printf("Not found...\n");
	return NULL; //thread not found 
}

///////////////////////////////////


/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
    	// create Thread Control Block
       // create and initialize the context of this thread
       // allocate space of stack for this thread to run
       // after everything is all set, push this thread int
       // YOUR CODE HERE
	
	printf("STARTING PTHREAD_CREATE\n\n\n");


	// MAKE CONTEXT---------------------------------------------------------------------------------------------------------------
	printf("step: make context\n");
	//first initialize the TCB - (change) need to malloc space for this tcb struct
	tcb* thread_control_block = (tcb*) malloc(sizeof(tcb));

	//initialize the thread ID
	*thread = tid;
	tid = tid+ 1; // increment GLOBAL thread ID so no 2 threads have same thread ID
	thread_control_block->rpthread_id = *thread;//set thread id in TCB
	printf("This new thread's ID is: %u\n", thread_control_block->rpthread_id);//DEBUG line idk if this works yet...
	thread_control_block->thread_status = READY;//set status in TCB
	printf("about to make context\n");
	if (getcontext(&thread_control_block->context) < 0){//init context, need this or will segfault
		perror("getcontext");
		exit(1);
	}
	//allocate space for the stack
	printf("about to malloc\n");
	void *stack = malloc(STACK_SIZE); //remember to FREE later
	if(stack == NULL){
		perror("Failed to allocate stack");
		exit(1);
	}

	//setup context to be used
	thread_control_block->context.uc_link = NULL;//let runQueue take care of this
	thread_control_block->context.uc_stack.ss_sp = stack;
	thread_control_block->context.uc_stack.ss_size = STACK_SIZE;
	thread_control_block->context.uc_stack.ss_flags = 0;

	makecontext(&thread_control_block->context,(void*)function,1,arg);// could be wrong
	thread_control_block->priority = 0; // prob wrong: default highest
	//setcontext(&thread_control_block->context); LINE USED TO DEBUG AND PROVE PTHREAD CREATE CREATES A CONTEXT THAT CAN BE SWITCHED TO...
	

	// ENQUEUE---------------------------------------------------------------------------------------------------------------
	printf("step: enqueue\n");
	if(runQueue == NULL){
		printf("Made a runqueue...\n");
		runQueue = createQueue();
	}
	enQueue(runQueue,thread_control_block);

	// SCHEDULER--------------------------------------------------------------------------------------------------------------
	/**************

	 * FIRST RUN CASE:
	 * now we must create a scheduler context to swap into when needed
	 * create a context for main and put it in the runQueue itself...but that means make a tcb itself for the main?
	 */
	if(check_sch_ctx == 0){//then create the context, else it already exists and DO NOTHING
		printf("step (once): scheduler\n");

		printf("Scheduler Context not found...now making one!\n");
		check_sch_ctx += 1; //1 means schedule context exists
		start.it_value.tv_sec = 0;
		start.it_value.tv_usec = 0;	
		/*first make scheduler context*/
		if(getcontext(&schedule_context) < 0){
			perror("getcontext");
			exit(1);
		}

		void* sch_stack = malloc(STACK_SIZE);
		if(stack == NULL){
			perror("Failed to allocate stack");
			exit(1);
		}

		schedule_context.uc_link = NULL;
		schedule_context.uc_stack.ss_sp = sch_stack;
		schedule_context.uc_stack.ss_size = STACK_SIZE;
		schedule_context.uc_stack.ss_flags = 0;
		
		makeSchContext();
		printf("Scheduler context [hopefully] properly made...\n");
		
		//now to address the main context...putting it in TCB and queuing it to be swap into in the future
		printf("Now making a main context!\n");
		tcb* main_thread_control_block = (tcb*) malloc(sizeof(tcb));
		main_thread_control_block->rpthread_id = tid;
		tid += 1;
		main_thread_control_block->thread_status = READY;
		
		getcontext(&main_thread_control_block->context);
		enQueue(runQueue, main_thread_control_block);
		printf("Main context made and queued! Swapping context now to scheduler...\n");
		//lastly swap from the main context to the schedule context to start some scheduler work!! Yerrrrrr
		swapcontext(&main_thread_control_block->context, &schedule_context);
		
	}
	printf("FINISHED PTHREAD_CREATE\n\n\n");
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
	printf("rpthread yield called...let go of CPU...move back to scheduler for next job!\n");
	setitimer(ITIMER_PROF, &start, NULL);
	currentThreadTCB->thread_status = READY;
	//send back to scheduler context
	swapcontext(&currentThreadTCB->context, &schedule_context);
	return 0;
};

/* terminate a thread */
void rpthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

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
	printf("STARTING PTHREAD_EXIT\n\n\n");
	printf("step: change status to DONE\n");
	currentThreadTCB->thread_status = DONE;
	
	// restart timer
	setitimer(ITIMER_PROF, &start, NULL);
	printf("FINISH PTHREAD_EXIT: switch back to scheduler\n\n\n");
	setcontext(&schedule_context);
};


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	
	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	//first find given thread
	printf("STARTING PTHREAD_JOIN\n");
	printf("rpthread join called..find thread..wait until it is DONE...continue\n");
	qNode* found_thread = isThread(thread, runQueue);
	if(found_thread == NULL){
		printf("Thread not found!!\n");
		return 1;
	}
	while(found_thread->data->thread_status != DONE){
	}//loop until found_thread is donei

	printf("ENDING PTHREAD_JOIN\n");
	return 0;
};

/* initialize the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex

	// YOUR CODE HERE

	// check mutex
	if(mutex==NULL){
		return -1;
	}

	// DO WE EVEN NEED THIS?
	//mutex = (rpthread_mutex_t*)malloc(sizeof(rpthread_mutex_t));

	// keep track of tcb's of threads waiting next in line
	mutex->curr_thread = NULL;
	mutex->wait_queue = createQueue();
	// keep track if locked or not
	mutex->isLocked = 0;

	return 0;
};

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //  
        // context switch to the scheduler thread

        // YOUR CODE HERE

		// loop stops other threads by switching to scheduler
		// loop doesn't run for first thread
		printf("STARTING LOCK\n\n");
		while(__sync_lock_test_and_set(&(mutex->isLocked),1)==1){
			// add curr thread into mutex's queue
			enQueue(mutex->wait_queue, currentThreadTCB);

			// change status to blocked
			currentThreadTCB->thread_status = BLOCKED;

			// switch to scheduler
			swapcontext(&(currentThreadTCB->context),&(schedule_context));
		}

		// first thread that calls lock only runs this line
		mutex->curr_thread = currentThreadTCB;
        return 0;
};


/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	printf("STARTING UNLOCK\n\n");
	// if not current thread not mutex's curr_thread, return -1
	if(mutex->curr_thread != currentThreadTCB){
		return -1;
	}else{// is mutex's curr_thread and already locked

		// add wait_queue to runqueue

		// first check if there is a wait queue
		if (mutex->wait_queue != NULL && mutex->wait_queue->front!=NULL){
			// while wait_queue's front has a next
			while(mutex->wait_queue->front->next != NULL){
				qNode *tempNode = deQueue(mutex->wait_queue);
				tempNode->data->thread_status = READY;
				enQueue(runQueue,tempNode->data);
			}

		}
	}

		// put mutex into unlocked state
		mutex->isLocked = 0;
		mutex->curr_thread = NULL;


	return 0;
};


/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in rpthread_mutex_init

	return 0;
};

/*the handler function we will switch to when timer switches*/
void schedule_handler(int signum){
	printf("Thread ran out of time. SWITCH!\n");
	swapcontext(&currentThreadTCB->context, &schedule_context); //switch back to scheduler now that the timer has run its timeslice
}//referencing timer.c example code given in the CS416 FAQ Link: https://www.cs.rutgers.edu/~sk2113/course/cs416-sp21/timer.c

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
	/*Let's start by setting the timer to when the schedule_handler is called (I think that's where I'll have the context swap to the next one on the queue?)*/
	//following is referenced from the timer.c code in https://www.cs.rutgers.edu/~sk2113/course/cs416-sp21/timer.c
	printf("STARTING SCHEDULER\n\n\n");
	printf("step (once): setting up timer...\n");
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &schedule_handler;
	sigaction(SIGPROF, &sa, NULL); //using SIGPROF timer instead of SIGREAL...shouldn't cause problems using the practice,c file we tested...
	//use the globally defined 'stop' ITIMER now, when timer stops at TIMESLICE (ms) then it'll trigger scheduler handler?
	stop.it_value.tv_sec = 0; //0 seconds!
	stop.it_value.tv_usec = TIMESLICE*1000; //TIMESLICE (ms) = TIMESLICE * 1000 (us), right? so timer should run for TIMESLICE ms here...
	//run while queue has threads to run!
	while(isQueueEmpty(runQueue) == 0){//FIX LATER
		// schedule policy
		#ifndef MLFQ
			// Choose RR
     			// CODE 1
     			sched_rr();
		#else 
			// Choose MLFQ
     			// CODE 2
     			sched_mlfq();
		#endif

		//ok this is the juicy part for the RR
		setitimer(ITIMER_PROF, &stop, NULL); //set timer and call schedule_handler @ time = stop!
		printf("Swapping context to newly dequeued thread...FINISHED SCHEDULER\n\n\n");
		currentThreadTCB->thread_status = SCHEDULED;
		printf("step: switch back to current thread\n");
		swapcontext(&schedule_context, &currentThreadTCB->context); //now with the timer started, switch to the current thread to give it some runtime!!
		//when timer hit time = stop, then it calls the schedule handler where I will do the next steps...
		//ok so signal handler (or YIELD) sent us back here...next steps ifs to put back the thread into the runqueue, if not DONE
		printf("BACK IN SCHEDULER\n");
		if(currentThreadTCB->thread_status != DONE){
			printf("step: enqueue to runqueue if not done\n");
			currentThreadTCB->thread_status = READY;
			enQueue(runQueue, currentThreadTCB);
		}else if(currentThreadTCB->thread_status == BLOCKED){	
			printf("step: enqueue to blockQueue if blocked\n");
			enQueue(blockQueue,currentThreadTCB);
		}
	}

}

//I think I need to do this since schedule() is static code...
void makeSchContext(){
	makecontext(&schedule_context, (void*) schedule, 0); //makecontext here, but idk whether the function reference is right? If doesn't work try adding an &?
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

	//so jus realized we need a GLOBAL variable to keep track of the current thread that we dequeue...
	//now dequeue a thread into it for the scheduler to swap context to...
	currentThreadTCB = deQueue(runQueue)->data;
	printf("Next TCB dequeued w/ TID:%u\n",currentThreadTCB->rpthread_id);
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE

