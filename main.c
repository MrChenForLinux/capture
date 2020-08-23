#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include <convert_manage.h>
#include <client_manage.h>
#include <disp_manage.h>
#include <render.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

PT_ClientOpr Clientopr;
int sockfd;

extern int PicMerge(int iX, int iY, PT_PixelDatas ptSmallPic, PT_PixelDatas ptBigPic);

void *RecvVideoThread(void *data)
{
	if(Clientopr->GetFrames(&sockfd, (PT_VideoBuf)data) < 0)
	{		
		printf("can not Get_Video\n");
	}

	return data;
}

/* main ip */
int main(int argc, char **argv)
{	
	int ret;
	int iPixelFormatOfVideo;
	int iPixelFormatOfDisp;
    PT_VideoConvert ptVideoConvert; 

	T_VideoBuf iConvertBuf;
	T_VideoBuf iFrameBuf;
	T_VideoBuf pClientBuf; 
	PT_VideoBuf piVideoBufCur; 

	int iXres,iYres,iBpp;
	int iTopLeftX,iTopLeftY;
    	
	pthread_t tid;
    
    if (argc != 2)
    {
        printf("Please input correct parameters\n");
        return -1;
    }
    
	
	//显示设备注册
   	DisplayInit();
	SelectAndInitDefaultDispDev("crt");
	GetDispResolution(&iXres, &iYres, &iBpp);
	GetVideoBufForDisplay(&iFrameBuf);
	iPixelFormatOfDisp = iFrameBuf.iPixelFormat;
	
	//客户端初始化
	clientInitDevice();
	Clientopr = GetClientOpr("http");
    iPixelFormatOfVideo = Clientopr->GetFormat();
	
    VideoConvertInit();
    ptVideoConvert = GetVideoConvertForFormats(iPixelFormatOfVideo, iPixelFormatOfDisp);
	printf("%s\n",ptVideoConvert->name);
    if (NULL == ptVideoConvert)
    {
        printf("can not support this format convert\n");
        return -1;
    }
	
	//连接
	if(Clientopr->Connect(&sockfd, argv[1]) < 0)
	{		
		printf("can not Connect to Server\n");
		return -1;
	}

	if(Clientopr->ClientInit(&sockfd) < 0)
	{
		printf("can not Init Client\n");
		return -1;
	}
		
	memset(&pClientBuf,0,sizeof(pClientBuf));
	pClientBuf.tPixelDatas.aucPixelDatas = malloc(30000);
	memset(&iConvertBuf, 0, sizeof(iConvertBuf));
    iConvertBuf.iPixelFormat     = iPixelFormatOfDisp;
    iConvertBuf.tPixelDatas.iBpp = iBpp;
    
	if( pthread_mutex_init(&pClientBuf.db, NULL) != 0 )		/* 初始化 pClientBuf.db 成员 */
	{
		return -1;
	}
	if( pthread_cond_init(&pClientBuf.db_update, NULL) != 0 )	/* 初始化 pClientBuf.db_update(条件变量) 成员 */
	{
		printf("could not initialize condition variable\n");
		return -1;
	}
	
	//创建线程获取摄像头数据
	pthread_create(&tid, NULL, &RecvVideoThread, &pClientBuf);
		
	while (1)
	{
		pthread_cond_wait(&pClientBuf.db_update, &pClientBuf.db);
		piVideoBufCur = &pClientBuf;
		//获取到数据，开始转换
		if (iPixelFormatOfVideo != iPixelFormatOfDisp)
		{
			// 转换为RGB 
			ret  = ptVideoConvert->Convert(&pClientBuf, &iConvertBuf);
			if (ret)
			{
				printf("Convert for %s error!\n", argv[1]);
				continue;
			}            
			piVideoBufCur = &iConvertBuf;
		}		
		
		
		// 算出居中显示时左上角坐标
		iTopLeftX = (iXres - piVideoBufCur->tPixelDatas.iWidth) / 2;
		iTopLeftY = (iYres - piVideoBufCur->tPixelDatas.iHeight) / 2;
		
		//复制到显存，显示
		PicMerge(iTopLeftX, iTopLeftY, &piVideoBufCur->tPixelDatas, &iFrameBuf.tPixelDatas);
		FlushPixelDatasToDev(&iFrameBuf.tPixelDatas);
		
    }
	//线程分离
	pthread_detach(tid);
	
	return 0;
}