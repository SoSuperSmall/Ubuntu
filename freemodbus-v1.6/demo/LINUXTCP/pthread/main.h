#ifndef _MAIN_H_
#define _MAIN_H_
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h> //exit
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> //struct sockaddr_in
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include<errno.h>
#include<signal.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<stdlib.h>
#include<arpa/inet.h> //inet_addr htons
#include<sys/socket.h> //socket bind listen accept connect
#include<netinet/in.h> //sockaddr_in
#include<semaphore.h>
#define N 128 
//#define MSGKEY 1023



void init();//初始化函数
void tcp_cmd();//tcp命令解析函数

pthread_mutex_t  mutex;
pthread_cond_t   cond;
int flag;
int shmid;
int msqid;
int  conn;
int client[8];
key_t msgkey;






#endif
