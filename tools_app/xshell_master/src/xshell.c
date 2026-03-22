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
#include <termios.h>

#define msleep(n) usleep(n*1000)
#define CMD_BUF_LEN (128)
#define CMD_HISTORY_MAX_LEN (128)
#define CMD_DEFAULT_HISTORY_MAX_LEN (128)
#define RET_OK 0
#define RET_NOK 1
#define MALLOC(n,type) ((type*)malloc((n)*sizeof(type)))

#define COLOR_NONE        "\033[0m"
#define FONT_COLOR_RED    "\033[0;31m"
#define FONT_COLOR_BLUE    "\033[1;34m"

#define SHELL            0x55  /* MQ */

#define TINYSH_HISTORY_NEXT 0
#define TINYSH_HISTORY_PREV 1

typedef unsigned long int     UADDR;
typedef unsigned int         U32;
typedef long int            SADDR;
typedef int                 S32;
typedef unsigned char         U8;
typedef enum 
{
    NOTTY_MODE = 2,
    NOTERM_MODE,
    RAW_MODE,
}mode;

typedef enum 
{
    abnormal = 1,
    normal,
}errstate;

typedef enum
{
    DISABLE = 0,    
    ENABLE,
}onoff;
enum KEY_ACTION{
    KEY_NULL = 0,        /* NULL */
    CTRL_A = 1,         /* Ctrl+a */
    CTRL_B = 2,         /* Ctrl-b */
    CTRL_C = 3,         /* Ctrl-c */
    CTRL_D = 4,         /* Ctrl-d */
    CTRL_E = 5,         /* Ctrl-e */
    CTRL_F = 6,         /* Ctrl-f */
    CTRL_H = 8,         /* Ctrl-h */
    TAB = 9,            /* Tab */
    CTRL_K = 11,        /* Ctrl+k */
    CTRL_L = 12,        /* Ctrl+l */
    ENTER = 13,         /* Enter */
    CTRL_N = 14,        /* Ctrl-n */
    CTRL_P = 16,        /* Ctrl-p */
    CTRL_T = 20,        /* Ctrl-t */
    CTRL_U = 21,        /* Ctrl+u */
    CTRL_W = 23,        /* Ctrl+w */
    ESC = 27,           /* Escape */
    BACKSPACE =  127    /* Backspace */
};

typedef struct 
{
    S32 ifd;            /* Terminal stdin file descriptor. */
    S32 ofd;            /* Terminal stdout file descriptor. */
    char *buf;          /* Edited cmd buffer. */
    size_t buflen;      /* Editedcmd buffer size. */
    const char *prompt; /* Prompt to display. */
    size_t plen;        /* Prompt length. */
    size_t pos;         /* Current cursor position. */
    size_t oldpos;      /* Previous refresh cursor position. */
    size_t len;         /* Current edited cmd length. */
    size_t cols;        /* Number of columns in terminal. */
    size_t maxrows;     /* Maximum num of rows used so far (multiline mode) */
    S32 history_index;  /* The history index we are currently editing. */
}gstTinyshState;

typedef struct
{
    char *b;
    U32 ulLen;
}AppendBuf;
//==================global variable=================== 
pthread_t pid;
typedef struct{
    SADDR mtype;
    char mtext[CMD_BUF_LEN];
}MSGQ_STRUCT;
MSGQ_STRUCT glMesg;
S32 glMsgQid = 0;
static char* Target = "/usr/local/app/SW_APP.out";

static int history_max_len = CMD_DEFAULT_HISTORY_MAX_LEN;
static int history_len = 0;
static char **history = NULL;
static char *unsupported_term[] = {"dumb","cons25","emacs",};
static struct termios termios_bak;
static U32 ulrawmode = DISABLE;
static char gpcRedirectName[64]={0};
//=====================================================

