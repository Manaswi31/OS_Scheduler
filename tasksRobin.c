#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<semaphore.h>
#include<unistd.h>
#include<signal.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include<unistd.h>

#define READY 0x00
#define RUNNING 0x01
#define BLOCKED 0x02
#define TERMINATED 0x03

#define numberOfTasks 3
#define resourceTotal 2
#define numberOfThreads 4
#define quantum 10

struct resourceInfo{
int resourceNumber;
int resourceUse;
int resourceDuration; 
};

struct taskResourceMap{
int taskNumber;
int taskDuration;
struct resourceInfo resourceInfoArray[resourceTotal];
};



struct taskControlBlock{
int taskId;
int taskState; // READY = 0x00; RUNNING = 0x01; BLOCKED=0x02; TERMINATED = 0x03 
int taskDuration;
int remainingTaskDuration;
struct resourceInfo resourceInfoArr[resourceTotal];
pthread_t threadArrayId;
	
};

struct linkedList{

struct taskControlBlock TCB;
struct linkedList *next;
};
typedef struct linkedList node;


void *thread_function(void *arg);
void *timer_function(void *time);
node* pushToReadyQueue(struct taskControlBlock tskCtrlBloc[numberOfThreads]);
void create(node *p);
node *insertAtEnd(node *p);
node *deleteNode(node *head, node *p);
void printList(node *head);
node *sortList(node *head);
void set_timer(void);
void err_sys(const char* x);
void scheduler(node *head, int methodOption, int executeTime);
void loadContext(node *head);
void saveContext(node *head,int executeTime);
void handlerSig(int signo);
void handler(int signo);
void delay(int nsec);

pthread_mutex_t my_mutex;
pthread_cond_t condSignal;
static node *headList;
pthread_t thread[numberOfThreads],threadArray[numberOfThreads];


struct taskResourceMap taskResourceMapArray[numberOfTasks];
struct taskControlBlock tskCtrlBloc[numberOfThreads];

void delay(int nsec)
{
    struct timespec ts;

    ts.tv_sec = nsec;
    ts.tv_nsec = 0;
    nanosleep(&ts, NULL);
}


void *thread_function( void *arg)
{
	
  	int id,sig;
	sigset_t mask;
		 
// 	id = *((int *)arg);
	
struct taskControlBlock *data = (struct taskControlBlock *)arg;

	
/*	pthread_mutex_lock(&my_mutex); //lock mutex
	
	pthread_cond_wait(&condSignal,&my_mutex); */
pthread_t tid;

tid = pthread_self();


	sigfillset(&mask);                /* will unblock all signals */  
   pthread_sigmask(SIG_UNBLOCK, &mask, (sigset_t *)0);

   sigwait(&mask, &sig);

printf("Received sig : %d %u\n", sig, (unsigned int)tid);



   switch(sig) {
     case SIGUSR1:
         handlerSig(sig);
         break;
	case SIGALRM:
	handler(sig);
	break;
     default:   
         break;
   }



//	printf( "Hello from thread %d! \n", data->taskId);
	
// 	pthread_mutex_unlock(&my_mutex); //unlock mutex

	usleep(1);
	
 	pthread_exit( NULL );
	
}

void *timer_function(void *time){

int id;
id = *(int *)time;

printf("Setting alarm in timer thread\n");
set_timer();

for (;;)
        sigpause();
}

void handler(int signo)
{
    int i;
    pthread_t id;

    id = pthread_self();
 printf("thread caught signal %d\n", signo);
	
}



void set_timer(void)
{
    struct itimerval tv;

    tv.it_value.tv_sec = 0;
    tv.it_value.tv_usec = 10;
    tv.it_interval.tv_sec = 0;
    tv.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &tv, NULL) < 0)
        err_sys("can't set timer");
}

void err_sys(const char* x) 
{ 
    perror(x); 
    exit(1); 
}

node*  pushToReadyQueue(struct taskControlBlock tskCtrlBloc[numberOfThreads]){
//pushes all tasks into linkedlist with task status as READY

node *first, *newnode, *head, *temp;

int len=0,i=0;
	first = (node *)malloc(sizeof(node));	 	
	head = (node *)malloc(sizeof(node));
	temp = (node *)malloc(sizeof(node));

	head -> TCB = tskCtrlBloc[i];
	head-> next = NULL;
	first = head;
	printf("first node : %d\n", first->TCB.taskId);
	i++;
while(1)//tskCtrlBloc[i].taskId < numberOfTasks
{
	if(tskCtrlBloc[i].taskId == numberOfTasks)
		break;	
	newnode = (node *)malloc(sizeof(node));
	newnode->TCB  = tskCtrlBloc[i];
	newnode->next = NULL;
	first->next = newnode;
	first = newnode;
	printf("new node :  %d %d %d\n", first->TCB.taskId, newnode->TCB.taskId,i);
	i++;
	
		
} 

newnode = (node *)malloc(sizeof(node));
newnode->TCB  = tskCtrlBloc[i];
newnode->next = NULL;
first->next = newnode;

/*while(head != NULL){
len++;
head = head->next;
} 
printf("Length : %d\n", len); */
printList(head);
temp = sortList(head);
printList(temp);

/* printf("HEad : %d\n", temp->TCB.taskId);
temp = temp->next;
printf("HEad : %d\n", temp->TCB.taskId);*/

return temp;
}

