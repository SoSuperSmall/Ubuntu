#ifndef _PTHREAD_MODBUS_H_
#define _PTHREAD_MODBUS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "mbfunc.h"
 
/* ----------------------- Defines ------------------------------------------*/
#define PROG            "freemodbus"


/* --------------修改和添加----------------------*/
#define REG_INPUT_START 1
#define REG_INPUT_NREGS 100
#define REG_HOLDING_START 1
#define REG_HOLDING_NREGS 100
#define REG_COILS_START 1
#define REG_COILS_SIZE 100
#define REG_DISCRETE_START 1
#define REG_DISCRETE_SIZE 100
/* -----------------------------------------------*/


static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];
/* -------------------进行添加-------------------- */
static USHORT	ucRegCoilsStart = REG_COILS_START;
static USHORT	ucRegCoilsBuf[REG_COILS_SIZE];
static USHORT	ucRegDiscreteStart = REG_DISCRETE_START;
static USHORT	ucRegDiscreteBuf[REG_DISCRETE_SIZE];
/* ------------------------------------------------ */
static pthread_mutex_t xLock = PTHREAD_MUTEX_INITIALIZER;
static enum ThreadState
{
	STOPPED,
        RUNNING,
        SHUTDOWN
} ePollThreadState;
  
/* ----------------------- Static functions ---------------------------------*/
static BOOL     bCreatePollingThread( void );
static enum ThreadState eGetPollingThreadState( void );
static void     eSetPollingThreadState( enum ThreadState eNewState );
static void* pvPollingThread( void *pvParameter );
 
static eMBException eMBFuncReadCoils( UCHAR * pucFrame, USHORT * usLen ); 
/* ----------------------- Start implementation -----------------------------*/
typedef struct stMbTcpInit{
          USHORT usport;
          USHORT usMode;
          UCHAR *pucAddr;
  }MB_TCP_INIT_S;



void *pthread_modbus(void *arg);


#endif
