#include <client_manage.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define SERV_PORT 8080

static int MjpegInit(int *sockfd)
{
    //连接成功后向服务器发送GET请求报文，请求获取图像
    char buf[100];
    unsigned char recvbuf[1000];
    int recvlen;
	int sendlen;

    memset(buf,0x0,100);
    strcpy(buf,"GET /?action=stream\n");
    //ssize_t send(int sockfd, const void *buf, size_t len, int flags); flags为0时，与write是同等的
    sendlen = send(*sockfd,buf,strlen(buf),0);
    if(sendlen <= 0)
    {
        close(*sockfd);
        return -1;
    }    

    //如果我们不使用密码功能!则只需发送任意长度为小于2字节的字符串，如"f\n"
    memset(buf,0x0,100);
    strcpy(buf,"f\n");
    sendlen = send(*sockfd,buf,strlen(buf),0);
    if(sendlen <= 0)
    {
        close(*sockfd);
        return -1;
    }

    //从服务器接收回应报文 200 OK
    //ssize_t recv(int sockfd, void *buff, size_t nbytes, int flags); 
     recvlen = recv(*sockfd,recvbuf,999,0); 
     if(recvlen <= 0)
     {
         close(*sockfd);
         return -1;
     }else
     {
         recvbuf[recvlen] = '\0';
         printf("http header: %s\n", recvbuf);
     }

     return 0;    
}

//获取一帧数据长度，保存头部信息后的图像数据
static long int get_head_len(int *sockfd,char *freebuf,int *freelen)
{
    int recvlen;
    char recvbuf[1024];
    char* pos,*buff;
    long int videolen;
    while(1)
    {
        recvlen = recv(*sockfd,recvbuf,1024,0);
        if(recvlen <= 0)
        {
            close(*sockfd);
            return -1;
        }    
        //解析数据信息,判断是否是报文，获取一帧长度
        //printf("recvbuf is %s\n",recvbuf);
        pos = strstr(recvbuf,"Length:");
        if(pos != NULL)
        {
            pos = strchr(pos,':');
            pos++;
            videolen = atol(pos);
            printf("data length is %ld\n",videolen);
        }
        //跳出循环
        buff = strstr(recvbuf,"\r\n\r\n");
        if(buff != NULL)
            break;
    }
    buff += 4;
    *freelen = 1024 - (buff - recvbuf);
    //printf("free size is %d\n",*freelen);
    //拷贝剩余数据
    memcpy(freebuf,buff,*freelen);
    
    return videolen;
}

static long int get_for_one_frame(int *sockfd, char **pbuff, long int size)
{
    int irecvLen = 0, recvLen = 0;
	char recvbuf[1024];
	//如果一帧剩余数据size > 1024
	while(size > 0)
	{
		irecvLen = recv(*sockfd, recvbuf, (size > 1024)? 1024: size, 0);
		if (irecvLen <= 0)
			break;
        
        //recvLen最终等于size
		recvLen += irecvLen;
		size -= irecvLen;

		if(*pbuff == NULL)
		{
			*pbuff = (char *)malloc(recvLen);
			if(*pbuff == NULL)
				return -1;
		}
		else
		{
            //若pbuff不为空，则重新分配recvLen大的空间
			*pbuff = (char *)realloc(*pbuff, recvLen);
			if(*pbuff == NULL)
				return -1;
		}

		memcpy(*pbuff+recvLen-irecvLen, recvbuf, irecvLen);
	}

	return recvLen;
}
static int MjpegGetFrames(int *sockfd, PT_VideoBuf ptVideoBuf)
{
    //获取图像数据
    //服务器会先发送一个头部信息buffer，告诉客户端即将发送图片的格式和大小，然后紧接着发出一帧图片数据
    //Content-Type: image/jpeg\r\n Content-Length: %d\r\n\r\n"
    //需要将头部信息后的图像数据取出
	long int videolen, irecvlen;
    int flen = 0;
    char tmpbuf[1024];
    char *fbuf = NULL;

    while(1)
    {
        videolen = get_head_len(sockfd,tmpbuf,&flen);
        irecvlen = get_for_one_frame(sockfd,&fbuf,videolen - flen);
        //将两次获得的数据封装成一个
        //原子操作
        pthread_mutex_lock(&ptVideoBuf->db);
        memcpy(ptVideoBuf->tPixelDatas.aucPixelDatas,tmpbuf,flen);
        memcpy(ptVideoBuf->tPixelDatas.aucPixelDatas+flen,fbuf,irecvlen);
        ptVideoBuf->tPixelDatas.iTotalBytes = videolen;
        pthread_cond_broadcast(&ptVideoBuf->db_update);	    //唤醒阻塞在条件变量上的线程 
		pthread_mutex_unlock( &ptVideoBuf->db );			// 原子操作结束
    }
	
    return 0;
}

static int MjpegGetFmt(void)
{
    return V4L2_PIX_FMT_YUYV;
}

static int MjpegConnect(int *sockfd,const char* ip)
{
    int ret;
    struct sockaddr_in servaddr;
    *sockfd = socket(AF_INET,SOCK_STREAM,0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    ret = inet_aton(ip, &servaddr.sin_addr);
    if(ret == 0)
    {
        printf("fail to ip->addr.s_addr\n");
        return -1;
    }
    memset(servaddr.sin_zero, 0, 8);

    ret = connect(*sockfd,(const struct sockaddr *)&servaddr, sizeof(struct sockaddr));
    if(ret < 0)
    {
        printf("connect failed\n");
        return -1;
    }

    return 0;
}

static int MjpegDisconnect(int *sockfd)
{
    int ret;
    ret = close(*sockfd);
    if(ret == -1)
    {
        printf("close error\n");
        return ret;
    }
    return 0;
}

static struct ClientOpr CltOpr = {
    .name = "http",
    .ClientInit = MjpegInit,
    .GetFrames   = MjpegGetFrames,
    .GetFormat  = MjpegGetFmt,
    .Connect    = MjpegConnect,
    .DisConnect = MjpegDisconnect,
};

int ClientForInit()
{
	return RegisterClientoOpr(&CltOpr);
}