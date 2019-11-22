/*
 * FreeModbus Libary: Win32 Demo Application
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

 /**********************************************************
 *	Linux TCP support.
 *	Based on Walter's project. 
 *	Modified by Steven Guo <gotop167@163.com>
 ***********************************************************/

/* ----------------------- Standard C Libs includes --------------------------*/
/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>*/

/* ----------------------- Modbus includes ----------------------------------*/
/*#include "mb.h"
#include "mbport.h"
#include "mbfunc.h"*/

/* ----------------------- Defines ------------------------------------------*/
/*#define PROG            "freemodbus"

#define REG_INPUT_START 1000
#define REG_INPUT_NREGS 4
#define REG_HOLDING_START 2000
#define REG_HOLDING_NREGS 130*/

/* ----------------------- Static variables ---------------------------------*/
/*static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];
static pthread_mutex_t xLock = PTHREAD_MUTEX_INITIALIZER;
static enum ThreadState
{
    STOPPED,
    RUNNING,
    SHUTDOWN
} ePollThreadState;*/

/* ----------------------- Static functions ---------------------------------*/
/*static BOOL     bCreatePollingThread( void );
static enum ThreadState eGetPollingThreadState( void );
static void     eSetPollingThreadState( enum ThreadState eNewState );
static void* pvPollingThread( void *pvParameter );

static eMBException eMBFuncReadCoils( UCHAR * pucFrame, USHORT * usLen );
*/
/* ----------------------- Start implementation -----------------------------*/
/*typedef struct stMbTcpInit{
	USHORT usport;
	USHORT usMode;
	UCHAR *pucAddr;
}MB_TCP_INIT_S;*/
#include "main.h"
#include "pthread_modbus.h"

extern int shmid;
extern int msqid;
extern key_t msgkey;
/*
int cmd_parse(int argc, char *argv[], MB_TCP_INIT_S *pstInfo)
{
	int rt= 0;
	int ci;
	char ch;
	char *s;	

	for (ci=0; ci< argc; ci++)
	{
		s = argv[ci];
		printf("\r\n arg cmd: [%d] %s  ", ci, s);
		ch =s[1];
		if (ch == '-') 
		{
			ci++;
			printf("  ii%c ",  s[2]);
			switch(s[2])
			{
				case 'p':
					s = argv[ci];
					pstInfo->usport = atoi(s);
					break;
				case 'n':
					s = argv[ci];
					pstInfo->pucAddr = s;
					break;
				default:
					printf("\r\n error cmd");
					rt = -1;
					break;
			}
		}
		printf("\r\n");
	}

	return rt;
}
*/ 

struct msgstr{
	long msgtype;
	char msgtext[2048];
};

