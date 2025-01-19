#include "public.h"
#include "cortex_a9.h"
#include "exception.h"

typedef struct {
	ExceptionHandler Handler;
	void *Data;
} Exception_VectorTableEntry;

static void ExceptionNullHandler(void *Data)
{
    disp("Exception NUll \n");
    (void) Data;
DieLoop: goto DieLoop;
}

Exception_VectorTableEntry Exception_VectorTable[7] =
{
	{ExceptionNullHandler, NULL},// reset
	{ExceptionNullHandler, NULL},// undefined 
	{ExceptionNullHandler, NULL},// swi
	{ExceptionNullHandler, NULL},// prefech abort
	{ExceptionNullHandler, NULL},// data abort
	{ExceptionNullHandler, NULL},// irq
	{ExceptionNullHandler, NULL},// fiq
};

void ExceptionRegisterHandler(unsigned int Exception_id,ExceptionHandler Handler,void *Data)
{
	Exception_VectorTable[Exception_id].Handler = Handler;
	Exception_VectorTable[Exception_id].Data = Data;
}

void irq_interrupt(void)
{
	Exception_VectorTable[EXCEPTION_ID_IRQ_INT].Handler(Exception_VectorTable[EXCEPTION_ID_IRQ_INT].Data);
}

