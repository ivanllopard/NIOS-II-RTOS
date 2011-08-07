#include "defines.h"
#include "list.h"
#include "kernel.h"
#include "main.h"

#include <stdlib.h>
#include <assert.h>

#define Timer0 0x10002000
#define Timer0Status ((volatile short*) (Timer0))
#define Timer0Control ((volatile short*) (Timer0+4))
#define Timer0TimeoutL ((volatile short*) (Timer0+8))
#define Timer0TimeoutH ((volatile short*) (Timer0+12))
#define TIMER_PERIOD 500000000

static void InitTimer();

TD *Active, Kernel;
TD * IDLE;
Stack KernelStack;

int TID = 2;
LL * ReadyQueue, * WaitingQueue, * BlockQueue;
extern TD threadTDs[MAX_THREADTD];
uval8 idle_stack[100];

RC ShowList();

char output[2];

void
InitKernel(void) {
  Active = CreateTD(1);
  InitTD(Active, 0, 0, 1);  //Will be set with proper return registers on context switch
#ifdef NATIVE
  InitTD(&Kernel, (uval32) SysCallHandler, (uval32) &(KernelStack.stack[STACKSIZE]), 0);
  Kernel.regs.sr = DEFAULT_KERNEL_SR;
#endif /* NATIVE */

 ReadyQueue = CreateList(L_PRIORITY);
  WaitingQueue = CreateList(L_WAITING);
  BlockQueue = CreateList(L_LIFO);

  CreateThread(Idle, &idle_stack, LOWESTPRIORITY);

  // begin the time slicing
  InitTimer();
}

static void InitTimer()
{
	myprint("Timer initialized\n");
	// Configure the timeout period to 1 second
	*(Timer0TimeoutL)= 0xffff & TIMER_PERIOD;
	*(Timer0TimeoutH)= TIMER_PERIOD >> 16;
	//Set the interrupt enable bit in the timer’s control register
	*(Timer0Control)=0x5;

#ifdef NATIVE
	//Enable interrupts for timer
	asm ("movi r8 , 0x1");
	asm ("wrctl ctl3, r8");

	//Enable interrupts globally (ctl0)
	asm ("movi r8 , 1");
	asm ("wrctl ctl0, r8");
#endif
}

void Dispatcher(void)
{
	myprint("Dispatcher called\n");
	// clear timeout bit
	//*(Timer0Status) = 0x0;
	//*(Timer0Control)=0x5;

	TD *next = DequeueHead(ReadyQueue);
	if(next == NULL) {
		myprint("Dispatcher: Nothing in ReadyQueue\n");
		return;
	} else if(next == Active) {
		myprint("Dispatcher: Something is wrong\n");
		return;
	}

	char out[2];
	out[0] = next->tid + 48;
	out[1] = '\n';

	myprint("Next TD: ");
	myprint(out);
	Active = next;

	return;
}

void TimerHandle(void){
	myprint("Enter timer handler\n");
	

	PriorityEnqueue(Active, ReadyQueue);
	Dispatcher();
	// clear timeout bit
	*(Timer0Status) = 0x0;
	*(Timer0Control)=0x5;


//	MOVE_ACTIVE_TO_SP;
//	MOVE_ACTIVE_TO_SR;
// 	LOAD_REGS;
// 	MOVE_ACTIVE_TO_PC;
	
#ifdef NATIVE
	//Enable interrupts globally (ctl0)
//	asm ("movi r8 , 1");
//	asm ("wrctl ctl0, r8");
	
  	//asm ( "addi sp,  sp, 116");
  	//asm ( "eret" );
#endif
	return;
}

