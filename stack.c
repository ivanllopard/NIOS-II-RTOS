#include "defines.h"
#include "stack.h"

#define BITMAP_SIZE ((STACK_BLOCKS / 8) + 1)

uval8 stack[BLOCK_SIZE * STACK_BLOCKS];
uval8 stack_bitmap[BITMAP_SIZE];
static uval8 *nextFreeAvail = NULL;

void stackInit(void)
{
	// A bitmap value of 0 means that corresponding block is free
	int i;
	for(i = 0; i < BITMAP_SIZE; i++)
		stack_bitmap[i] = 0x0;
}

uval8 *stackAlloc(int numBlocks)
{
	// check if this is our first stack allocation
	if(nextFreeAvail == NULL) {
		nextFreeAvail = &stack;

		if(numBlocks > STACK_BLOCKS)
			return NULL;
	}
}

void setBit(int bit, uval8 val)
{
	uval8 *stackpt = &stack_bitmap;
	int r;
	while(1) {
		r = bit % 8;
		bit = bit / 8;

		if(bit > 0)
			stackpt += 1;
		else
			break;
	}

	*stackpt = *stackpt | (val << r);
}

uval8 *stacDealloc(uval8 *stack, int numBlocks)
{

}

void testSetBit(void)
{
	myprint("Testing Stack - SetBit...\n");
}