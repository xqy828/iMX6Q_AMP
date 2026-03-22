//#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/select.h>
#include <sys/prctl.h>
#include "xshell.h"
#include "public.h"

//==================global variable===================
SYMTBL gSymTblArry[0xffff]={{0}};
MSGQ_STRUCT glMesg;
S32 glMsgQid = 0;
static char* Target = "/usr/local/app/SW_APP.out";
static char* SymTbl = "/usr/local/app/SymTbl.txt";
static char* Path   = "/usr/local/app/";
//===================test function and variable==============
U32 XShellInt = 0;
//===========================================================
char convert(char ch)
{
    if(ch>='A' && ch<='F')
        return ch+32;        
    else
        return ch;
}

SADDR str2int(const char* str, SADDR * slValue)
{
    U32 i;
    SADDR data = 0;
    const char* ptr;
    bool sig = true;
    static char ArgStr[128];
    char *pcStrHead = NULL;
    char *pcStrTail = NULL;
    S32 slParamLen = 0;
    if(NULL == str)
    {
        printf("error string\n");
        return RET_NOK;
    }
    slParamLen = strlen(str);
    ptr=str;
    pcStrHead = strchr(ptr,'"');
    if(NULL != pcStrHead)
    {
        pcStrTail = strchr(pcStrHead + 1,'"');
        if((pcStrTail == NULL) || ((pcStrTail - pcStrHead) != (slParamLen - 1)))
        {
            printf("Invalid Parameter!\n");
            return RET_NOK;
        }
        else
        {
            memset(ArgStr,0,128);
            memcpy(ArgStr,ptr+1,strlen(ptr)-2);
            *slValue = (SADDR)ArgStr;
            return RET_OK;
        }
    }
    if(*ptr== '-')
    {
        sig = false;
        if(*(ptr+1) == '0' && (*(ptr+2) == 'X' || *(ptr+2)=='x'))
        {
            i = 3;
        }
        else
        {
            *slValue = atoi(ptr);
            return RET_OK;
        }
    }
    else 
    {
        if(*(ptr) == '0' && (*(ptr+1) == 'X' || *(ptr+1)=='x'))
        {
            i = 2;    
        }
        else
        {
            *slValue = atoi(ptr);
            return RET_OK;
        }
    }
    for(;(*(ptr+i)>='0' && *(ptr+i)<='9') || (*(ptr+i)>='a' && *(ptr+i)<='f')\
        || (*(ptr+i)>='A' && *(ptr+i)<= 'F');i++)
    {
        if(convert(*(ptr+i)) > '9')
        {
            data = 16*data + (10+convert(*(ptr+i))-'a');
        }
        else
        {
            data = 16*data + (convert(*(ptr+i))-'0');    
        }
    }
    data = (sig == true) ?  data :  (-1*data);
    *slValue = data;
    return RET_OK;
}

UADDR GetAddrAndType(const char * str , char ** SymType)
{
    U32 line;
    U32 ret = 0xFFFFFFFF;    
    for(line =0;line < 0xFFFF;line++)
    {
        if((!strcmp(gSymTblArry[line].symName,str)))
        {
            *SymType = gSymTblArry[line].symType;
            return gSymTblArry[line].uaddr;
        }        
    }
    if(line ==(0xFFFF))
    {
        printf("not found SymType\n");
        return ret; 
    }
    return ret;
}