node* createList(node *p){

	//p->TCB = head;
	p->next = NULL;
}
node *insertAtEnd(node *p){


node *new_node, *current, *start,*temp;

new_node = (node *)malloc(sizeof(node));
start = (node *)malloc(sizeof(node));
temp = (node *)malloc(sizeof(node));
start = headList;

if(new_node == NULL)
printf("failed to allocate memory");

new_node = p;

if(start == NULL)

{

	start = new_node;
	current = new_node;

}
else{
	start = start->next;
	temp = start;
	
	while(temp->next->next !=NULL)

	{ temp = temp->next;
	}
	
	
	new_node->next = temp->next;
	
	temp->next = new_node;
	
	
	}
headList = start;
return(start);
}


node *deleteNode(node *head, node *n)
{
    // When node to be deleted is head node
    if(head == n)
    {
        if(head->next == NULL)
        {
            printf("There is only one node. The list can't be made empty ");
            return(head);
        }
 
        /* Copy the data of next node to head */
        head->TCB = head->next->TCB;
 
        // store address of next node
        n = head->next;
 
        // Remove the link of next node
        head->next = head->next->next;
 
        // free memory
        free(n);
 
        return(head);
    }
 else{
 
    // When not first node, follow the normal deletion process
 
    // find the previous node
    node *prev = head;
    while(prev->next != NULL && prev->next != n)
        prev = prev->next;
 printf("Node being removed : %d\n", prev->TCB.taskId);
    // Check if node really exists in Linked List
    if(prev->next == NULL)
    {
        printf("\n Given node is not present in Linked List");
        return(head);
    }
 
    // Remove node from Linked List
    prev->next = prev->next->next;
 	head = prev;
    // Free memory
    free(n);
 
    return(head); 
}
}
 


node *sortList(node *head) {

//sort the list in READY QUEUE in the order of their task durations
//make sure idle task is at the end of the queue even though its task duration is zero

    node *tmpPtr;
    node *tmpNxt;
    node* tmp = NULL;
	tmpPtr = (node *)malloc(sizeof(node));
	tmpNxt = (node *)malloc(sizeof(node));	
	tmp = (node *)malloc(sizeof(node));	

	tmpPtr = head;
	tmpNxt = head->next;
   

    while(tmpNxt->next != NULL){
           while(tmpNxt != tmpPtr){
                    if(tmpNxt->TCB.taskDuration < tmpPtr->TCB.taskDuration){
                            tmp->TCB = tmpPtr->TCB;
                            tmpPtr->TCB = tmpNxt->TCB;
                            tmpNxt->TCB = tmp->TCB;
			printf("I swapped : %d with %d\n", tmpPtr->TCB.taskId, tmpNxt->TCB.taskId);
                    }
                    tmpPtr = tmpPtr->next;
            }
            tmpPtr = head;
            tmpNxt = tmpNxt->next;
    }
         return tmpPtr ; // Place holder 
} 

void printList(node *head){

node  *nextNode, *curr;

	nextNode = (node *)malloc(sizeof(node));
	nextNode = head;
	
if(head == NULL){
printf("Next node: %d\n", head->TCB.taskId);
return;
}

while(1){

if(nextNode == NULL)
break;
printf("Next node : %d\n", nextNode->TCB.taskId);
nextNode = nextNode -> next;

}
 

}


void handlerSig(int signo)
{
    int i=0,count = 0,temp = 0,executeTime = 0;
    pthread_t id;
	node *tempNode;
	node *newnode;
		
		tempNode = (node *)malloc(sizeof(node));
		newnode = (node *)malloc(sizeof(node));
		newnode = headList;
		
	   	id = pthread_self();
	 
	printf("thread %d: caught signal : %u\n", signo, (unsigned int)id);
	
	while(1){

	if(newnode->TCB.threadArrayId == id){
		printf(" task id : %d\n", newnode->TCB.taskId);
		break;
		}
	newnode = newnode->next;
	}
	
	count = temp = quantum;
	while(1)
	{
		if(newnode->TCB.taskId != numberOfThreads){
			count--;
		}
		if(count == 0){
			executeTime = temp;
			scheduler(newnode,1,executeTime);
			break;
		//else{
		//		tskCtrlBloc[i].remaningTaskDuration = temp - count;
		//		break;
		//	}

				
		}
	}


if(newnode->TCB.remainingTaskDuration == 0){//task completed its execution
printf("New node : %d\n", newnode->TCB.taskId);
tempNode = deleteNode(headList,newnode);//delete the node and invoke scheduler to execute the next task in queue
printList(tempNode);
scheduler(tempNode,0,0);//loadContext
}
else{
scheduler(headList, 0,0);//load Context
}
	delay(2);
	//printf("I %d slept after that too !\n", i);

}