//从共享内存读数据
char *shmread(char *data){
      char *shmread;
      char wbuf[128]={0};
      if((int)(shmread=shmat(shmid,NULL,0))==-1){
                printf("the server thread attach shmat is failed\n");
        }
       pthread_mutex_lock(&mutex);
       printf("write a string into shared memory\n");
       fgets(wbuf,128,stdin);
       pthread_mutex_unlock(&mutex);
}
//命令行模式执行测试
void *cmdtest(){
    int             iExitCode;
    int 	      rt = MB_ENOERR;
    CHAR           cCh;
    int            tfcode;	
    BOOL            bDoExit;
    MB_TCP_INIT_S stInfo = {0};
/*   if (argc >=3)
    {
          rt = cmd_parse(argc, argv, &stInfo);
	   if (MB_ENOERR != rt)
          {
               fprintf( stderr, "%s: command parse failed \r\n", PROG );
               rt = eMBTCPInit( MB_TCP_PORT_USE_DEFAULT, NULL);
      	   }
	   else
     	  {
     	       fprintf( stderr, " serveraddr %s port %d \r\n", stInfo.pucAddr, stInfo.usport );
     	 	rt = eMBTCPInit( stInfo.usport, stInfo.pucAddr);
     	  }
    }*/
   // else
   // {
    	rt = eMBTCPInit( MB_TCP_PORT_USE_DEFAULT, NULL);
  // }
	
    if(rt  != MB_ENOERR )
    {
        fprintf( stderr, "%s: can't initialize modbus stack!\r\n", PROG );
        iExitCode = EXIT_FAILURE;
    }
    else
    {
        eSetPollingThreadState( STOPPED );
        if( bCreatePollingThread( ) != TRUE ) 
         {
               printf(  "Can't start protocol stack! Already running?\r\n" ); 
          }

        /* CLI interface. */
        printf(  "Type 'q' for quit or 'h' for help!\r\n"  );
        bDoExit = FALSE;
        do
        {
            printf(  "> "  );
            cCh = getchar(  );
            switch ( cCh )
            {
            case  'q' :
                bDoExit = TRUE;
                break;
            case  'd' :
                eSetPollingThreadState( SHUTDOWN );
                break;
         /*   case  'e' :
                if( bCreatePollingThread(  ) != TRUE )
                {
                    printf(  "Can't start protocol stack! Already running?\r\n"  );
                }
                break;*/
            case  's' :
                switch ( eGetPollingThreadState(  ) )
                {
                case RUNNING:
                    printf(  "Protocol stack is running.\r\n"  );
                    break;
                case STOPPED:
                    printf(  "Protocol stack is stopped.\r\n"  );
                    break;
                case SHUTDOWN:
                    printf(  "Protocol stack is shuting down.\r\n"  );
                    break;
                }
		break;
            case  'h':
                printf(  "FreeModbus demo application help:\r\n" );
                printf(  "  'd' ... disable protocol stack.\r\n"  );
                printf(  "  'e' ... enabled the protocol stack\r\n"  );
                printf(  "  's' ... show current status\r\n"  );
                printf(  "  'q' ... quit applicationr\r\n"  );
                printf(  "  'h' ... this information\r\n"  );
	//	printf(  "  't' ... test function code\r\n"  );
                printf(  "\r\n"  );
                printf(  "Copyright 2007 Steven Guo <gotop167@163.com>\r\n"  );
                break;
            default:
                if( cCh != '\n' )
                {
                    printf(  "illegal command '%c'!\r\n" , cCh );
                }
                break;
            }

            /* eat up everything untill return character. */
            while( cCh != '\n' )
            {
                cCh = getchar(  );
            }
        }
        while( !bDoExit );

        /* Release hardware resources. */
        ( void )eMBClose(  );
        iExitCode = EXIT_SUCCESS;
    }
    return iExitCode;

}
//qt界面控制执行测试
void *qttest(void *arg)
{
    int     
        iExitCode;
    int 	      rt = MB_ENOERR;
    CHAR           cCh;
    int            tfcode;	
    BOOL            bDoExit;
    MB_TCP_INIT_S stInfo = {0};
/*   if (argc >=3)
    {
          rt = cmd_parse(argc, argv, &stInfo);
	   if (MB_ENOERR != rt)
          {
               fprintf( stderr, "%s: command parse failed \r\n", PROG );
               rt = eMBTCPInit( MB_TCP_PORT_USE_DEFAULT, NULL);
      	   }
	   else
     	  {
     	       fprintf( stderr, " serveraddr %s port %d \r\n", stInfo.pucAddr, stInfo.usport );
     	 	rt = eMBTCPInit( stInfo.usport, stInfo.pucAddr);
     	  }
    }*/
   // else
   // {
    	rt = eMBTCPInit( MB_TCP_PORT_USE_DEFAULT, NULL);
  // }
	
    if(rt  != MB_ENOERR )
    {
        fprintf( stderr, "%s: can't initialize modbus stack!\r\n", PROG );
        iExitCode = EXIT_FAILURE;
    }
    else
    {
        eSetPollingThreadState( STOPPED );
        /* CLI interface. */
        printf(  "Type 'q' for quit or 'h' for help!\r\n"  );
        bDoExit = FALSE;
        do
        {
            printf(  "> "  );
            cCh = getchar(  );
            switch ( cCh )
            {
            case  'q' :
                bDoExit = TRUE;
                break;
            case  'd' :
                eSetPollingThreadState( SHUTDOWN );
                break;
            case  'e' :
                if( bCreatePollingThread(  ) != TRUE )
                {
                    printf(  "Can't start protocol stack! Already running?\r\n"  );
                }
                break;
            case  's' :
                switch ( eGetPollingThreadState(  ) )
                {
                case RUNNING:
                    printf(  "Protocol stack is running.\r\n"  );
                    break;
                case STOPPED:
                    printf(  "Protocol stack is stopped.\r\n"  );
                    break;
                case SHUTDOWN:
                    printf(  "Protocol stack is shuting down.\r\n"  );
                    break;
                }
		break;
            case  'h':
                printf(  "FreeModbus demo application help:\r\n" );
                printf(  "  'd' ... disable protocol stack.\r\n"  );
                printf(  "  'e' ... enabled the protocol stack\r\n"  );
                printf(  "  's' ... show current status\r\n"  );
                printf(  "  'q' ... quit applicationr\r\n"  );
                printf(  "  'h' ... this information\r\n"  );
	//	printf(  "  't' ... test function code\r\n"  );
                printf(  "\r\n"  );
                printf(  "Copyright 2007 Steven Guo <gotop167@163.com>\r\n"  );
                break;
            default:
                if( cCh != '\n' )
                {
                    printf(  "illegal command '%c'!\r\n" , cCh );
                }
                break;
            }

            /* eat up everything untill return character. */
            while( cCh != '\n' )
            {
                cCh = getchar(  );
            }
        }
        while( !bDoExit );

        /* Release hardware resources. */
        ( void )eMBClose(  );
        iExitCode = EXIT_SUCCESS;
    }
    return iExitCode;
}
//pthread_modbus线程执行函数  
void *pthread_modbus(void *arg){
	//cmdtest();
	struct msgstr msgs;  
  	int ret_value;  
	
  	char *str=NULL; 
	char *xieyi=NULL;
    	char *delim =" ";
	char *p=NULL;
	char *string=NULL;
	char *gongneng=NULL;

	printf("pthread_modbus!!!!!\n");
/*        key_t msgkey;
       if((msgkey = ftok("./test",1))<0)
        {
            printf("ftok failed\n");
            exit(EXIT_FAILURE);
        }
	printf("ftok ok,msgkey =%d\n");*/
	sleep(2);
   	while(1){
	
//	memset(string,0,sizeof(*string));
//	memset(p,0,sizeof(*p));
 
       	msqid = msgget(msgkey,IPC_EXCL );/*检查消息队列是否存在 */
       	if(msqid < 0){
          printf("msq not existed! errno=%d [%s]\n",errno,strerror(errno));
	}
       	/*接收消息队列*/
	sleep( 2);
       	ret_value = msgrcv(msqid,&msgs,sizeof(struct msgstr),0,0);
       	printf("text=[%s] pid=[%d]\n",msgs.msgtext,getpid());
       
	string=strdup(msgs.msgtext);
	printf("dfsgdf==%s\r\n",string);
	sleep(2);
	//for(
//	p=strsep(&string,delim);
	//p!=NULL;p=strsep(&string,delim)){
//	xieyi=strdup(p);	
//	printf("%s\n",xieyi);
//	p=strsep(&string,delim);
//	gongneng=strdup(p);	
//	printf("%s\n",gongneng);
	//p=strsep(&string,delim);
	//string=strdup(p);
	//printf("%S\n",string);
	//}
//	if(strcmp(xieyi,"02")==0)
//	{
		cmdtest();
//	}	
//	printf("msg  is %s\r\n",p);
	/*if(strcmp(p,"02")==0){

                 printf("text=[%s] pid=[%d]\n",msgs.msgtext,getpid());  
  	 }    
        printf("nizai shui shenm \n") ;
*/	
	
   } 
/*

  	while(1){  

     		msqid = msgget(msgkey,IPC_EXCL );//检查消息队列是否存在 
     		if(msqid < 0){  
     		printf("msq not existed! errno=%d [%s]\n",errno,strerror(errno));  
        	sleep(2);  
        	//continue;  
     	}  
     	//接收消息队列  
     	ret_value = msgrcv(msqid,&msgs,sizeof(struct msgstr),0,0);  
	
	str[512]=msgs.msgtext;
	p=strsep(&str,"");
	if(strcmp(p,"02")){
     		printf("text=[%s] pid=[%d]\n",msgs.msgtext,getpid());  
  	}    
	}
	while(1){
	printf("pthread 22222\n");
*///	}
}


