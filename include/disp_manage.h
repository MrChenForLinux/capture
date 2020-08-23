#ifndef _DISP_MANAGER_H
#define _DISP_MANAGER_H

#include <pic_operation.h>
#include <client_manage.h>

//��ʾ���������Ͻ����꣬���½����꣬����ͼ�꣬strIconName�����ļ���
typedef struct Layout {
	int iTopLeftX;
	int iTopLeftY;
	int iBotRightX;
	int iBotRightY;
	char *strIconName;
}T_Layout, *PT_Layout;


// VideoMem��״̬:���С�Ԥ����ʾ����ǰ��ʾ
//��ʾ����ʱ������׼��һ��VideoMem�����ڴ洢��һ����Ҫ��ʾ�����ݣ���ʾʱ�ٰ�VideoMem�е����ݸ��Ƶ��豸���Դ��ϣ��ӿ��ٶ�
typedef enum {
	VMS_FREE = 0,
	VMS_USED_FOR_PREPARE,
	VMS_USED_FOR_CUR,	
}E_VideoMemState;

// VideoMem���ڴ���ͼƬ��״̬:�հס��������ɡ��Ѿ�����
typedef enum {
	PS_BLANK = 0,
	PS_GENERATING,
	PS_GENERATED,	
}E_PicState;


typedef struct VideoMem {
	int iID;                        /* IDֵ,���ڱ�ʶ��ͬ��ҳ�� */
	int bDevFrameBuffer;            /* 1: ���VideoMem����ʾ�豸���Դ�; 0: ֻ��һ����ͨ���� */
	E_VideoMemState eVideoMemState; /* ���VideoMem��״̬ */
	E_PicState ePicState;           /* VideoMem���ڴ���ͼƬ��״̬ */
	T_PixelDatas tPixelDatas;       /* �ڴ�: �����洢ͼ�� */
	struct VideoMem *ptNext;        /* ���� */
}T_VideoMem, *PT_VideoMem;

typedef struct DispOpr {
	char *name;              /* ��ʾģ������� */
	int iXres;               /* X�ֱ��� */
	int iYres;               /* Y�ֱ��� */
	int iBpp;                /* һ�������ö���λ����ʾ */
	int iLineWidth;          /* һ������ռ�ݶ����ֽ� */
	unsigned char *pucDispMem;   /* �Դ��ַ */
	int (*DeviceInit)(void);     /* �豸��ʼ������ */
	int (*ShowPixel)(int iPenX, int iPenY, unsigned int dwColor);    /* ��ָ�������������Ϊĳ��ɫ */
	int (*CleanScreen)(unsigned int dwBackColor);                    /* ����Ϊĳ��ɫ */
	int (*ShowPage)(PT_PixelDatas ptPixelDatas);                         /* ��ʾһҳ,����Դ��ptVideoMem */
	struct DispOpr *ptNext;      /* ���� */
}T_DispOpr, *PT_DispOpr;

//ע����ʾ�豸����	
int RegisterDispOpr(PT_DispOpr ptDispOpr);

//��ʾ֧�ֵ���ʾ�豸
void ShowDispOpr(void);

//ע��ģ��
int DisplayInit(void);

//��������ȡ����ʾģ�飬����ʼ��������
void SelectAndInitDefaultDispDev(char *name);

//���ѡ�����ʾģ��
PT_DispOpr GetDefaultDispDev(void);

//�����ʾ�豸�ķֱ���
int GetDispResolution(int *piXres, int *piYres, int *piBpp);

//ΪVideoMem�����ڴ�
int AllocVideoMem(int iNum);

//����豸�Դ�
PT_VideoMem GetDevVideoMem(void);

//���һ��ɲ�����VideoMem(�����ڴ洢Ҫ��ʾ������),�������PutVideoMem���ͷ�         
//	iID  - IDֵ,�ȳ������ڶ�VideoMem���ҵ�IDֵ��ͬ��
//	bCur - 1��ʾ��ǰ��������Ҫʹ��VideoMem,�޷���ζ�Ҫ����һ��VideoMem
//	       0��ʾ����Ϊ�˸Ľ����ܶ���ǰȡ��VideoMem,���Ǳ����
PT_VideoMem GetVideoMem(int iID, int bCur);

//�ͷ�VideoMem
void PutVideoMem(PT_VideoMem ptVideoMem);

//����Ϊĳ��ɫ
void ClearVideoMem(PT_VideoMem ptVideoMem, unsigned int dwColor);

//ָ����������Ϊĳ��ɫ
void ClearVideoMemRegion(PT_VideoMem ptVideoMem, PT_Layout ptLayout, unsigned int dwColor);

//ע��framebuffer��ʾ�ɱ�
int FBInit(void);

//ע��crt��ʾ�豸
int CRTInit(void);

int GetVideoBufForDisplay(PT_VideoBuf ptFrameBuf);
void FlushPixelDatasToDev(PT_PixelDatas ptPixelDatas);

#endif /* _DISP_MANAGER_H */

