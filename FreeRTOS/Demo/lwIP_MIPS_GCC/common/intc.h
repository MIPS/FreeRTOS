#ifndef __INTC_H__
#define __INTC_H__

#include "FreeRTOS.h"

typedef struct handler{
	int int_num;
	int ext_num;
	uint32_t mask;
	void (*function)(void*);
	void *parameter;
	struct handler *next;
}HANDLER_DESC_T;

typedef struct {} INTC_T; /* opaque */

int32_t intc_RegisterHandler(INTC_T* ctrlr, HANDLER_DESC_T* irq);
void intc_init(INTC_T* ctrlr);
void intc_ack(int num);


#endif
