#include <stdio.h>
#include "./pthread/main.h"
#include "./pthread/pthread_sendmsg.h"
#include "./pthread/pthread_modbus.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
//添加消息队列头文件
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//添加获取网卡所需的头文件
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>



#define ERR_EXIT(m) \
      do { \
         perror(m); \
         exit(EXIT_FAILURE); \
      } while (0)


extern void *pthread_sendmsg(void *arg);
extern void *pthread_modbus(void *fd);



//定义需要接收的结构体
struct header
{
	short type;
	char ip[16];
	short length;
	char tdata[526];
};
//定义消息定义
struct msbuf
{
	long type;
	char ip[16];
};


int shmid;
int flag;

int msqid;
int  conn;//已连接套接字
int client[8];

//int client[8];//client数组用来保存已连接的套接字

key_t msgkey;
pthread_mutex_t  mutex= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   cond= PTHREAD_COND_INITIALIZER;



void tcpinit();
void cominit();
void daiding();
// 消息队列结构体
struct msgstr  
{  
   long msgtype;  
   char msgtext[1024];   
}; 


//定义存储网卡名和ip的结构体
struct mac
{
	char name[10];
	char ipaddr[16];
}strmac;

struct mac macList[6];         //结构体数组，存储多个网卡信息

static int c=0;

void InitArray()
{
	int j;
	for(j = 0;j<6;j++)
	{
		strcpy(macList[j].name,"1");
		strcpy(macList[j].ipaddr,"1");
	}
}

void addList(char name[],char ip[])
{
	if(c == 6)
		printf("数组已满，无法存储网卡信息!\n");
	else
	{
		strcpy(macList[c].name,name);
		strcpy(macList[c].ipaddr,ip);
		c++;
	}

}

//获取网卡和IP函数
int get_local_ip(char *ip)
{
	struct ifaddrs *ifAddrStruct;
	void *tmpAddrPtr = NULL;
	getifaddrs(&ifAddrStruct);
	while(ifAddrStruct != NULL)
	{
		if(ifAddrStruct->ifa_addr->sa_family == AF_INET)
		{
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			inet_ntop(AF_INET,tmpAddrPtr,ip,INET_ADDRSTRLEN);
			if(strcmp(ifAddrStruct->ifa_name,"lo") != 0)
			{
				addList(ifAddrStruct->ifa_name,ip);
				//printf("%s IP Address:%s\n",ifAddrStruct->ifa_name,ip);
			}
		}
		ifAddrStruct = ifAddrStruct->ifa_next;
	}
	freeifaddrs(ifAddrStruct);
	return 0;
}