BOOL
bCreatePollingThread()
{
    BOOL            bResult;
	pthread_t       xThread;
    if( eGetPollingThreadState(  ) == STOPPED )
    {
        if( pthread_create( &xThread, NULL, pvPollingThread,NULL) != 0 )
        {
            /* Can't create the polling thread. */
            bResult = FALSE;
        }
        else
        {
            bResult = TRUE;
        }
    }
    else
    {
        bResult = FALSE;
    }

    return bResult;
}

void* pvPollingThread( void *pvParameter)
{
    eSetPollingThreadState( RUNNING );

    if( eMBEnable(  ) == MB_ENOERR )
    {
        do
        {
            if( eMBPoll(  ) != MB_ENOERR )
                break;
        }
        while( eGetPollingThreadState(  ) != SHUTDOWN );
    }

    ( void )eMBDisable(  );

    eSetPollingThreadState( STOPPED );

    return 0;
}

enum ThreadState
eGetPollingThreadState(  )
{
    enum ThreadState eCurState;

    ( void )pthread_mutex_lock( &xLock );
    eCurState = ePollThreadState;
    ( void )pthread_mutex_unlock( &xLock );

    return eCurState;
}

void
eSetPollingThreadState( enum ThreadState eNewState )
{
    ( void )pthread_mutex_lock( &xLock );
    ePollThreadState = eNewState;
    ( void )pthread_mutex_unlock( &xLock );
}

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_HOLDING_START ) &&
        ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
          /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
                *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( UCHAR ) ( usRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;
	
             /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/* ---------------对eMBRegCoilsCB函数进行补全--------------- */
eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
eMBErrorCode     eStatus = MB_ENOERR;
     int         iCoils;
     int          usBitOffset;
 
     if(( (int)usAddress >= REG_COILS_START ) && ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )  
     {
         usBitOffset = ( int)( usAddress - REG_COILS_START );
         switch(eMode)
         {
              case MB_REG_READ:
              while( iCoils > 0)
              {
                 *pucRegBuffer++ = xMBUtilGetBits( ucRegCoilsBuf,usBitOffset,(int)( iCoils > 8 ? 8 : iCoils ));
                 iCoils -= 8;
                 usBitOffset += 8;
              }
              break;
 
              case MB_REG_WRITE:
              while( iCoils > 0 )
              {
                 xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,(int)(iCoils > 8 ? 8 : iCoils),*pucRegBuffer++);
                 iCoils -= 8;
              }
              break;
         }
     }
     else
     {
         eStatus = MB_ENOREG;
     }
     return eStatus;
}

/* ---------------对eMBRegDiscreateCB函数进行补全-------------- */
eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMBErrorCode      eStatus = MB_ENOERR;
     int iNDiscrete = (int)usNDiscrete;
     int usBitOffset;
 
     if((int)(usAddress >= REG_DISCRETE_START) && (usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE))
     {
         usBitOffset = (int)(usAddress - REG_DISCRETE_START);
 
         while(iNDiscrete > 0)
         {
              *pucRegBuffer++ = xMBUtilGetBits(ucRegDiscreteBuf,usBitOffset,(int)(iNDiscrete > 8 ? 8 : iNDiscrete));
              iNDiscrete -= 8;
              usBitOffset += 8;
         }
     }
     else
     {
         eStatus = MB_ENOREG;
     }
     return eStatus;

}
