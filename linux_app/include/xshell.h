#ifndef __XSHELL_H__
#define __XSHELL_H__
#include "public.h"

#define CMD_BUF_LEN (128)
#define STRING(arg) #arg
#define COMBINE(x,y) x##y

#define FUNC            0x01
#define OBJECT          0x10                                          
#define NOCARE          (FUNC|OBJECT)
#define SHELL           0x55

typedef U32 (*tinyshfunc)(SADDR,SADDR,SADDR,SADDR,SADDR,SADDR,\
                           SADDR,SADDR,SADDR,SADDR,SADDR,SADDR,\
                           SADDR,SADDR,SADDR,SADDR,SADDR,SADDR,\
                           SADDR,SADDR,SADDR,SADDR,SADDR,SADDR );

typedef struct{
	UADDR  uaddr;
	char symType[10];
	char symName[128];
}SYMTBL;
 
typedef struct{
	SADDR mtype;
	char mtext[CMD_BUF_LEN];
}MSGQ_STRUCT;

S32 XshellInit(void);

#endif