int main(int argc,char *argv[])
{
	InitArray();
	char ip[16];
	memset(ip,0,sizeof(ip));
	get_local_ip(ip);
	int count;
	for(count=0;count<6;count++)
	{
		if(strcmp(macList[count].name,"1") != 0)
		printf("macList[%d].name:%s,ip:%s\n",count,macList[count].name,macList[count].ipaddr);
	}

int listenfd; //被动套接字(文件描述符），即只可以accept, 监听套接字
    if ((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0)  
           ERR_EXIT("socket error");
 
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(503);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    /* servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); */
    /* inet_aton("127.0.0.1", &servaddr.sin_addr); */
    
    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt error");
 
    if (bind(listenfd, (struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");
 
    if (listen(listenfd, SOMAXCONN) < 0) //listen应在socket和bind之后，而在accept之前
        ERR_EXIT("listen error");
    
	struct sockaddr_in peeraddr; //传出参数
    socklen_t peerlen = sizeof(peeraddr); //传入传出参数，必须有初始值
    

//	int client[8];
//    int conn; // 已连接套接字(变为主动套接字，即可以主动connect)
    int i;
    int maxi = 0; //这次为了避免每次遍历到FD_SETSIZE，设置maxi,即client数组中最大的已连接套接字
    for (i = 0; i < 8; i++)//初始化已连接套接字数组
        client[i] = -1;
 
    int nready;
    int maxfd = listenfd;
    fd_set rset;
    fd_set allset;
    FD_ZERO(&rset);
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
 int st=0;
	pthread_t sendmsg_pthread;
    while (1) {
        rset = allset; /*这里rset和allset是一致的，由于rset是传入传出参数，
                        *select返回可能会改变rset的值，所以把套接字先保存在allset中，再复制到rset中
                        */
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);//返回值为检测到的事件个数
        if (nready == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("select error");
        } 
        if (nready == 0)
            continue;
 
        if (FD_ISSET(listenfd, &rset)) {
        
            conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen);  //accept不再阻塞
            if (conn == -1)
                ERR_EXIT("accept error");
            
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = conn;
                    if (i > maxi)
                        maxi = i;
                    break;
                } 
            }
            
            if (i == FD_SETSIZE) {
                fprintf(stderr, "too many clients\n");
                exit(EXIT_FAILURE);
            }
 	    	

//将网卡信息结构体数组发送给qt客户端
			char tmpbuf[1024];
			memcpy(tmpbuf,&macList,sizeof(macList));
			write(conn,tmpbuf,sizeof(macList));
			


//接受客户端发来的结构体，并将ip存入消息队列
			char buffer[1024];
			bzero(buffer,1024);
			struct header head;
			int res = recv(conn,buffer,sizeof(head),0);
			if(res < 0)
			{
					printf("recieve data failed!\n");
					break;
			}
			memset(&head,0x0,sizeof(head));
			memcpy(&head,buffer,sizeof(buffer));
			printf("type=%d,length=%d,ip=%s,tdata=%s\n",head.type,head.length,head.ip,head.tdata);
			//定义消息队列并将结构体放进消息队列
			int msgid;
			msgid = msgget(0x1000,IPC_CREAT | 0666);
			struct msbuf mb;
			mb.type = 1;
			strcpy(mb.ip,head.ip);
			int send = msgsnd(msgid,&mb,sizeof(mb.ip),0);
			if(send<0)
			{
					perror("write wrong!\n");
			}




	daiding();
            printf("recv connect ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr),
                ntohs(peeraddr.sin_port));
	 st=pthread_create(&sendmsg_pthread,0,pthread_sendmsg,NULL);
                if(st!=0)
                 {
                         printf("create server_thread is errno\n");
                 }



	
            FD_SET(conn, &allset);
            if (conn > maxfd)
                maxfd = conn;
 
            if (--nready <= 0)//第一次调用select的时候nready为1
                continue;
     	}   
 
        for (i = 0; i <= maxi; i++) {
            conn = client[i];
            if (conn == -1)
                continue;
 
            /*if (FD_ISSET(conn, &rset)) {
                
                char recvbuf[1024] = {0};
                int ret = read(conn, recvbuf, 1024);
                if (ret == -1)
                    ERR_EXIT("read error");
                else if (ret  == 0) { //客户端关闭 
                    printf("client close \n");
                    FD_CLR(conn, &allset);//客户端若关闭需要把conn从allset中删除
                    client[i] = -1;//同时删除客户端数组中的conn
                    close(conn);
                }
        
                fputs(recvbuf, stdout);
                write(conn, recvbuf, strlen(recvbuf));
                
                if (--nready <= 0)
                    break; 
            }*/
        }
 	
    }
//	tcpinit();
//	cominit();
	
	return 0;
}
void tcpinit()
{	
	int i;
	char buf[N]={};	
	int listenfd;//监听套接字
	int  acceptfd;//已连接套接字（变为主动套接字，即可主动connect）
	int epfd;//epoll函数文件描述符
	int client[FD_SETSIZE];//client数组用来保存已连接的套接字
	int maxi=0;//为了避免遍历到FD_SETSIZE，即client数组中最大的已连接套接字
	 for(i=0;i<FD_SETSIZE;i++)
		client[i]=-1;


        int nready;
      //  int maxfd = listenfd;
        fd_set rset;
        fd_set allset;
        FD_ZERO(&rset);
        FD_ZERO(&allset);
        FD_SET(listenfd, &allset);

	int ret_events;//epoll_wait()的返回值
	int msg_type;

	struct msgstr msgs;
	int cnt=0;
	int ret_value;

/*	int msqid,ret_value;
	key_t msgkey;  
	printf("11111\n");
        if((msgkey = ftok("./test",1))<0)  
        {  
           printf("ftok failed\n");  
            exit(EXIT_FAILURE);  
        }
	printf("2222\n");     
        printf("ftok ok ,msgkey = %d\n", msgkey);  */
	msqid=msgget(msgkey,IPC_EXCL);  //检查消息队列是否存在  
  	if(msqid < 0){  
    		msqid = msgget(msgkey,IPC_CREAT|0666);//创建消息队列  
    	if(msqid <0){  
    	printf("failed to create msq | errno=%d [%s]\n",errno,strerror(errno));  
   	 exit(-1);  
   		}  
	}
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		errlog("fail to socket");
	}

	//填充网络信息结构
	struct sockaddr_in server_addr,client_addr;
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(6666);
	server_addr.sin_addr.s_addr=inet_addr(("192.168.1.17"));
	socklen_t addrlen=sizeof(server_addr);

	//绑定网路信息结构体
	socklen_t clientlen = sizeof(client_addr); 
	if(bind(listenfd,(struct sockaddr*)&server_addr,addrlen)<0)
	{
		errlog("fail to bind");
		printf("bind id failed!!!\n");
	}


	//设置监听状态
	if(listen(listenfd,SOMAXCONN)<0)
	{
		errlog("fail  to listen");
	}
	//设置套接字接受超时时间
	struct timeval tv;
	tv.tv_sec=1;
	tv.tv_usec=0;
	int maxfd = listenfd;
	if(setsockopt(listenfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0)
	{
		perror("fail to setsockopt ");	
	}
 while (1) {
        rset = allset; /*这里rset和allset是一致的，由于rset是传入传出参数，
                        *select返回可能会改变rset的值，所以把套接字先保存在allset中，再复制到rset中
                        */
	printf("111111\n");
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);//返回值为检测到的事件个数
        /*if (nready == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("select error");
        }*/
 
	printf("11111111111111\r\n");            
        if (nready == 0)
            continue;
 
        if (FD_ISSET(listenfd, &rset)) {
        
            acceptfd = accept(listenfd, (struct sockaddr*)&client_addr, &clientlen);  //accept不再阻塞
            if (acceptfd == -1)
                ERR_EXIT("accept error");
            
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = acceptfd;
                    if (i > maxi)
                        maxi = i;
                    break;
                } 
            }
            if (i == FD_SETSIZE) {
                fprintf(stderr, "too many clients\n");
                exit(EXIT_FAILURE);
            }
 		
            printf("recv connect ip=%s port=%d\n", inet_ntoa(client_addr.sin_addr),
                ntohs(client_addr.sin_port));
 
            FD_SET(acceptfd, &allset);
            if (acceptfd > maxfd)
                maxfd = acceptfd;
 
            if (--nready <= 0)//第一次调用select的时候nready为1
                continue;
        }
 
        for (i = 0; i <= maxi; i++) {
            acceptfd = client[i];
            if (acceptfd == -1)
                continue;
 
            /*if (FD_ISSET(acceptfd, &rset)) {
                
                char recvbuf[1024] = {0};
                int ret = read(acceptfd, recvbuf, 1024);
                if (ret == -1)
                    ERR_EXIT("read error");
                else if (ret  == 0) { //客户端关闭 
                    printf("client close \n");
                    FD_CLR(acceptfd, &allset);//客户端若关闭需要把conn从allset中删除
                    client[i] = -1;//同时删除客户端数组中的conn
                    close(acceptfd);
                }
        
                fputs(recvbuf, stdout);
                write(acceptfd, recvbuf, strlen(recvbuf));
                
                if (--nready <= 0)
                    break; 
            }*/
        }
 
 
    }