U32 execute_func(const char* msg)
{
    tinyshfunc pfunc;
    UADDR funcaddr = 0;
    char* str =NULL;
    char* ptr = NULL;
    char* VariableStr = NULL;
    char* Value = NULL;
    char* SymType =NULL;
    char* arg[128] = {NULL};
    char* pcStr;
    U32 cnt = 0;
    SADDR arg_tmp[128] = {0};
    SADDR slPara = 0;
    U32 ulRet = RET_NOK;
    str = MALLOC(CMD_BUF_LEN,char);
    memset(str,0,CMD_BUF_LEN);
    strcpy(str,msg);
    
    ptr = MALLOC(CMD_BUF_LEN,char);
    memset(ptr,0,CMD_BUF_LEN);
    strcpy(ptr,msg);
        
    pcStr = strtok(str," ,=");
    if(pcStr == NULL)
    {
        printf("input error\n");
        free(str);
        free(ptr);
        return RET_NOK;
    }
    funcaddr = GetAddrAndType(pcStr,&SymType);    
    if((0 == funcaddr) || (0xFFFFFFFF==funcaddr))
    {   
        printf("not found funcaddr\n");
        free(str);
        free(ptr);
        return RET_NOK;
    }
    if(!strcmp(SymType,"FUNC"))
    {
        pfunc = (tinyshfunc)funcaddr;
        if (!strncmp(pcStr,"TestAmpFuncCallMsgSend",22))
        {
            pcStr = strchr(ptr,'"');
            arg[0] = pcStr;
            ulRet = str2int(arg[0],&slPara);
            if(RET_OK == ulRet)
            {
                arg_tmp[0] = slPara;
            }
            else
            {
                free(str);
                free(ptr);
                return RET_NOK;
            }
        }
        else
        {
            pcStr = strtok(ptr," ,");
            while(NULL != pcStr)
            {
                pcStr = strtok(NULL," ,");
                arg[cnt] = pcStr;
                cnt++;
            }
            cnt = 0;
            while(NULL != arg[cnt])
            {
                ulRet = str2int(arg[cnt],&slPara);
                if(RET_OK == ulRet)
                {
                    arg_tmp[cnt] = slPara;
                    cnt++;
                }
                else
                {
                    free(str);
                    free(ptr);
                    return RET_NOK;
                }
            }
        }
        pfunc(arg_tmp[0],arg_tmp[1],arg_tmp[2],arg_tmp[3],arg_tmp[4],arg_tmp[5],\
              arg_tmp[6],arg_tmp[7],arg_tmp[8],arg_tmp[9],arg_tmp[10],arg_tmp[11],\
             arg_tmp[12],arg_tmp[13],arg_tmp[14],arg_tmp[15],arg_tmp[16],arg_tmp[17],\
              arg_tmp[18],arg_tmp[19],arg_tmp[20],arg_tmp[21],arg_tmp[22],arg_tmp[23] );
        printf("\033[1;34maddr:0x%08lx.\033[0m\r\n",funcaddr);
     
    }
    else if(!strcmp(SymType,"OBJECT"))
    {
        VariableStr = strchr(ptr,'=');
        if(VariableStr == NULL)
        {
            printf("\033[1;34maddr:0x%08lx:value=%ld.\033[0m\r\n",funcaddr,*((UADDR*)funcaddr));
        }
        else
        {
            Value = strtok(VariableStr," ,=");
        }
        if(Value != NULL)
        {
            ulRet = str2int(Value,&slPara);//only support int type 
            if(RET_OK == ulRet)
            {
                *((UADDR*)funcaddr) = slPara;
                printf("\033[1;34maddr:0x%08lx:value=%ld.\033[0m\r\n",funcaddr,*((UADDR*)funcaddr));
            }
            else
            {
                free(str);
                free(ptr);
                return RET_NOK;
            }
        }
    }

    free(str);
    free(ptr);
    return RET_OK;
}

