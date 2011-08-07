#ifndef _STACK_H_
#define _STACK_H_

#include "defines.h"

#define STACK_BLOCKS 10		/* maximum number of blocks we have for our stack */
#define BLOCK_SIZE	50		/* block size in bytes */

/* Initialize the stack allocator */
void stackInit(void);

/* Allocate numBlocks blocks of stack space and return a pointer to it */
uval8 *stackAlloc(int numBlocks);

/* Deallocate the given stack space which includes numBlocks blocks */
uval8 *stacDealloc(uval8 *stack, int numBlocks);

void testSetBit(void);

#endif