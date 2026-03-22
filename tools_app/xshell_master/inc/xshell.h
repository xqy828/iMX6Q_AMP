#ifndef __XSHELL_H__
#define __XSHELL_H__

#define CMD_BUF_LEN (128)
#define STRING(arg) #arg
#define COMBINE(x,y) x##y

#define  TRUE   true
#define  FALSE  false

#define DEF_ENABLED     TRUE 
#define DEF_DISABLED    FALSE

#define TERMINAL_CFG_HISTORY_EN             DEF_ENABLED
#define TERMINAL_CFG_HISTORY_ITEMS_NBR      16 /* cfg nbr history items*/
#define TERMINAL_CFG_HISTORY_ITEM_LEN       64 /* cfg history item len*/

/*
typedef S32 (SHELL_CMD_FUNC*)(U32 argc, char *argc[], SHELL_OUT_FUNC out_func,SHELL_CMD_PARAM *pCmdParam);

typedef struct shell_cmd{
    const char *Name;
    SHELL_CMD_FUNC Func;
}SHELL_CMD;

*/

typedef struct ReadLineStatus
{
    unsigned int ulInputfd;
    unsigned int ulOutputfd;
    unsigned int ulBufLen;
    unsigned int ulPromptLen;
    unsigned int ulCurrentCursorPos;
    unsigned int ulPreCursorPos;
    unsigned int ulHistoryIndex;
    unsigned int ulLen;
    char *buf;
    const char *Prompt;

}TerminalReadStatus;

#endif
