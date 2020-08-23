
#include <config.h>
#include <convert_manage.h>
#include <string.h>

static PT_VideoConvert g_ptVideoConvertHead = NULL;

//ע��
int RegisterVideoConvert(PT_VideoConvert ptVideoConvert)
{
	PT_VideoConvert ptTmp;

	if (!g_ptVideoConvertHead)
	{
		g_ptVideoConvertHead   = ptVideoConvert;
		ptVideoConvert->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptVideoConvertHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext     = ptVideoConvert;
		ptVideoConvert->ptNext = NULL;
	}

	return 0;
}

//��ʾһ��ģ��
void ShowVideoConvert(void)
{
	int i = 0;
	PT_VideoConvert ptTmp = g_ptVideoConvertHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

//���һ��ģ��
PT_VideoConvert GetVideoConvert(char *pcName)
{
	PT_VideoConvert ptTmp = g_ptVideoConvertHead;
	
	while (ptTmp)
	{
		if (strcmp(ptTmp->name, pcName) == 0)
		{
			return ptTmp;
		}
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

//���ݸ�ʽѡ��ģ��
PT_VideoConvert GetVideoConvertForFormats(int iPixelFormatIn, int iPixelFormatOut)
{
	PT_VideoConvert ptTmp = g_ptVideoConvertHead;
	
	while (ptTmp)
	{
        if (ptTmp->isSupport(iPixelFormatIn, iPixelFormatOut))
        {
            return ptTmp;
        }
		ptTmp = ptTmp->ptNext;
	}
	return NULL;
}

//��ʼ��
int VideoConvertInit(void)
{
	int iError;

    iError = Yuv2RgbInit();
    iError |= Mjpeg2RgbInit();
    iError |= Rgb2RgbInit();

	return iError;
}