/*truct msgstru
 10 {
 11    long msgtype;
 12    char msgtext[2048];
 13 };
	
	while(1)
	{

		//阻塞等待客户端连接请求
		if((acceptfd=accept(sockfd,(struct sockaddr *)&client_addr,&addrlen))<0)
		{
			if(errno==11)  //EAGAIN
			{
				printf("connect timeout.......\r\n");
			}
			else{
				//	printf("errno:%d\n",errno);
				errlog("fail to accept");
			}
		}
		else
		{
			printf("connect successed!!\n");												
	//	}
			while(1){ 
				memset(buf,0,sizeof(buf)); 
				cnt =recv(acceptfd, buf,N,0);
				if(cnt>0){	
					printf("read data is %s\n",buf);
				}
				msgs.msgtype =sizeof(buf);  
				strcpy(msgs.msgtext, buf);  
	    				// 发送消息队列   
	    				//ret_value = msgsnd(msqid,&msgs,sizeof(struct msgstr),IPC_NOWAIT);  
	    				if ( ret_value < 0 ) {  
	   			    		printf("msgsnd() write msg failed,errno=%d[%s]\n",errno,strerror(errno));  
	      			    		//exit(-1);  
	    			 		} 
				iif(strcmp(buf,"1")>0 ){
					printf("receive data is %s\n",buf);
					flag=1;
					printf("flag ==  %d\n",flag);
					memset(buf,0,sizeof(buf));	
					fgets(buf, N, stdin);
					send(acceptfd,buf,N,0);
				}
					printf("msgsend successed!!\n");
				
				printf("msgs.msgtext is %s\n",msgs.msgtext);
				// 发送消息队列 

				ret_value = msgsnd(msqid,&msgs,sizeof(struct msgstr),IPC_NOWAIT);
				if ( ret_value < 0 ) {
					printf("msgsnd() write msg failed,errno=%d[%s]\n",errno,strerror(errno));
					exit(-1);
				}
				sleep(10);
			}
		}	
	}	
	*/
