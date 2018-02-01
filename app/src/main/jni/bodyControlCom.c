#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

#include "os_support.h" 
#include "bodyControlCom.h"
//#include "list.h"
#include "uart.h"
#include "modbus.h"
#include <sys/time.h>


#define ANDROID_JNI
#ifdef ANDROID_JNI 
#include "./utils/android_log_print.h"
#else
	#define LOGD(fmt, args...) do {\
		printf (fmt, ##args);\
	} while(0)

#endif	

#define MAX_FRAME_SIZE BUFSIZE

typedef struct _frame_t
{
	unsigned int cmd;
	int frame_size;
	unsigned char buffer[MAX_FRAME_SIZE]; 
}frame_t;

//*--------------------------------------------------------------------------------------
//*�豸ģ�������ӳ���
//*--------------------------------------------------------------------------------------

static int gBodyComInit = 0;
static int serial_handle = 0;
static int serial_port = 2;
static int serial_baud = 9600;
static pthread_t tid;  //  		�߳�ID��
static int runing = 0;
static datacallback commit_java_func = NULL;
static pthread_mutex_t dev_lock;
//static struct list_head tx_head;
//static struct list_head ack_head;
LIST_HEAD(tx_head);
LIST_HEAD(ack_head);
static int pending = 0;
static unsigned char rxBuf[BUFSIZE] = {0};
static int rxLen = 0;
static long long resTimeout_timer = 0;
static int err_frame = 0;
static int err_timeout = 0;

static int serialBoard_send_pack (int handle, unsigned int baud, 
									unsigned char *pData, int size)
{
	int i = 0;

	if (1)
	{
		LOGD("send pack data: size = %d", size);


	}
	return mindpush_serialWrite (handle, baud, pData,  size);
}

static int bodyCom_sync (unsigned int command,
									unsigned char *buf, int size)
{
	int status = 0;
	
	struct bodyCom_msg *msg = (struct bodyCom_msg *) malloc (sizeof (*msg) + size);
    LOGD("xxxxxxxx bodyCom_sync xxxxxxx\n");
	msg->ret = 1;
	memcpy (msg->data, buf, size);
	msg->len = size;
	msg->status = 0;
	msg->command = command;
	msg->entry_times = get_sysclock_millis ();
	PTHREAD_MUTEX_INIT(&msg->lock, NULL);
	PTHREAD_COND_INIT (&msg->wait, NULL);
	
	msg->is_sync = 1;
	PTHREAD_MUTEX_LOCK (&dev_lock);
	list_add (&msg->cmd_node, &tx_head);
	PTHREAD_MUTEX_UNLOCK (&dev_lock);
	
	PTHREAD_MUTEX_LOCK (&msg->lock);
	while (msg->ret)
		PTHREAD_COND_WAIT (&msg->wait, &msg->lock);
	PTHREAD_MUTEX_UNLOCK (&msg->lock);
	
	status = msg->status;
	
	PTHREAD_COND_DESTROY (&msg->wait);
	PTHREAD_MUTEX_DESTROY (&msg->lock);
	free (msg);

    return status;	
}

static int bodyCom_async (unsigned int command, unsigned char *buf, int size)
{
	int status = 0;
	
	
	struct bodyCom_msg *msg = (struct bodyCom_msg *) malloc (sizeof (*msg) + size);
    LOGD("xxxxxxxx bodyCom_async xxxxxxx\n");
	msg->ret = 1;
	
	memcpy (msg->data, buf, size);
	msg->len = size;
	msg->status = 0;
	msg->command = command;
	msg->entry_times = get_sysclock_millis ();
	PTHREAD_MUTEX_INIT(&msg->lock, NULL);
	PTHREAD_COND_INIT (&msg->wait, NULL);
	
	msg->is_sync = 0;
	PTHREAD_MUTEX_LOCK (&dev_lock);
	list_add (&msg->cmd_node, &tx_head);
	PTHREAD_MUTEX_UNLOCK (&dev_lock);
    LOGD("xxxxxxxx bodyCom_async end xxxxxxx\n");
	return 0;
	
}

static int bodyCom_submit (frame_t *frame, unsigned mode)
{
	if (mode)
	{
		return bodyCom_sync (frame->cmd, frame->buffer, frame->frame_size);
	}
	return bodyCom_async (frame->cmd, frame->buffer, frame->frame_size);
}

