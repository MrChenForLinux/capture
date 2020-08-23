#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

#include <pic_operation.h>
#include <client_manage.h>

//显示的区域，左上角坐标，右下角坐标，若有图标，strIconName即是文件名
typedef struct Layout {
	int iTopLeftX;
	int iTopLeftY;
	int iBotRightX;
	int iBotRightY;
	char *strIconName;
}T_Layout, *PT_Layout;


// VideoMem的状态:空闲、预备显示、当前显示
//显示内容时，我们准备一个VideoMem，用于存储下一个将要显示的内容，显示时再把VideoMem中的数据复制到设备的显存上，加快速度
typedef enum {
	VMS_FREE = 0,
	VMS_USED_FOR_PREPARE,
	VMS_USED_FOR_CUR,	
}E_VideoMemState;

// VideoMem中内存里图片的状态:空白、正在生成、已经生成
typedef enum {
	PS_BLANK = 0,
	PS_GENERATING,
	PS_GENERATED,	
}E_PicState;


typedef struct VideoMem {
	int iID;                        /* ID值,用于标识不同的页面 */
	int bDevFrameBuffer;            /* 1: 这个VideoMem是显示设备的显存; 0: 只是一个普通缓存 */
	E_VideoMemState eVideoMemState; /* 这个VideoMem的状态 */
	E_PicState ePicState;           /* VideoMem中内存里图片的状态 */
	T_PixelDatas tPixelDatas;       /* 内存: 用来存储图像 */
	struct VideoMem *ptNext;        /* 链表 */
}T_VideoMem, *PT_VideoMem;

typedef struct DispOpr {
	char *name;              /* 显示模块的名字 */
	int iXres;               /* X分辨率 */
	int iYres;               /* Y分辨率 */
	int iBpp;                /* 一个象素用多少位来表示 */
	int iLineWidth;          /* 一行数据占据多少字节 */
	unsigned char *pucDispMem;   /* 显存地址 */
	int (*DeviceInit)(void);     /* 设备初始化函数 */
	int (*ShowPixel)(int iPenX, int iPenY, unsigned int dwColor);    /* 把指定坐标的象素设为某颜色 */
	int (*CleanScreen)(unsigned int dwBackColor);                    /* 清屏为某颜色 */
	int (*ShowPage)(PT_PixelDatas ptPixelDatas);                         /* 显示一页,数据源自ptVideoMem */
	struct DispOpr *ptNext;      /* 链表 */
}T_DispOpr, *PT_DispOpr;

//注册显示设备操作	
int RegisterDispOpr(PT_DispOpr ptDispOpr);

//显示支持的显示设备
void ShowDispOpr(void);

//注册模块
int DisplayInit(void);

//根据名字取出显示模块，并初始化，清屏
void SelectAndInitDefaultDispDev(char *name);

//获得选择的显示模块
PT_DispOpr GetDefaultDispDev(void);

//获得显示设备的分辨率
int GetDispResolution(int *piXres, int *piYres, int *piBpp);

//为VideoMem分配内存
int AllocVideoMem(int iNum);

//获得设备显存
PT_VideoMem GetDevVideoMem(void);

//获得一块可操作的VideoMem(它用于存储要显示的数据),用完后用PutVideoMem来释放         
//	iID  - ID值,先尝试在众多VideoMem中找到ID值相同的
//	bCur - 1表示当前程序马上要使用VideoMem,无法如何都要返回一个VideoMem
//	       0表示这是为了改进性能而提前取得VideoMem,不是必需的
PT_VideoMem GetVideoMem(int iID, int bCur);

//释放VideoMem
void PutVideoMem(PT_VideoMem ptVideoMem);

//清屏为某颜色
void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor);

//指定区域清屏为某颜色
void ClearVideoMemRegion(PT_VideoMem ptVideoMem, PT_Layout ptLayout, unsigned int dwColor);

//注册framebuffer显示仙贝
int FBInit(void);

//注册crt显示设备
int CRTInit(void);

int GetVideoBufForDisplay(PT_VideoBuf ptFrameBuf);
void FlushPixelDatasToDev(PT_PixelDatas ptPixelDatas);

#endif /* _DISP_MANAGER_H */

