#include "defines.h"
#include "list.h"
#include "main.h"

#include <stdlib.h>

#define COMPARE(a, b) (a < b)

// we don't want to use malloc so we allocate a fixed size array of TDs
TD threadTDs[MAX_THREADTD];
int freeTD = 0; // pointer to the next free thread descriptor

TD *CreateTD(ThreadId tid)
{
  // Check if we have space
  if(freeTD == MAX_THREADTD) {
    myprint("Failed to allocate new thread; reached maximum number allowable\n");
	return NULL;
  }

  TD *thread = &threadTDs[freeTD];

  if(thread != NULL) {
    freeTD++;
    thread->link = NULL;
    thread->tid = tid;
    thread->priority = 0;
    thread->waittime = 0;
    thread->inlist = NULL;
    thread->returnCode = 0;

    thread->regs.pc = 0;
    thread->regs.sp = 0;
    thread->regs.sr = 0;
  } else {
    myprint("Failed to allocate new thread\n");
  }

  return thread;
}

void InitTD(TD *td, uval32 pc, uval32 sp, uval32 priority)
{
  if(td != NULL) {
    td->regs.pc  = pc;
    td->regs.sp = sp;
    td->regs.sr  = DEFAULT_THREAD_SR;
    td->priority = priority;
  } else {
    myprint("Tried to initialize NULL pointer\n");
  }
}

// allocates and properly initializes a list structure and
// returns a pointer to it or null
LL *CreateList(ListType type) {

  LL *list = (LL *) calloc(1, sizeof(LL));
  if(list != NULL) {
    list->type = type;
    list->head = NULL;
  }

  return list;
}

// Destroys list and returns RC_SUCCESS if successful,
// otherwise RC_FAILURE.
RC DestroyList(LL *list) {

  if(list != NULL) {
    
    //TODO: do we really need to free each element in the
    // list or can we just free the list structure.
    if(list->head != NULL) {
      TD *next = list->head->link;
      free(list->head);
      while(next != NULL) {
        TD *cur = next;
        next = cur->link;
        free(cur);
      }
    }

    free(list);
    return RC_SUCCESS;
  }
  
  return RC_FAILURE;
}

// Dequeue the TD at the head of the list and return a
// pointer to it, or else null.
TD *DequeueHead(LL *list) {

  TD *ret = list->head;

  if(ret != NULL) {
    list->head = ret->link;
  }
  
  return ret;
}

RC PriorityEnqueue(TD *td, LL *list){
	TD * cur;

	//Check the given pointers
	if (td == NULL || list == NULL){
		return RC_FAILURE;
	}

	if (list->type != L_PRIORITY){
		return RC_INVTYPE;
	}

	cur = list->head;

	//no element in the list
	if (cur == NULL){
		list->head = td;
		td->inlist = list;
		td->link = NULL;
		return RC_SUCCESS;
	} else if (COMPARE(td->priority, cur->priority)){
		//if td has highest priority.
		list->head = td;
		td->link = cur;
		td->inlist = list;
		return RC_SUCCESS;
	}

	while (cur->link != NULL){
		//Check if the next element of cur has lower priority than td
		if (COMPARE(td->priority, cur->link->priority)){
			td->link = cur->link;
			td->inlist = list;
			cur->link = td;
			return RC_SUCCESS;
		}

		cur = cur->link;
	}

	cur->link = td;
	td->inlist = list;
	td->link = NULL;
	return RC_SUCCESS;
}

RC EnqueueAtHead(TD *td, LL *list)
{
	//Check the given pointers
	if (td == NULL || list == NULL){
		return RC_FAILURE;
	}

	if(list->type != L_LIFO)
		return RC_INVTYPE;

	if(list->head == NULL) {
		list->head = td;
		td->inlist = list;
		td->link = NULL;

		return RC_SUCCESS;
	} else {
		td->link = list->head;
		list->head = td;
		td->inlist = list;

		return RC_SUCCESS;
	}
}

RC WaitlistEnqueue(TD *td, int waittime, LL *list)
{
	//Check the given pointers
	if (td == NULL || list == NULL){
		return RC_FAILURE;
	}

	if(list->type != L_WAITING)
		return RC_INVTYPE;

	if(list->head == NULL) {
		list->head = td;
		td->waittime = waittime;
		td->link = NULL;
		td->inlist = list;

		return RC_SUCCESS;
	}

	TD *next = list->head;
	TD *prev = NULL;
	uval32 total_waittime = 0;

	while(next != NULL) {
		total_waittime += next->waittime;

		if(waittime <= total_waittime) {

			// found the spot for insert
			if(prev == NULL)
				list->head = td;
			else
				prev->link = td;

			td->link = next;
			td->inlist = list;
			td->waittime = waittime - (total_waittime - next->waittime);
			next->waittime -= td->waittime;

			return RC_SUCCESS;
		}

		prev = next;
		next = next->link;
	}

	// add the thread to the end of the queue
	prev->link = td;
	td->waittime = waittime - total_waittime;
	td->link = NULL;
	td->inlist = list;

	return RC_SUCCESS;
}

TD *FindTD(ThreadId tid, LL *list)
{
	TD *td = list->head;
	while(td != NULL) {
		if(td->tid == tid)
			return td;
		td = td->link;
	}

	return NULL;
}

RC DequeueTD(TD *td)
{
  if(td->inlist == NULL)
    return RC_FAILURE;

  LL *list = td->inlist;
  if(list->type == UNDEF)
    return RC_FAILURE;
  else {

    // check if the thread is at the head of the list
    if(list->head != td)
      return RC_FAILURE;

    list->head = td->link;
    td->link = NULL;
    td->inlist = NULL;

    return list->type;
  }
}