static void bodyCom_xfer_msg (void)
{
   // LOGD("bodyCom_xfer_msg  enter succeful");
	struct list_head *list = NULL;

	if (pending)
		return;
	struct bodyCom_msg *msg = NULL;

	PTHREAD_MUTEX_LOCK (&dev_lock);
   // LOGD("bodyCom_xfer_msg if hjh (!list_empty(&tx_head) \n");
	if (!list_empty(&tx_head))
	{
        LOGD("bodyCom_xfer_msg !list_empty(&tx_head)  EINT\n");
		list = &tx_head;
		msg = list_first_entry(list, struct bodyCom_msg, cmd_node);
        LOGD("bodyCom_xfer_msg --------------------0\n");
		list_del(&msg->cmd_node);
        LOGD("-----------------bodyCom_xfer_msg --------------------1\n");
		list_add (&msg->cmd_node, &ack_head);
        LOGD("bodyCom_xfer_msg --------------------2\n");
		pending = 1;
        LOGD("pending = %d  \n",pending);
		//serial send
		serialBoard_send_pack(serial_handle, serial_baud, msg->data, msg->len);
        //LOGD("serial_handle is %d  serial_baud  is %d   data is %s  len is  %s  ",serial_handle,serial_baud,msg->data,msg->len);
        LOGD("serialBoard_send_pack successful \n ");
	}
	PTHREAD_MUTEX_UNLOCK (&dev_lock);
}

static int respond_proc (struct bodyCom_msg *msg, unsigned int command, unsigned char *respond_param, int len)
{
	int ret = 0;
	int isEmpty = 0;
	struct bodyCom_msg *curr = msg;
	int frame_len;

/* xray test
	if((CRC16(respond_param, frame_len)) != 0)
    {
		err_frame++;
		LOGD("[%s, %d] response frame crc err:\n", __FUNCTION__, __LINE__);

		return -1;
    }
*/    
	if (commit_java_func)
	{
		PTHREAD_MUTEX_LOCK (&dev_lock);
		ret = commit_java_func(command, respond_param, len);
		PTHREAD_MUTEX_UNLOCK (&dev_lock);
	}
	
	if (ret < 0)
	{
		err_frame++;
		LOGD("[%s, %d] response frame  content err:\n", __FUNCTION__, __LINE__);

		return -1;
	} 
	PTHREAD_MUTEX_LOCK (&dev_lock);
	list_del(&msg->cmd_node);
	PTHREAD_MUTEX_UNLOCK (&dev_lock);

	if (curr)
	{
		PTHREAD_MUTEX_LOCK (&dev_lock);
		err_timeout = 0;
		PTHREAD_MUTEX_UNLOCK (&dev_lock);
		{
			PTHREAD_MUTEX_LOCK (&dev_lock);
			pending = 0;
			msg->status = 0;
			PTHREAD_MUTEX_UNLOCK (&dev_lock);
			if (curr->is_sync)
			{
				PTHREAD_MUTEX_LOCK (&curr->lock);
				curr->ret = 0;
				PTHREAD_COND_SIGNAL(&curr->wait);
				PTHREAD_MUTEX_UNLOCK (&curr->lock);
			}
			else
			{
				PTHREAD_COND_DESTROY (&curr->wait);
				PTHREAD_MUTEX_DESTROY (&curr->lock);
				free (curr);
			}
		}
	}

	return 0;
}

static void respond_timeout (void)
{
	struct bodyCom_msg *msg = NULL;
	long long curr_times;
	struct list_head *list = NULL;

    if (list_empty(&ack_head))
    {
        return;
    }
    PTHREAD_MUTEX_LOCK (&dev_lock);
	list = &ack_head;
	msg = list_first_entry(list, struct bodyCom_msg, cmd_node);
	PTHREAD_MUTEX_UNLOCK (&dev_lock);
	curr_times = get_sysclock_millis ();
	LOGD("respond_timeout hjh xxxxxxxxxxxxxxxxxxxxxx msg[%d]\n", (int)msg);
	if ((msg) && ( (curr_times - msg->entry_times) > 500))
	{
		PTHREAD_MUTEX_LOCK (&dev_lock);
		int isLast = list_empty(&ack_head);
		LOGD("timeout sta msg[0x%x] is NULL = %d\n", (int)msg, isLast);
		list_del(&msg->cmd_node);
		pending = 0;
		msg->status = -1;
        err_timeout++;
        PTHREAD_MUTEX_UNLOCK (&dev_lock);
        if (msg->is_sync)
		{
			PTHREAD_MUTEX_LOCK (&msg->lock);
			msg->ret = 0;
			PTHREAD_COND_SIGNAL (&msg->wait);
			PTHREAD_MUTEX_UNLOCK (&msg->lock);
		}
		else
		{
			PTHREAD_COND_DESTROY (&msg->wait);
			PTHREAD_MUTEX_DESTROY (&msg->lock);
			free (msg);
		}
		
	}
}

static void respond_submit_process(unsigned char *buffer, int len)
{

#define CODE_COMMAND_POS 5
	//int ret;
	struct bodyCom_msg *msg = NULL;
	unsigned int command;
	struct list_head *list = NULL;
	unsigned int empty = 0;
	
	PTHREAD_MUTEX_LOCK (&dev_lock);
	empty = list_empty(&ack_head);
	PTHREAD_MUTEX_UNLOCK (&dev_lock);
    // read data is empty ?
	if (!empty)
	{
		PTHREAD_MUTEX_LOCK (&dev_lock);
		list = &ack_head;
		msg = list_first_entry(list, struct bodyCom_msg, cmd_node);
		PTHREAD_MUTEX_UNLOCK (&dev_lock);
		if (msg)
		{
			respond_proc(msg, buffer[CODE_COMMAND_POS], buffer, len);
		}

	}

}