/*	close(listenfd);
	close(acceptfd);
	msgctl(msqid,IPC_RMID,0);
	pthread_exit(0);*/
	//return 0;

}

void cominit()
{
	//创建共享内存
	shmid=shmget(IPC_PRIVATE,sizeof(char)*4092,IPC_CREAT|0666);
	printf("shmid: %d\n",shmid);


	msqid=msgget(msgkey,IPC_EXCL);  //检查消息队列是否存在  
  	if(msqid < 0){  
    		msqid = msgget(msgkey,IPC_CREAT|0666);//创建消息队列  
    	if(msqid <0){  
    	printf("failed to create msq | errno=%d [%s]\n",errno,strerror(errno));  
   	 exit(-1);  
   		}  
	}
}

void daiding()
{
	
	int fd[2];
	
	int st=0,mt=0;
		pthread_t modbus_pthread;
		//创建tcpserver线程

		//ftok获得key值
		if((msgkey = ftok("./test",1))<0)
		{
			printf("ftok failed\n");
			exit(EXIT_FAILURE);
		}	
	//	st=pthread_create(&server_pthread,0,pthread_server,NULL);
		if(st!=0)
		{
			printf("create server_thread is errno\n");
		}
		//创建modbus线程
		//mt=pthread_create(&modbus_pthread,0,pthread_modbus,NULL);
		mt=pthread_create(&modbus_pthread,0,pthread_modbus,NULL);
		if(mt!=0)
		{
			printf("create modbus_)thread is errno");
		}
		
		/*添加代码监听及读管道*/
		/*struct epoll_event ev;
		struct epoll_event events[MAXEVENTS];
		char readbuf[2048];   //创建读取管道数组
		if(pipe(fd)<0)         //判断管道是否创建成功
		{
			printf("create pipe error!\n");
		}
		ev.data.fd=fd[0];         //设置监听文件描述符
		ev.events=EPOLLIN|EPOLLET;
		int epfd=epoll_create(MAXEVENTS);
		int ret=epoll_ctl(epfd,EPOLL_CTL_ADD,fd[0],&ev);
		if(ret!=0)
		{
			printf("epoll_ctl fail\n");
			close(fd[1]);
			close(fd[0]);
			close(epfd);
			return -1;
		}
		int count = epoll_wait(epfd,events,MAXEVENTS,5000);
		int i;
		for(i=0;i<count;i++)
		{
			if((events[i].data.fd==fd[0])&&(events[0].events&EPOLLIN))
			{
				read(fd[0],readbuf,sizeof(readbuf));
				printf("read buf=%s\n",readbuf);
				send(conn,readbuf,strlen(readbuf),0);
				write(conn,readbuf,strlen(readbuf));
			}
		}
		close(fd[1]);
		close(fd[0]);
		close(epfd);*/
		

	//	pthread_create(&demo_pthread,0,pthread_demo,NULL);	

		printf("flag is == %d\n",flag);

//		pthread_cond_wait(&cond,&mutex);
	/*
	//pthread_create(&server_pthread,0,pthread_server,NULL);

	//	pthread_join(server_pthread,NULL);

	//	pthread_create(&demo_pthread,0,pthread_demo,NULL);


		while(1){
	//		printf("循环成功进入>>>>>>>\n");
			if(flag==1){
			printf("flag== %d\n",flag);
				pthread_create(&modbus_pthread,0,pthread_demo,NULL);
	//			pthread_join(demo_pthread,NULL);
			}
		sleep(50);
		printf("flag is == %d\n",flag);
		break;
		}*/
}


