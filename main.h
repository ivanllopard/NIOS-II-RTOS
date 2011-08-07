#ifndef _MAIN_H_
#define _MAIN_H_

#include "defines.h"

int main(void);
void myprint(char *text);
void printHex(uval32 num);


#ifdef NATIVE

void interrupt_handler(void);
void pushbutton_isr(void);
void timer_isr(void);
void check_exception(void);
#endif /* NATIVE */

#endif 

