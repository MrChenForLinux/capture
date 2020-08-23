#ifndef _CLIENT_MANAGE_H_
#define _CLIENT_MANAGE_H_

#include <config.h>
#include <pic_operation.h>
#include <linux/videodev2.h>
#include <pthread.h>

struct ClientOpr;
typedef struct ClientOpr T_ClientOpr, * PT_ClientOpr;

/* 存放获得数据的结构体*/
typedef struct ClientBuf {
    T_PixelDatas tPixelDatas;                //存放数据
    int iPixelFormat;                        //图片格式
    pthread_mutex_t db;                      //线程锁
    pthread_cond_t  db_update;               //条件变量
}T_VideoBuf, *PT_VideoBuf;

/* 操作函数结构体 */
struct ClientOpr {
    char *name;
    int (*ClientInit)(int *sockfd);
    int (*GetFrames)(int *sockfd, PT_VideoBuf ptVideoBuf);
    int (*GetFormat)(void);
    int (*Connect)(int *sockfd,const char* ip);
    int (*DisConnect)(int *sockfd);
    struct ClientOpr *ptNext;
};

int RegisterClientoOpr(PT_ClientOpr clientOpr);
int ShowClientOpr(void);
PT_ClientOpr GetClientOpr(char *ptname);
int clientInitDevice();
int ClientForInit();

#endif 