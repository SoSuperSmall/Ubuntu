#include <stdio.h>   
#include <sys/types.h>   
#include <sys/ipc.h>   
#include <sys/msg.h>   
#include <errno.h>   

#define MSGKEY 1024   

struct msgstru  
{  
   long msgtype;  
   char msgtext[2048];  
};

int main(){  

/*子进程，监听消息队列*/   
  struct msgstru msgs;  
  int ret_value;
  int msgid; 
  while(1){  
     msgid = msgget(0x2000,IPC_EXCL );/*检查消息队列是否存在 */  
     if(msgid < 0){  
        printf("msq not existed! errno=%d [%s]\n",errno,strerror(errno));  
        sleep(2);  
        continue;  
     }  
     /*接收消息队列*/  
     ret_value = msgrcv(msgid,&msgs,sizeof(struct msgstru),0,0);  
     printf("text=[%s] pid=[%d]\n",msgs.msgtext,getpid());  
  }  
  return;  
} 