void scheduler(node *head,int methodOption, int executeTime){
//load context
//get signal or interrupt
int execTime;

execTime = executeTime;

if(methodOption == 0){	 //load Context
//if(head->TCB.taskState == 0x00 || head->TCB.taskState == 0x01 || head->TCB.taskState == 0x03){ // ready, running or terminated
loadContext(head);
return;
}

//save context
//get signal or interrupt

else{ //save Context
saveContext(head,execTime);
return;
}
}

void loadContext(node *p){


int i = 0, executionTime = 0;

int timer = 0;
sigset_t mask;
struct sigaction act;

node *sigNode;
sigNode = p;

//define the signal handler for thread


 /* set up signal mask to block all in main thread */
   sigfillset(&mask);                /* to turn on all bits */
   pthread_sigmask(SIG_BLOCK, &mask, (sigset_t *)0);

   /* set up signal handlers for SIGINT & SIGUSR1 */
   act.sa_flags = 0;
   act.sa_handler = handler;
   sigaction(SIGALRM, &act, (struct sigaction *)0);
   act.sa_handler = handlerSig;
   sigaction(SIGUSR1, &act, (struct sigaction *)0);
   printf("Task state : %d\n", p->TCB.taskState);


//invoke the process to wake up
if(p->TCB.taskState == 0x00){ //READY
	sigNode->TCB.taskState = 0x01; //RUNNING
	printf("Changed status from %d to %d\n", p->TCB.taskState,sigNode->TCB.taskState );
	

delay(1);		
int error=0;
	if((error = pthread_kill(threadArray[p->TCB.taskId],SIGUSR1)) != 0) 
printf("Signal not sent: %d \n",error);
else
printf("signal sent successfully\n");

	

}


/*** code **/
//consume time until process expires or scheduler removes the process

/*while(1){
if (p->TCB.taskDuration == quantum || p->TCB.remainingTaskDuration == 0)
break; 
i++;
}*/


//get signal to invoke process to wake up

/*** code ***/

//modify the state of the process
/*p->TCB.taskState = 0x00; //Ready

//load process at start of READY queue
int n = 0;
insertAtEnd(p,n); //insert this task at the end of READY queue
sortList(p); //sort the queue according to task durations*/




}
void saveContext(node *p, int executeTime){

int n;
node *insertNode;

insertNode = (node *)malloc(sizeof(node));
//unload running process


//modify the state of the process - task status change,
//update time left

if (p->TCB.taskState = 0x01){ //running
p->TCB.taskState = 0x02; //Blocked
printf("Changed status to : %d\n",p->TCB.taskState);
 
p->TCB.remainingTaskDuration = p->TCB.remainingTaskDuration - executeTime;
printf("Remaining Task duration : %d\n", p->TCB.remainingTaskDuration);

insertNode = insertAtEnd(p);
printList(insertNode);
//place it in appropriate position in READY queue

}
//place the process in waiting queue having process pend on a signal

//create a wait queue with insertion options

//wait for a signal

/***** code *****/
}






int main(){

int t,i,tmp,res,var;
int schedulingName;
char *inputFileName="input.txt";
FILE *fp;
int *token;
pthread_attr_t attr;
sigset_t mask;
struct sigaction act;


int arg[4] = {0,1,2,3};
pthread_t timerThread;

//read the configuration file and store it in required structures

fp = fopen(inputFileName,"r");

if(fp == NULL)
{
	printf("Error in opening File %s\n", inputFileName);
}
else{ //map config file data to structures
		
	t =0;
	token = fscanf(fp,"%d", &taskResourceMapArray[t].taskNumber);
	while(token != NULL)
	{
	
 	token = fscanf(fp,"%d", &taskResourceMapArray[t].taskDuration);
	printf("%d %d\n", taskResourceMapArray[t].taskNumber, taskResourceMapArray[t].taskDuration);
	for(i=0; i< resourceTotal; i++)
	{
		token = fscanf(fp, "%d %d %d", &taskResourceMapArray[t].resourceInfoArray[i].resourceNumber, &taskResourceMapArray[t].resourceInfoArray[i].resourceUse, &taskResourceMapArray[t].resourceInfoArray[i].resourceDuration);
	printf("%d %d %d\n", taskResourceMapArray[t].resourceInfoArray[i].resourceNumber, taskResourceMapArray[t].resourceInfoArray[i].resourceUse, taskResourceMapArray[t].resourceInfoArray[i].resourceDuration);
	}
		t++;
		token = fscanf(fp,"%d", & taskResourceMapArray[t].taskNumber);
	}
	
   fclose(fp);   	
}

printf("Enter the scheduling Method Name\n0-FCFS\n1-Round Robin\n");
scanf("%d", &schedulingName);

printf("Schedule name : %d(0-FCFS 1-Round Robin)\n", schedulingName);


// initialize and set the thread attributes
 	pthread_attr_init( &attr );
 	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
 	pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );


//use mutexes to synchronize threads 

	res = pthread_mutex_init(&my_mutex,NULL);

	if(res!=0)
	{
		perror("Mutex initialisation failed \n");
		exit(EXIT_FAILURE);
	}
//initialise cond variables

	var = pthread_cond_init(&condSignal,NULL);
	
	if(var!=0)
	{
		perror("Condition initialisation failed \n");
		exit(EXIT_FAILURE);
	}

//define the signal handler for the thread


/* set up signal mask to block all in main thread */
  /* sigfillset(&mask);                 to turn on all bits 
   pthread_sigmask(SIG_BLOCK, &mask, (sigset_t *)0);*/

   /* set up signal handlers for SIGINT & SIGUSR1 
   act.sa_flags = 0;
   act.sa_handler = handler;
   sigaction(SIGINT, &act, (struct sigaction *)0);
   act.sa_handler = handlerSig;
   sigaction(SIGUSR1, &act, (struct sigaction *)0); */


//create threads using pthread_create

				
		
		for(i=0; i < numberOfThreads; i++)
		{
			
			//push these threads to READY queue based on the creation times(Round Robin) or their execution times(FCFS)
					
			tskCtrlBloc[i].taskId = i;
			tskCtrlBloc[i].taskState = 0x00; //READY

			if(i == numberOfThreads-1){ //idle task
			
				tskCtrlBloc[i].taskDuration = 0;
				tskCtrlBloc[i].remainingTaskDuration = 0 ;
	
				for(t=0 ; t< resourceTotal; t++){
					tskCtrlBloc[i].resourceInfoArr[t].resourceNumber=0;
					tskCtrlBloc[i].resourceInfoArr[t].resourceUse=0;
					tskCtrlBloc[i].resourceInfoArr[t].resourceDuration=0;
				}
			}	
			else{
	
 				tskCtrlBloc[i].taskDuration = taskResourceMapArray[i].taskDuration;
				tskCtrlBloc[i].remainingTaskDuration = taskResourceMapArray[i].taskDuration ;
				
				for(t=0; t< resourceTotal; t++)
					tskCtrlBloc[i].resourceInfoArr[t] = taskResourceMapArray[i].resourceInfoArray[t];;
			}			
			tmp = pthread_create( &thread[i], &attr, thread_function, &tskCtrlBloc[i]);
 			if ( tmp != 0 )
 			{
 				printf("Creating thread %d failed", i) ; 
 				return 1;
 			}
 	
			
			printf( "Hello from thread %d! %d %d %d %d %u\n", i, tskCtrlBloc[i].taskId,tskCtrlBloc[i].taskState, tskCtrlBloc[i].taskDuration , tskCtrlBloc[i].remainingTaskDuration, (unsigned int)thread[i]);
		threadArray[i] = thread[i];
		tskCtrlBloc[i].threadArrayId = thread[i];
		
			
		}

		headList = NULL;

		node *sampleNode;
				
		sampleNode = (node *)malloc(sizeof(node));	
		sampleNode = headList;
		sampleNode = pushToReadyQueue(tskCtrlBloc);
		
		headList = sampleNode;
		int time = 0;


//create a timer thread

/*tmp = pthread_create(&timerThread, &attr, timer_function, (void *)&time);
 			if ( tmp != 0 )
 			{
 				printf("Creating timer thread failed" ) ; 
 				return 1;
			}

		//sigaction(SIGALRM, &act, NULL);*/
int methodOption = 0,executeTime = 0;// 0 - load context ; 1-save context
		node *saveNode;
		saveNode = (node *)malloc(sizeof(node));
		saveNode = headList;
		scheduler(saveNode,methodOption,executeTime);//load context	

 	// joining threads
 	for ( i=0; i < numberOfThreads; i++ )
 	{
 		tmp = pthread_join(thread[i], NULL );
 		if ( tmp != 0 )
 		{
 			printf("Joining thread %d failed", i) ; 
 			return 1;
 		}
 	}
printf("Bye world\n");
	
	//destroy the attributes and exit
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&my_mutex);
	pthread_cond_destroy(&condSignal);
	pthread_exit(NULL);
 

return 0;
}

