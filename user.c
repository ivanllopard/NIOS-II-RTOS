#include "defines.h"
#include "list.h"
#include "user.h"
#include "main.h"

#ifndef NATIVE

#include "kernel.h"

#endif /* NATIVE */

#include <stdlib.h>
#include <assert.h>

uval8 test_stack[10000];

RC SysCall(SysCallType type, uval32 arg0, uval32 arg1, uval32 arg2)
{
  uval32 returnCode;

#ifdef NATIVE
  uval32 sysMode = SYS_ENTER;
  asm volatile("ldw r8, %0\n\t"
	       "ldw r4, %1\n\t"
	       "ldw r5, %2\n\t"
	       "ldw r6, %3\n\t"
	       "ldw r7, %4\n\t"
	       "trap"
	       : : "m" (sysMode), "m" (type), "m" (arg0), "m" (arg1), "m" (arg2)
	       : "r4", "r5", "r6", "r7", "r8");
#else /* NATIVE */
  CreateThread(arg0, arg1, arg2); //Kernel system call - not normally accessible from user space
#endif /* NATIVE */

  asm("stw r4, %0"
  			: "=m" (returnCode));
  //returnCode = RC_SUCCESS; //Change this code to take the actual return value
  return returnCode;
}

ThreadId SysCreateThread(uval32 pc, uval32 stackSize, int priority){
	return SysCall(SYS_CREATE, pc, stackSize, priority);
}

RC SysDestroyThread(ThreadId id){
	return SysCall(SYS_DESTROY, id,  0, 0);
}

RC SysYield(){
	return SysCall(SYS_YIELD,  0,  0, 0);
}

RC SysSuspend(){
	return SysCall(SYS_SUSPEND, 0, 0, 0);
}

RC SysResumeThread(ThreadId id){
	return SysCall(SYS_RESUME, id,  0, 0);
}

RC SysChangePriority(ThreadId id, int priority){
	return SysCall(SYS_PRIORITY, id, (uval32) priority, 0);
}

//Test Yield()
void test(){
	#ifdef NATIVE
	  //asm(".align 4; .global TestThread; TestThread:");
	  uval32 sysMode = SYS_EXIT;
	#endif
	int i;	
	while(1){
		//SysCall(SYS_YIELD, (uval32) test, (uval32) &test_stack, 0);
		
		
		for(i = 0; i < 1000000; i++)
		{}		
		
		myprint("This is a Test!\n");
		//SysYield();
		
	}

}

//Test DestroyThread() and Yield()
void test2(){
	#ifdef NATIVE
	  //asm(".align 4; .global TestThread; TestThread:");
	#endif
	int i;

	for (i = 0; i < 10; i ++)
	{
		//SysCall(SYS_YIELD, (uval32) test, (uval32) &test_stack, 0);
		myprint("This is a Test!\n");
		SysYield();

	}

	SysDestroyThread(0);

}

void mymain()
{
  ThreadId ret;

#ifdef NATIVE
  myprint("begin mymain\n");
  //ret = SysCall(SYS_CREATE, (uval32) test, (uval32) &(test_stack[8000]), 1);
  //ret = SysCall(SYS_SHOWLIST, 0, 0, 0);
  ret = SysCreateThread((uval32) test, (uval32) &(test_stack[10000]), 1);
#endif



  myprint("DONE\n");
  int i;  
  
  while(1){
  
		//myprint("This is #1!\n");
		
		for(i = 0; i < 1000000; i++)
		{} 
		
	    myprint("This is the main!\n");
	    
		//SysYield();
      
    
		
	}
}

//Test suspend() and resumeThread()
void test1(){
	#ifdef NATIVE
	  //asm(".align 4; .global TestThread; TestThread:");
	  uval32 sysMode = SYS_EXIT;
	#endif





	while(1){
		//SysCall(SYS_YIELD, (uval32) test, (uval32) &test_stack, 0);
		myprint("This is a Test!\n");
		SysSuspend();

	}

}

void mymain1()
{
  ThreadId ret;
  int i, j;

#ifdef NATIVE
  myprint("begin mymain\n");
  ret = SysCreateThread((uval32) test1, (uval32) &(test_stack[8000]), 1);
#endif

   char out[2];
	out[0] = ret + 48;
	out[1] = '\n';

  myprint("DONE\n");
   myprint(out);

  while(1){
	    for (i = 0; i < 5; i ++){
	    	myprint("This is the main!\n");
	    	SysYield();
	    }

	    SysResumeThread(ret);
	}
}

/*
 * Test changing priority
 */

void test3(){
	#ifdef NATIVE
	  //asm(".align 4; .global TestThread; TestThread:");
	  uval32 sysMode = SYS_EXIT;
	#endif


	while(1){
		myprint("This is a Test!\n");
		SysYield();

	}

}

void mymain3()
{
  ThreadId ret;
  int i, j;

#ifdef NATIVE
  myprint("begin mymain\n");
  ret = SysCreateThread((uval32) test3, (uval32) &(test_stack[8000]), 2);
#endif



  myprint("DONE\n");
  j = 2;

  while(1){
	    for (i = 0; i < 5; i ++){
	    	myprint("This is the main!\n");
	    	SysYield();
	    }

	    if (j == 2){
	    	j = 1;
	    	SysChangePriority(ret, 1);
	    } else {
	    	j = 2;
	    	SysChangePriority(ret, 2);
	    }
	}
}
