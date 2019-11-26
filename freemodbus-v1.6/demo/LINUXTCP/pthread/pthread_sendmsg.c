#include "main.h"
#include "pthread_sendmsg.h"
#include <sys/epoll.h>
#include <stdbool.h>
#define MAXEVENTS 1024

extern int flag;
extern int shmid;
extern int msqid;
//extern int conn;
extern int client[8];

extern key_t msgkey;
int  fd,led_fd,buzzer_fd;
int pipe_fd[2];

void sendmsgs();
void tcp();

//发送结构体
struct sendhead
{
	short type;
	short length;
	char sdata[0];
};

struct msgstr  
{  
   long msgtype;  
   char msgtext[100];   
}; 
void *pthread_sendmsg(void *arg)
{
	
	sendmsgs();
}

void shmwrite(char *data){
	char *shmwrite;
	char wbuf[128]={0};
	if((shmwrite=shmat(shmid,NULL,01))==-1){
		printf("the modbus thread attach shmat is failed\n");
	}
	pthread_mutex_lock(&mutex);
	printf("read a string into shared memory\n");
	fgets(wbuf,128,stdin);
	pthread_mutex_unlock(&mutex);
}
void tcp(){
	char buf[N]={};
	//创建套接字
	int sockfd,acceptfd;
	
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
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
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
	if(bind(sockfd,(struct sockaddr*)&server_addr,addrlen)<0)
	{
		errlog("fail to bind");
		printf("bind id failed!!!\n");
	}


	//设置监听状态
	if(listen(sockfd,5)<0)
	{
		errlog("fail  to listen");
	}
	//设置套接字接受超时时间
	struct timeval tv;
	tv.tv_sec=5;
	tv.tv_usec=0;

	if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv))<0)
	{
		perror("fail to setsockopt ");	
	}
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
	    				/* 发送消息队列 */  
	    				/*ret_value = msgsnd(msqid,&msgs,sizeof(struct msgstr),IPC_NOWAIT);  
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
				*/
				printf("msgs.msgtext is %s\n",msgs.msgtext);
				/* 发送消息队列 */

				ret_value = msgsnd(msqid,&msgs,sizeof(struct msgstr),IPC_NOWAIT);
				if ( ret_value < 0 ) {
					printf("msgsnd() write msg failed,errno=%d[%s]\n",errno,strerror(errno));
					exit(-1);
				}
				sleep(10);
			}
		}	
	}	
	
	close(sockfd);
	close(acceptfd);
	msgctl(msqid,IPC_RMID,0);
	pthread_exit(0);
	//return 0;
}

void sendmsgs(){
                   
    struct msgstr msgs;
    int ret_value;
    int msgid;
	char buf[35]={"ni  zai  na \n"};
    while(1){
      /* msgid = msgget(0x2000,IPC_EXCL );
	
       if(msgid < 0){

          printf("msq not existed! errno=%d [%s]\n",errno,strerror(errno));
	           msgid = msgget(0x2000,IPC_CREAT|0666);//创建消息队列  
          if(msgid <0){
          printf("failed to create msq | errno=%d [%s]\n",errno,strerror(errno));

          sleep(2);
          continue;
          }
	}
	sleep(2);*/
       /*接收消息队列*/
       //ret_value = msgrcv(msgid,&msgs,sizeof(struct msgstr),0,0);
       //printf("text=[%s] pid=[%d]\n",msgs.msgtext,getpid());
	
	//write(conn,buf,strlen(buf));
	//write(conn,msgs.msgtext,strlen(msgs.msgtext));	
	//printf("线程2开启\n");
	/*添加代码监听及读管道*/
                 struct epoll_event ev;
                 struct epoll_event events[MAXEVENTS];
                 char readbuf[1024];   //创建读取管道数组
                 if(pipe(pipe_fd)<0)         //判断管道是否创建成功
                 {
                         printf("create pipe error!\n");
                 }
                 ev.data.fd=pipe_fd[0];         //设置监听文件描述符
                 ev.events=EPOLLIN|EPOLLET;
                 int epfd=epoll_create(MAXEVENTS);
                 int ret=epoll_ctl(epfd,EPOLL_CTL_ADD,pipe_fd[0],&ev);
                 if(ret!=0)
                 {
                         printf("epoll_ctl fail\n");
                         close(pipe_fd[1]);
                         close(pipe_fd[0]);
                         close(epfd);
                         return -1;
                 }
                 int count = epoll_wait(epfd,events,MAXEVENTS,5000);
                 int i;
                 for(i=0;i<count;i++)
                 {
                         if((events[i].data.fd==pipe_fd[0])&&(events[0].events&EPOLLIN))
                         {
								 close(pipe_fd[1]);
                                 read(pipe_fd[0],readbuf,sizeof(readbuf));
								 char p[16];
								 int i;
								 struct sockaddr_in peeraddr;
								 int len = sizeof(peeraddr);
								 for(i = 0;i<8;i++)
								 {
										 if(client[i] == -1)
												 break;
										 getpeername(client[i],(struct sockaddr *)&peeraddr,&len);
										 strncpy(p,readbuf,strlen(inet_ntoa(peeraddr.sin_addr)));
										 //printf("addr= %s,p = %s\n",inet_ntoa(peeraddr.sin_addr),p);
										 if(strcmp(inet_ntoa(peeraddr.sin_addr),p) == 0)
										 {
												 struct sendhead *sh = (struct snedhead *)malloc(sizeof(struct sendhead)+sizeof(readbuf));
												 sh->length = sizeof(readbuf);
												 sh->type = 2;
												 memcpy(sh->sdata,&readbuf,sizeof(readbuf));
												 char tmpbuf[1052];
												 memcpy(tmpbuf,sh,sizeof(struct sendhead)+sizeof(readbuf));
												 write(client[i],tmpbuf,sizeof(tmpbuf));
										 }
								 }
								 //write(conn,readbuf,strlen(readbuf));
                         }
                 }
                 close(pipe_fd[1]);
                 close(pipe_fd[0]);
                 close(epfd);


    }
}