static void pr_exit(S32 status)
{
    if(WIFEXITED(status))
    {
        printf("normal termination, exit status = %d \n",WEXITSTATUS(status));
    }
    else if ( WIFSIGNALED(status))
    {
        printf("abnormal termination, signal number = %d%s \n",WTERMSIG(status),
 
         #ifdef WCOREDUNP
            WCOREDUMP(status) ? " (core file generated)" : "");
        #else
            "");
        #endif   
    } 
    else if(WIFSTOPPED(status))
    {
        printf("child stopped, signal number = %d\n",WSTOPSIG(status));

    }

}

static __attribute__((used)) S32 linux_SystemWithCheck(char *cmd)
{
    S32 rc;
    if(cmd==NULL)
    {
        printf(FONT_COLOR_RED"linux_SystemWithCheck:Input cmd error !\n"COLOR_NONE);
        return RET_NOK;
    }
    if((rc = system(cmd)) < 0 )
    {
        printf("sysrem() error");
    }
    pr_exit(rc);
    return RET_OK;
}


static S32 my_system(char *cmd)
{
    FILE *fp;
    S32 rc =0;
    char buf[1024] = {0};
    if(cmd==NULL)
    {
        printf(FONT_COLOR_RED"my_system:Input cmd buff error !\n"COLOR_NONE);
        return RET_NOK;
    }
    if((fp = popen(cmd, "r") ) == NULL)
    {
        perror("popen\n");
        printf(FONT_COLOR_RED"my_system:popen error: %s\n"COLOR_NONE,strerror(errno));
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

void tinysh_atexit(void)
{
    U32 i;
    if(ulrawmode && tcsetattr(STDIN_FILENO,TCSAFLUSH,&termios_bak) != -1 )
    {
        ulrawmode = DISABLE;
    }
    if(history) 
    {
        for(i = 0;i<history_len;i++)
        {
            free(history[i]);
        }
        free(history);
    }
}
U32 isUnsupportedTerm(void)
{
    char * term = NULL;    
    U32 i;
    term = getenv("TERM");
    if(term == NULL)
    {
        return 0;
    }
    for(i = 0; i < sizeof(unsupported_term)/sizeof(unsupported_term[0]); i++)
    {
        if(!strcasecmp(term,unsupported_term[i]))
        {
            return 1;
        }
    }
    return 0;
}

S32 disableRawMode(S32 fd)
{
     /* Don't even check the return value as it's too late. */
    if (ulrawmode==ENABLE && tcsetattr(fd,TCSAFLUSH,&termios_bak) != -1)
        ulrawmode =DISABLE;
    return RET_OK;
}

S32 enableRawMode(S32 fd)
{
    struct termios raw;
    tcgetattr(fd,&termios_bak);
    raw = termios_bak;/* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer.
     * We want read to return every single byte, without timeout. */
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

    /* put terminal in raw mode after flushing */
    tcsetattr(fd,TCSAFLUSH,&raw) ;
    ulrawmode = ENABLE;
    return RET_OK;
}

U32 tinysh_modecheck(void)
{
    if(!isatty(STDIN_FILENO))
    {
        return NOTTY_MODE;
    }
    if(isUnsupportedTerm())
    {
        return NOTERM_MODE;
    }
    else 
    {
        atexit(tinysh_atexit);
        return RAW_MODE;
    }
}

U32 tinyshHistoryAdd(const char *buf)
{
    char * buftemp;
    if(history_max_len == 0)
    {
        return RET_NOK;
    }
    if(history == NULL)
    {
        history = malloc(sizeof(char*)*history_max_len);
        memset(history,0,sizeof(char*)*history_max_len);
    }
    if(history_len && !strcmp(history[history_len-1],buf))
    {
        return RET_OK;
    }
    buftemp = strdup(buf);
    if (history_len == history_max_len) 
    {
        free(history[0]);
        memmove(history,history+1,sizeof(char*)*(history_max_len-1));
        history_len--;
    }
    history[history_len] = buftemp;
    history_len++;
    return RET_OK;
}

U32 tinyshHistorySave(const char *filename)
{
    mode_t old_umask = umask(S_IXUSR|S_IRWXG|S_IRWXO);
    FILE *fp;
    int j;

    fp = fopen(filename,"w");
    umask(old_umask);
    if (fp == NULL) return RET_NOK;
    chmod(filename,S_IRUSR|S_IWUSR);
    for (j = 0; j < history_len; j++)
        fprintf(fp,"%s\n",history[j]);
    fclose(fp);
    return RET_OK;
}

U32 tinysh_historyload(const char *filename)
{
    FILE *fp = fopen(filename,"r");  
    char *p;
    char cmd_buf[CMD_HISTORY_MAX_LEN];    
    if (NULL == fp)
    {
        return RET_NOK;
    }
    while (fgets(cmd_buf,CMD_HISTORY_MAX_LEN,fp) != NULL)  
    {
        p = strchr(cmd_buf,'\r');
        if(!p) 
        {
            p = strchr(cmd_buf,'n');
        }
        if(p) 
        {
            *p= '\0';
        }
        tinyshHistoryAdd(cmd_buf);
    }
    fclose(fp);
    return RET_OK;
}

S32 getCursorPosition(int ifd, int ofd)
{
    char buf[32];
    S32 slCols, slRows;
    U32 i = 0;
    if(write(ofd, "\x1b[6n", 4) != 4)
    {
        return RET_NOK;
    }
    while (i < sizeof(buf) -1)
    {
        if(read(ifd,buf+i,1) !=1)
        {
            break;
        }
        if(buf[i]=='R')
        {
            break;
        }
        i++;
    }
    buf[i]= '\0';
    if(buf[0]!=ESC || buf[1]!='[')
    {
        return RET_NOK;
    }
    if(sscanf(buf+2,"%d,%d",&slRows,&slCols)!=2)
    {
        return RET_NOK;
    }
    return slCols;
} 


/* Try to get the number of columns in the current terminal, or assume 80
 * if it fails. */
U32 getColumns(int ifd, int ofd) 
{
    struct winsize SIZE;
    S32 Cursor_Start = 0;
    S32 Cursor_Current= 0;
    char seq[32];
    if (ioctl(1, TIOCGWINSZ, &SIZE) == -1 || SIZE.ws_col == 0) 
    {
        /* ioctl() failed. Try to query the terminal itself. */
        /* Get the initial position so we can restore it later. */
        Cursor_Start= getCursorPosition(ifd,ofd);
        if (Cursor_Start == RET_NOK) 
        {
            return  80;
        }
        /* Go to right margin and get position. */
        if (write(ofd,"\x1b[999C",6) != 6) 
        {
            return  80;
        }
        Cursor_Current = getCursorPosition(ifd,ofd);
        if (Cursor_Current == RET_NOK) 
        {
            return  80;
        }
        /* Restore position. */
        if (Cursor_Current > Cursor_Start)
        {
            snprintf(seq,32,"\x1b[%dD",Cursor_Current-Cursor_Start);
            if (write(ofd,seq,strlen(seq)) == -1) 
            {
                /* Can't recover... */
            }
        }
        return Cursor_Current;
    }
    else 
    {
        return SIZE.ws_col;
    }
}

U32 dataAppend(AppendBuf * ab,const char *s ,U32 len)
{
    char *New = realloc(ab->b,ab->ulLen+len);
    if(New == NULL)
    {
        printf("%s:realloc fail !\n",__func__);
        return RET_NOK;
    }
    memcpy(New+ab->ulLen,s,len);
    ab->b= New;
    ab->ulLen +=len;
    return RET_OK;
}

U32 refreshtinysh(gstTinyshState *shell)
{
    char seq[64];
    size_t plen = strlen(shell->prompt);
    U32 fd = shell->ofd;
    char *buf=shell->buf;
    size_t len = shell->len;
    size_t pos=shell->pos;
    
    AppendBuf stuAppendBuf;
    
    while((plen + pos) >= shell->cols)
    {
        buf++;
        len--;
        pos--;
    }
    while(plen + len > shell->cols)
    {
        len--;
    }

    stuAppendBuf.b = NULL;
    stuAppendBuf.ulLen = 0;
    /* Cursor to left edge */
    snprintf(seq,64,"\r");
    dataAppend(&stuAppendBuf,seq,strlen(seq));
    /* Write the prompt and the current buffer content */
    dataAppend(&stuAppendBuf, shell->prompt,strlen(shell->prompt));
    dataAppend(&stuAppendBuf,buf, len);
    /* Erase to right */
    snprintf(seq,64,"\x1b[0K");
    dataAppend(&stuAppendBuf,seq,strlen(seq));
    /* Move cursor to original position. */
    snprintf(seq,64,"\r\x1b[%dC", (U32)(pos+plen));
    dataAppend(&stuAppendBuf,seq,strlen(seq));
    write(fd ,stuAppendBuf.b,stuAppendBuf.ulLen);
    free(stuAppendBuf.b);
    return RET_OK;
}

U32 tinyshBackSpace( gstTinyshState *shell)
{
    if(shell->pos >0 && shell->len>0)
    {
        memmove(shell->buf+shell->pos-1,shell->buf+shell->pos,shell->len-shell->pos);
        shell->pos--;
        shell->len--;
        shell->buf[shell->len] = '\0';
        refreshtinysh(shell);
    }
    return RET_OK;
}

U32 tinyshDelete(gstTinyshState *shell)
{
    if(shell->len > 0 && shell->pos < shell->len)
    {
        memmove(shell->buf +shell->pos,shell->buf+shell->pos+1,shell->len-shell->pos);
        shell->len--;
        shell->buf[shell->len]='\0';
        refreshtinysh(shell);
    }
    return RET_OK;
}
/* Move cursor on the left. */
U32 tinyshMoveLeft(gstTinyshState *shell)
{
    if(shell->pos >0)
    {
        shell->pos--;
        refreshtinysh(shell);
    }
    return RET_OK;
}
/* Move cursor on the right. */
U32 tinyshMoveRight(gstTinyshState *shell)
{
    if(shell->pos !=shell->len)
    {
        shell->pos++;
        refreshtinysh(shell);
    }
    return RET_OK;
}
/* Move cursor on the home. start of cmd */
U32 tinyshMoveHome(gstTinyshState *shell)
{
    if(shell->pos !=0) 
    {
        shell->pos=0;
        refreshtinysh(shell);
    }
    return RET_OK;
}

U32 tinyshMoveEnd(gstTinyshState *shell)
{
    if(shell->pos != shell->len) 
    {
        shell->pos = shell->len;
        refreshtinysh(shell);
    }
    return RET_OK;
}

U32 tinyshHistoryShow( gstTinyshState *shell,U8 history_dir)
{
/*
    if(history_len > 1)
    {
        free(history[history_len - 1 - shell->history_index]);
        history[history_len - 1 - shell->history_index] = strdup(shell->buf);
    }
*/
    shell->history_index += (history_dir == TINYSH_HISTORY_PREV) ? 1 : -1;
    if(shell->history_index <0)
    {
        shell->history_index=0;
        return RET_OK;
    }
    else if(shell->history_index >= history_len)
    {
        shell->history_index = history_len - 1;
        return RET_OK;
    }
    strncpy(shell->buf,history[history_len-1-shell->history_index],shell->buflen);
    shell->buf[shell->buflen-1] = '\0';
    shell->len = shell->pos = strlen(shell->buf);
    refreshtinysh(shell);
    return RET_OK;
}

U32 tinyshClearScreen(void)
{
    write(STDOUT_FILENO,"\x1b[H\x1b[2J",7);
    return RET_OK;
}

U32  tinyshInsert(gstTinyshState *shell, char c) 
{
    if (shell->len < shell->buflen) 
    {
        if (shell->len == shell->pos)
        {
            shell->buf[shell->pos] = c;
            shell->pos++;
            shell->len++;
            shell->buf[shell->len] = '\0';
            refreshtinysh(shell);
        }
        else 
        {
            memmove(shell->buf+shell->pos+1,shell->buf+shell->pos,shell->len-shell->pos);
            shell->buf[shell->pos] = c;
            shell->len++;
            shell->pos++;
            shell->buf[shell->len] = '\0';
            refreshtinysh(shell);
        }
    }
    return RET_OK;
}

U32 tinysh_ctrl(S32 stdin_fd, S32 stdout_fd, char *buf, size_t buflen, const char *prompt)
{
    char c;
    S32 nread;
    char seq[3];
    U32 auxtemp;
    gstTinyshState  tinysh_state;
    tinysh_state.ifd = stdin_fd;
    tinysh_state.ofd = stdout_fd;
    tinysh_state.buf = buf;
    tinysh_state.buflen = buflen;
    tinysh_state.prompt = prompt;
    tinysh_state.plen = strlen(prompt);
    tinysh_state.oldpos = tinysh_state.pos = 0;
    tinysh_state.len = 0;
    tinysh_state.cols = getColumns(stdin_fd,stdout_fd);
    tinysh_state.maxrows = 0;
    tinysh_state.history_index = 0;

    tinysh_state.buf[0] = '\0';
    tinysh_state.buflen--;
    /* The latest history entry is always our current buffer, that
     * initially is just an empty string. */
    tinyshHistoryAdd("");
    
    write(tinysh_state.ofd ,tinysh_state.prompt,tinysh_state.plen);
    for(;;)
    {
        nread = read(tinysh_state.ifd,&c,1);
        if(nread == 0)
        {
            return RET_NOK;
        }
        /*wait add TAB */
        
        switch(c)
        {
            case ENTER:
                history_len--;
                free(history[history_len]);
                refreshtinysh(&tinysh_state);
                return RET_OK;
            break;
            case BACKSPACE:
            case 8:
                tinyshBackSpace(&tinysh_state);
            break;
            case CTRL_D: /* ctrl-d, remove char at right of cursor, or if the
                                        line is empty, act as end-of-file. */
                if(tinysh_state.len >0)
                {
                    tinyshDelete(&tinysh_state);
                }
                else
                {
                    history_len--;
                    free(history[history_len]);
                    return RET_NOK;
                }                
            break;
            case CTRL_T: /* ctrl-t, swaps current character with previous. */
            if (tinysh_state.pos > 0 && tinysh_state.pos < tinysh_state.len) {
                auxtemp = buf[tinysh_state.pos-1];
                buf[tinysh_state.pos-1] = buf[tinysh_state.pos];
                buf[tinysh_state.pos] = auxtemp;
                if (tinysh_state.pos != tinysh_state.len-1) tinysh_state.pos++;
                refreshtinysh(&tinysh_state);
            }
            break;
            case CTRL_B:     /* ctrl-b */
                tinyshMoveLeft(&tinysh_state);
            break;
            case CTRL_F:
                tinyshMoveRight(&tinysh_state);
            break;
            case CTRL_P:
                tinyshHistoryShow(&tinysh_state,TINYSH_HISTORY_PREV);
            break;
            case CTRL_N:
                tinyshHistoryShow(&tinysh_state,TINYSH_HISTORY_NEXT);
            break;
            case  ESC:
                read(tinysh_state.ifd,seq,1);
                read(tinysh_state.ifd,seq+1,1);
                if(seq[0]=='[')
                {
                    if(seq[1] >='0' && seq[1]<= '9')
                    {
                        read(tinysh_state.ifd,seq+2,1);
                        if(seq[2]=='~')
                        {
                            switch(seq[1])
                            {
                                case '3' :/* delete key */
                                tinyshDelete(&tinysh_state);                                
                                break;
                            }    
                        }
                    }
                    else
                    {
                        switch(seq[1])
                        {
                            case 'A': /*   UP    */
                                tinyshHistoryShow(&tinysh_state,TINYSH_HISTORY_PREV);
                            break;
                            case 'B': /*  DOWN    */
                                tinyshHistoryShow(&tinysh_state,TINYSH_HISTORY_NEXT);
                            break;
                            case 'C': /* Right */
                                tinyshMoveRight(&tinysh_state);
                            break;
                            case 'D':/*  left  */
                                tinyshMoveLeft(&tinysh_state);
                            break;
                            case 'H':/*  home  */
                                tinyshMoveHome(&tinysh_state);
                            break;
                            case 'F': /* end  */
                                tinyshMoveEnd(&tinysh_state);
                            break;
                        }
                    }
                }
                else if(seq[0] == 'O')
                {
                    switch(seq[1])
                    {
                        case 'H':
                            tinyshMoveHome(&tinysh_state);
                        break;
                        case 'F':
                            tinyshMoveEnd(&tinysh_state);
                        break;
                    }
                }
            break;            
            case CTRL_U : 
                buf[0]='\0';/* Ctrl+u, delete the whole line. */
                tinysh_state.pos = tinysh_state.len = 0;
                refreshtinysh(&tinysh_state);
            break;
            case CTRL_K:/* Ctrl+k, delete from current to end of line. */
                buf[tinysh_state.pos]='\0';
                tinysh_state.len = tinysh_state.pos;
                refreshtinysh(&tinysh_state);
            break;
            case CTRL_A:/* Ctrl+a, go to the start of the line */
                tinyshMoveHome(&tinysh_state);
            break;
            case CTRL_E: /* ctrl+e, go to the end of the line */
                tinyshMoveEnd(&tinysh_state);
            break;
            case CTRL_L: /* ctrl+l, clear screen */
                tinyshClearScreen();
                refreshtinysh(&tinysh_state);
            break;
            default: 
                tinyshInsert(&tinysh_state,c);
            break;
        }
    }
    return RET_OK;
}

U32 gets_thread(void *arg)
{    
    S32 rc;
    U32 ulLen = 0;
    char *msg = NULL;
    U32 c = 0;
    msg = MALLOC(CMD_BUF_LEN,char);
    U32 ulTermMode= *(U32*)arg;
    glMesg.mtype = SHELL;
    for(;;)
    {
        switch(ulTermMode)
        {
            case NOTTY_MODE:
                for(;;)
                {
                    c = fgetc(stdin);
                    if(c == EOF || c == '\n')
                    {
                        if (c == EOF && ulLen == 0)
                        {
                            msg[0] = '\n'; 
                            break;
                        }
                        else 
                        {
                            msg[ulLen] = '\0';
                            break;
                        }
                    }
                    else 
                    {
                            msg[ulLen] = c; 
                            ulLen++;
                    }
                }      
                if(msg[0] == '\n')
                {
                    continue;
                } 
            break;
            case NOTERM_MODE:
                printf("xshell>>");
                fflush(stdout);
                if(NULL == fgets(msg,CMD_BUF_LEN,stdin))
                {
                    continue;
                }        
                if(msg[0] == '\n')
                {
                    continue;
                } 
                ulLen  = strlen(msg);      
                while (ulLen && (msg[ulLen - 1] == '\n' || msg[ulLen - 1] == '\r' ))    
                {
                    ulLen -- ;
                    msg[ulLen] = '\0';
                }
            break;
            case RAW_MODE:
                enableRawMode(STDIN_FILENO);
                tinysh_ctrl(STDIN_FILENO,STDOUT_FILENO,msg,CMD_BUF_LEN,"xshell>>");
                disableRawMode(STDIN_FILENO);
                printf("\n");
            break;
            default :
                printf("terminal mode :%d error \n",ulTermMode);
            break;
        }
        if(!strcmp(msg,"exit"))
        {
                printf("exit xshell !\n");
                free(msg);
                disableRawMode(STDIN_FILENO);
                return RET_NOK;
        }
        if(msg[0]!='\0' && msg[0]!='/')
        {
            tinyshHistoryAdd(msg);
            tinyshHistorySave("/var/log/sh-history.txt");
            memcpy(&glMesg.mtext,msg,CMD_BUF_LEN);
            rc = msgsnd(glMsgQid,&glMesg,CMD_BUF_LEN,IPC_NOWAIT);
            if(rc == -1)
            {
                printf(FONT_COLOR_RED"Msg send error !\n"COLOR_NONE);
                continue;
            }
            //printf("msg:'%s' \n",msg);
        }
        memset(&glMesg.mtext,0,CMD_BUF_LEN);
        memset(msg,0,CMD_BUF_LEN);
        msleep(100);
    }
    return RET_OK;
}

U32 tinysh_init(U32 ulTermMode)
{
        U32 ret;
        U32 ulMode = ulTermMode;
    ret = pthread_create(&pid,NULL,(void*)gets_thread,(void*)&ulMode);
    if(ret)
    {
        perror("tinysh thread create failed \n");
        return RET_NOK;
    }
        sleep(1);
    return RET_OK;
}

U32 CreateMsq(char* MsgName)
{
    key_t key;
    if((key = ftok(MsgName,'b')) == -1)
    {
        perror("Master Msg Key Create Fail !\n");
        return RET_NOK;
    }
    if((glMsgQid = msgget(key,IPC_CREAT|0666) == -1))
    {
        perror("Mster Msg Qid Create Fail !\n");
        return RET_NOK;
    }
    return RET_OK;
}


U32 getCurrentTty(char *pcTtyBuf, U32 ulBufLen)
{
    FILE *pFile = NULL;
    U32 ulFileLen = 0;
    S32 rc = 0;
    if(pcTtyBuf == NULL)
    {
        printf(FONT_COLOR_RED"Input  buff error !\n"COLOR_NONE);
        return RET_NOK;
    }
    rc = my_system("tty > /var/log/tty.txt");
    if(rc !=RET_OK)
    {
        printf(FONT_COLOR_RED"tty error !\n"COLOR_NONE);
        return RET_NOK;
    }
    pFile = fopen("/var/log/tty.txt","r");
    if(pFile == NULL)
    {
        printf("open tty.txt error \n");
        return RET_NOK;
    }    
    fseek(pFile,0,SEEK_END);
    ulFileLen = ftell(pFile);
    rewind(pFile);
    if(ulFileLen >= ulBufLen)
    {
        printf("tty.txt length is %d overflow (MAX %u)\n",ulFileLen,ulBufLen);
        fclose(pFile);
        return RET_NOK;
    }
    memset(pcTtyBuf,0,ulBufLen);
    fread(pcTtyBuf,1,ulFileLen-1,pFile);
    fclose(pFile);
    printf("Current tty: %s\n",pcTtyBuf);
    return RET_OK;
}

U32 RedirectTty(void)
{
    char ttybuf[64] = {0};
    U32 ulLen = 64;
    S32 rc =0;
    getCurrentTty(ttybuf,ulLen);
    fflush(stdout);
    freopen(ttybuf,"w",stdout);
    memcpy(gpcRedirectName,"TTY:",4);
    strncpy(gpcRedirectName+4,ttybuf,sizeof(gpcRedirectName)-1-4);
    memcpy(&glMesg.mtext,gpcRedirectName,sizeof(gpcRedirectName)-1);
    /* send tty to app */
    glMesg.mtype = SHELL;
    rc = msgsnd(glMsgQid,&glMesg,CMD_BUF_LEN,IPC_NOWAIT);
    if(rc == -1)
    {
        printf(FONT_COLOR_RED"Msg send error !\n"COLOR_NONE);  
     }
    memset(&glMesg.mtext,0,CMD_BUF_LEN);
    return RET_OK;
} 


/*==============================================
 *SHELL Input Format Eg:
 *UShellInt=-123 or UShellInt=0x55 or UShellInt=123 or UShellInt=-0x55
 *hello(S32 arg,S32 b,S32 c):
 *hello' ';hello 12; hello 12 -0x55; hello 12 -0x55 0x55
 *GetStatus (bool* status):     
 *UShellInt' ';-->addr:0x0000000000f83088:value=0
 *GetStatus' ' 0xf83088;
 *UShellInt' ';-->addr:0x0000000000f83088:value=1
 *Input string must end with '\n'
 *System command not support pipe and cd
 *Input exit command quit shell
 *Not support float and double
 *==============================================*/
S32 main (S32 argc, char* argv[])
{
    U32 tinysh_mode;
    CreateMsq(Target);
    usleep(200000);
    RedirectTty();
    tinysh_historyload("/var/log/sh-history.txt");
    tinysh_mode = tinysh_modecheck();
    tinysh_init(tinysh_mode);
    pthread_join(pid,NULL);    
    return RET_OK;
}

