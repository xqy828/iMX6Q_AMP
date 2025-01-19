#ifndef EXCEPTION_H_
#define EXCEPTION_H_

typedef void (*ExceptionHandler)(void *data);


#define EXCEPTION_ID_RESET                      0
#define EXCEPTION_ID_UNDEFINED_INT              1
#define EXCEPTION_ID_SWI_INT                    2
#define EXCEPTION_ID_PREFETCH_ABORT_INT         3
#define EXCEPTION_ID_DATA_ABORT_INT             4
#define EXCEPTION_ID_IRQ_INT                    5
#define EXCEPTION_ID_FIQ_INT                    6

void ExceptionRegisterHandler(unsigned int Exception_id,ExceptionHandler Handler,void *Data);




#endif
