#include <reent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <stdlib.h>
#include "asm_defines.h"

extern int free_memory_start;
extern int free_memory_end;

char * __env[1] = { 0 };
__attribute__ ((unused)) static char ** environ = __env;

__attribute__ ((noreturn)) void _sys_exit(int32_t return_code)
{
    // Only go to sleep in release builds.
    // just put system into WFI mode
    _ARM_WFI();

    while (1) ;
}

__attribute__ ((naked)) void mybkpt(void)
{
    asm volatile (
        "bx lr          \n"
        );
}

int _fstat(int fd, struct stat * st)
{
    return 0;
}


int _wait(int *status)
{
    errno = ECHILD;
    return -1;
}

pid_t _getpid()
{
    return 1;
}

__attribute__ ((noreturn)) void _exit(int status)
{
    _sys_exit(status);
    while (1) ;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

caddr_t _sbrk(int nbytes)
{
    return 0;
}

int _write(int fd,char *ptr,int len)
{
    return 0;
} 

int _read(int fd, char *buf, int nbytes)
{
    return 0;
}

off_t _lseek(int fd, off_t offset, int whence)
{
    return (off_t)0;
}

int _close(int fd)
{
    return -1;
}

int _isatty(int fd)
{
    return 1;
}

int _open(const char *name, int flags, int mode)
{
    return -1;
}

int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}

int _fork()
{
    errno = EAGAIN;
    return -1;
}
