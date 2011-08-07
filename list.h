#ifndef _LIST_H_
#define _LIST_H_

#include "defines.h"


typedef enum { UNDEF, L_PRIORITY, L_LIFO, L_WAITING} ListType ;

#define MAX_THREADTD 10	/* maximum number of thread descriptors */

#define MIN_PRIORITY 100

typedef struct type_LL LL;
typedef struct type_TD TD;
typedef struct type_REGS Registers;

struct type_REGS
{
  uval32 sp;
  uval32 pc;
  uval32 sr;
};

// singly-linked list
struct type_LL {
  TD *head;
  ListType type;
};

// thread descriptor
struct type_TD
{
  TD *link;
  ThreadId tid;
  Registers regs;
  int priority;
  int waittime;
  RC returnCode;
  LL * inlist;
};

TD *CreateTD(ThreadId tid);
void InitTD(TD *td, uval32 pc, uval32 sp, uval32 priority);

TD * DequeueHead( LL *list );
RC PriorityEnqueue(TD *td, LL *list);

#endif
