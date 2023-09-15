#ifndef __PUBLIC_H__
#define __PUBLIC_H__
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <errno.h>

#define RET_OK 0
#define RET_NOK 1

#ifndef FALSE
#define FALSE false
#endif

#ifndef TRUE
#define TRUE true
#endif

#define COLOR_NONE  "\033[0m"
#define COLOR_RED   "\033[0;31m"
#define COLOR_BLUE  "\033[1;34m"

#define msleep(n) usleep(n*1000)
#define MALLOC(n,type) ((type*)malloc((n)*sizeof(type)))
#define U32_MAX         (4294967295u)                                  
#define U64_MAX         (18446744073709551615u)
#define offset_of(TYPE,MEMBER)   ((size_t)&((TYPE*)0)->MEMBER)
#define container_of(ptr,type,member)\
    do{\
    void *__mptr = (void*)(ptr);\
    ((type*)(__mptr - offset_of(type,member)));\
    }while(0)

#define MEM_B(X) (*((unsigned char *)(X)))
#define MEM_W(X) (*((unsigned int *)(X)))
#define MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#define MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#define UPCASE( c ) ( ((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c) )
#define ARR_SIZE( a ) ( sizeof( (a) ) / sizeof( (a[0]) ) )
#define UNUSED_PARA(para)   ((void)(para))
#define SIZE_1M             (1024*1024)

typedef unsigned long int   	UADDR;
typedef unsigned int        	U32;
typedef long int            	SADDR;
typedef int                 	S32;
typedef bool					BOOL;
typedef volatile unsigned int 	VU32; 
typedef unsigned short      U16;
typedef unsigned long long int U64;
typedef signed long long int S64;
typedef short                        S16;
typedef unsigned short          U16;
typedef float                          FLOAT;
typedef double                      DOUBLE;
typedef unsigned char 		U8;

typedef enum
{
    Disable = 0,
    Enable,
}SYS_ONOFF;

 __attribute__((unused)) static S32 my_system(char *cmd)
{
    FILE *fp;
    S32 rc =0;
    char buf[1024] = {0};
    if(cmd==NULL)
    {
        printf("\033[0;31mmy_system:Input cmd buff error !\033[0m\n");
        return RET_NOK;
    }
    if((fp = popen(cmd, "r") ) == NULL)
    {
        perror("popen\n");
        printf("\033[0;31mmy_system:popen error: %s \033[0m\n",strerror(errno));
        return RET_NOK;
    }
    else
    {
        while(fgets(buf, sizeof(buf), fp))
        {
            fprintf(stdout, "%s", buf);
        }
        if((rc = pclose(fp)) == -1)
        {
            printf("my_system:close popen file pointer fp error!\n");
            return rc;
        }
        else if(rc == 0)
        {
            return RET_OK;
        }
        else
        {
            printf("my_system:close popen rc is %d \n",rc);
            return rc;
        }
    }
}

#endif
