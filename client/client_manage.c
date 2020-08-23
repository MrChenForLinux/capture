#include <config.h>
#include <client_manage.h>
#include <string.h>

PT_ClientOpr g_ClientOprHead = NULL;

/* 注册操作函数 */
int RegisterClientoOpr(PT_ClientOpr clientOpr)
{
    if(g_ClientOprHead)
    {
        PT_ClientOpr tmp = g_ClientOprHead;
        while(tmp->ptNext)
            tmp = tmp->ptNext;
        tmp->ptNext = clientOpr;
        clientOpr->ptNext = NULL;
    }else
    {
        g_ClientOprHead = clientOpr;
        clientOpr->ptNext = NULL;
    }
    return 0;
}

/* 显示支持的操作集 */
int ShowClientOpr(void)
{
    PT_ClientOpr tmp = g_ClientOprHead;
    while(tmp)
    {
        printf("support %s \n",tmp->name);
        tmp = tmp->ptNext;
    }
    return 0;
}

/* 获取一个操作集 */
PT_ClientOpr GetClientOpr(char *ptname)
{
    PT_ClientOpr tmp = g_ClientOprHead;
    while(tmp)
    {
        if(strcmp(ptname, tmp->name) == 0)
            return tmp;
        tmp = tmp->ptNext;
    }
    return NULL;
}

int clientInitDevice()
{
	int ret;
	
	ret = ClientForInit();
	
	return ret;
}