void K_SysCall( SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2)
{
#ifdef NATIVE
  asm(".align 4; .global SysCallHandler; SysCallHandler:");
  uval32 sysMode = SYS_EXIT;
#endif
	
  uval32 returnCode ;
  switch( type ) {
    case SYS_CREATE:
    	returnCode = CreateThread( arg0, arg1, arg2 ) ;
    	break ;
    case SYS_SHOWLIST:
    	returnCode = ShowList();
    	break;
    case SYS_YIELD:
    	myprint("call yield!\n");
    	returnCode = Yield();
    	break;
    case SYS_DESTROY:
		returnCode = DestroyThread(arg0);
		myprint("call destroy!\n");						
		break;
    case SYS_SUSPEND:		
		returnCode = Suspend();
		break;
    case SYS_RESUME:		
		returnCode = ResumeThread(arg0);		
		break;
    case SYS_PRIORITY:		
		returnCode = ChangeThreadPriority(arg0, arg1);
		myprint("call priority!\n");
		break;
  default:
    myprint("Invalid SysCall type\n");
    returnCode = RC_FAILED;
    break;
  }
#ifdef NATIVE
  asm volatile("ldw r8, %0\n\t"
			  "ldw r4, %1"
			  : : "m" (sysMode), "m" (returnCode): "r8", "r4");
  asm( "trap" );
#endif /* NATIVE */
}

uval32 CreateThread( uval32 pc, uval32 sp, uval32 priority )
{
	//How to create a
	int curId = TID;
	TID ++;

	TD * td = CreateTD(curId);


	InitTD(td, pc, sp-116, priority);
	myprint("CreateThread ");

	//Active = td;
	//return RC_SUCCESS;
	PriorityEnqueue(td, ReadyQueue);
	return td->tid;
}

RC DestroyThread(uval32 tid){
	int i;
	myprint("Enter destroy!\n");
	
	char out[2];
	out[0] = tid + 48;
	out[1] = '\n';
	
	myprint(out);

	if (tid == (uval32) 0 || tid == Active->tid){
		myprint("Enter destroy #2!\n");
		DequeueTD(Active);
		//free memory
		Active->tid = -1;
		Dispatcher();
	} else {
		for (i = 0; i < MAX_THREADTD; i++){
			//get the point of thread from physical queue
			if (threadTDs[i].tid == tid){
				DequeueTD( &(threadTDs[i]) );
				//free memory
				threadTDs[i].tid = -1;
				return RC_SUCCESS;
			}
		}
		return RC_FAILED;
	}

	return RC_SUCCESS;
}

RC Yield(){
	PriorityEnqueue(Active, ReadyQueue);
	myprint("run Yield");
	Dispatcher();
	return RC_SUCCESS;
}

RC Suspend(){
	myprint("call suspend!\n");
	DequeueTD(Active);
	EnqueueAtHead(Active, BlockQueue);	
	Dispatcher();

	return RC_SUCCESS;
}

RC ResumeThread(ThreadId tid){
	myprint("call resume!\n");
	if (tid <= 0 || tid >= MAX_THREADTD){
		return RC_FAILED;
	}

	TD * th = FindTD(tid, BlockQueue);
	if (th == NULL){
		return RC_FAILED;
	}

	DequeueTD(th);
	return PriorityEnqueue(th, ReadyQueue);
}

RC ChangeThreadPriority(ThreadId tid, int newPriority){
	if (tid <= 0 || tid >= MAX_THREADTD){
			return RC_FAILED;
		}

	TD * th = FindTD(tid, BlockQueue);
	if (th != NULL){
		th->priority = newPriority;
		return RC_SUCCESS;
	}

	th = FindTD(tid, ReadyQueue);
	if (th != NULL){
		DequeueTD(th);
		th->priority = newPriority;
		PriorityEnqueue(th, ReadyQueue);
		if (newPriority < Active->priority)
			Yield();
		return RC_SUCCESS;
	}

	return RC_FAILED;
}

RC ShowList(){
	output[1] = 0;
	TD * idle = DequeueHead(ReadyQueue);
	output[0] = 48+idle->tid;
	myprint(output);
	if (idle->tid == 3) {
		myprint("Show list 3 OK\n");

		idle = DequeueHead(ReadyQueue);
		if (idle->tid == 2)
			myprint("Show list 2 OK\n");

	} else {
		myprint("Show list fail\n");
	}


	return RC_SUCCESS;
}

void
Idle()
{

  int i;
  while( 1 )
    {
      myprint( "CPU is idle\n" );
      for( i = 0; i < MAX_THREADS; i++ )
	{
	}
      Yield();
    }
}