//raed's data is complete ?
static void bodyCom_response_parse (int len)
{

#define LIMMIT_FULL 32
#define FRAME_LEN_POS 4
#define FRAME_LEN 10

	unsigned char buf[128] = { 0 };
	long long cur_millis = 0;
    int b = 0;

	if (len > 0)
	{
		int frame_len = 0;

		rxLen += len;
		resTimeout_timer = get_sysclock_millis();
		
		if (rxLen > LIMMIT_FULL)
		{
			LOGD("debug [%s, %d] rcv response will full frame len [%d]\n", __FUNCTION__, __LINE__, rxLen);
			memset(rxBuf, 0, BUFSIZE);
			rxLen = 0;
			resTimeout_timer = 0;

			return ;
		}

	redo:
	/*	if (rxBuf[0] != 0x11)
		{
			LOGD("debug [%s, %d] rcv response  head err: len [%d]\n",
											__FUNCTION__, __LINE__, rxLen);
			memset(rxBuf, 0, BUFSIZE);
			rxLen = 0;

			return ;
		}*/

		if (rxLen < 4)
		{
            LOGD("--------RXlen < 4--------");
			return ;

		}


		if (rxLen < rxBuf[FRAME_LEN_POS] +7)
		{
            LOGD("--------rexlen < 8--------");
			return;
		}

		frame_len = 18;
		memset(buf, 0, 64);
		memcpy(buf, &rxBuf[0], frame_len);
        for(b=0;b<frame_len;b++)
        LOGD("the rx_buf data is : %x \n",rxBuf[b]);
		respond_submit_process(buf, frame_len);
		/*rxLen -= frame_len;
		if (rxLen > 0)
		{
			memmove(rxBuf, &rxBuf[frame_len], rxLen);
			goto redo;
		}
*/
	}
	else
	{
        //read end
		cur_millis = get_sysclock_millis();
		if (cur_millis - resTimeout_timer > 100)
		{
			resTimeout_timer = cur_millis;
			memset(rxBuf, 0, BUFSIZE);
			rxLen = 0;
		}
    }

}

void *msg_pipeLine_process (void *arg)
{
	long long curr_times =0;
	int  len = 0;
	int bufSize = BUFSIZE;
	long long last_time = 0;
	
	while (runing)
	{
      		len = mindpush_serialRead(serial_handle, serial_baud, &rxBuf[rxLen], bufSize - rxLen, ACK_1);

#define RCV_DEBUG
#ifdef RCV_DEBUG
		if (len > 0) {
            int i;
            LOGD("uart read [%d] :\n", len);

            for (i = 0; i < len; i++)
                LOGD("0x%x ", rxBuf[rxLen + i]);
            LOGD("\n\n");
        }
#endif
            bodyCom_response_parse(len);
            curr_times = get_sysclock_millis();
            if ((curr_times - last_time) > 100) {
                last_time = curr_times;
                respond_timeout();
            }
		bodyCom_xfer_msg();

	}
	runing = -1;

	return NULL;
}

int mindpush_bodyCom_command_send (int command, unsigned char *buf, int len)
{
	struct subBoard_dev *board = NULL;
	frame_t frame;
	unsigned short crc;
	
	if (gBodyComInit)
	{
		crc = CRC16(buf, len - 2);
		buf [len-2] = crc >> 8;
        buf [len-1]	= crc;

		frame.frame_size = len;
		frame.cmd = command;
		memcpy(frame.buffer, buf, len);
		bodyCom_submit(&frame, 0);
        LOGD ("xxxxxxxxx  bodyCom_submit  0xxxxxxxxxxxxxxxx\n");

    }
	LOGD ("xxxxxxxxx command_send board end xxxxxxxxxxxxxxxx\n");

	return 0;
}

int minpush_bodyCom_init(int port, int baud, datacallback function)
{
	if (!gBodyComInit)
	{

		PTHREAD_MUTEX_INIT(&dev_lock, NULL);
		serial_handle = uart_init(port, baud, US_DATA_BIT_8, US_STOP_BIT_1, US_PARITY_EVEN);
		if (serial_handle < 0)
		{
			LOGD("init uart module false\n");
			
			return -1;
		}
		serial_port = port;
		serial_baud = baud;
		runing = 1;
		if(pthread_create(&tid, NULL, msg_pipeLine_process, (void *)NULL) != 0)
		{	
			uart_exit(port);
			LOGD("body com do not create task:\n");
			
			return -1;
		}

		//PTHREAD_MUTEX_INIT(&board->dev_lock, NULL);
		commit_java_func = function;
		gBodyComInit = 1;
	}
 	
	return 0;
}

int mindpush_bodyCom_exit(void)
{
	if (gBodyComInit)
	{
		runing = 0;
		pthread_join(tid, NULL);
		uart_exit(serial_port);
		PTHREAD_MUTEX_DESTROY(&dev_lock);
	}
	
	return 0;
}