U32 gets_thread(void)
{    
    char* msg;
    S32 rc= 0;
    prctl(PR_SET_NAME, "t_shell", 0, 0, 0);    
    msg = MALLOC(CMD_BUF_LEN,char);
    for(;;)
    {    
        rc = msgrcv(glMsgQid,&glMesg,CMD_BUF_LEN,SHELL,0);
        if(rc > 0)
        {
#ifdef DEBUG
            printf(COLOR_BLUE"msgrcv:%s\n"COLOR_NONE,glMesg.mtext);
#endif
        }
        else
        {
            continue;
        }
        memcpy(msg,&glMesg.mtext,CMD_BUF_LEN);
        if(msg[strlen(msg)-1]=='\n')
        {
            msg[strlen(msg)-1]='\0';
        }
/*===============================
        if(!strcmp(msg,"exit"))
        {
            free(msg);
            continue;
        }
================================*/
        if(0 == strncmp(msg,"TTY:",4))
        {
            printf("APP tty is redirect to %s\n",&msg[4]);
            freopen(&msg[4],"w",stdout);
        }
        else
        {
            execute_func(msg);
        }
        memset(msg,0,CMD_BUF_LEN);
        memset(glMesg.mtext,0,CMD_BUF_LEN);
        msleep(100);
    }
    return RET_OK;
}

U32 tinysh_init(pthread_t id)
{
    U32 ret;
    ret = pthread_create(&id,NULL,(void*)gets_thread,NULL);
    if(ret)
    {
        perror("shell client thread create fail \n");
        return RET_NOK;
    }
    printf("xshell start...\n");
    return RET_OK;
}

U32 CreateMsq(char* MsgName)
{
    key_t key;
    if((key = ftok(MsgName,'b')) == -1)
    {
        perror("Client Msg Key Create Fail !\n");
        return RET_NOK;
    }
    if((glMsgQid = msgget(key,IPC_CREAT|0666) == -1))
    {
        perror("Client Msg Qid Create Fail !\n");
        return RET_NOK;
    }
    return RET_OK;
}

U32 CreateSymTbl(void)
{
    FILE* fp = NULL;
    char *buff;
    U32 line = 0;
    U32 size = 0;    
    fp = fopen(SymTbl,"rb");
    if(NULL == fp )
    {
        perror("open error");
        return RET_NOK;
    }
    fseek(fp,0,SEEK_END);
    size = ftell(fp);//get .txt size 
    if(size > 0)
    {
        printf(COLOR_BLUE"size of SymTbl.txt is %d\n"COLOR_NONE,size);
        buff=MALLOC(128,char);
        memset(buff,0,128);
    }
    fseek(fp,0,SEEK_SET);
    while(NULL != fgets(buff,128,fp))
    {
        line++;    
        if(buff[strlen(buff)-1]=='\n')
            buff[strlen(buff)-1]='\0';
        sscanf(buff,"%lx %s %s",&gSymTblArry[line-1].uaddr,gSymTblArry[line-1].symType,\
                    gSymTblArry[line-1].symName);
        memset(buff,0,128);
    }
    free(buff);
    fclose(fp);
    return RET_OK;
}
/*==============================================
 *SHELL Input Format Eg:
 *XShellInt=-123 or XShellInt=0x55 or XShellInt=123 or XShellInt=-0x55
 *hello(S32 arg,S32 b,S32 c):
 *hello' ';hello 12; hello 12 -0x55; hello 12 -0x55 0x55
 *GetStatus (bool* status):     
 *XShellInt' ';-->addr:0x0000000000f83088:value=0
 *GetStatus' ' 0xf83088;
 *XShellInt' ';-->addr:0x0000000000f83088:value=1
 *Input string must end with '\n'
 *System command not support pipe and cd
 *Input exit command quit shell
 *Not support float or double
 *if want to call cpu1 function,you can use "TestAmpFuncCallMsgSend" + cpu1 function cmd,
 *like: TestAmpFuncCallMsgSend "TestSendSgi2Cpu0"
 *==============================================*/
S32 XshellInit(void)
{
    U32 ret;
    char cmd[128]={0};
    pthread_t pid = 0;
    sprintf(cmd,"readelf -s -w %s |grep -E 'FUNC | OBJECT' | awk '{print $2,$4,$8;}' 1> %sSymTbl.txt",Target,Path);
    system(cmd);
    ret = CreateSymTbl();
    if(RET_NOK == ret)
    {
        printf("file analyse error !\n");
        return RET_NOK;
    }
    CreateMsq(Target);
    tinysh_init(pid);
    return RET_OK;
}
