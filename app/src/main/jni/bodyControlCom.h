#ifndef _body_control_com_h_
#define _body_control_com_h_

#include <pthread.h>
#include "list.h"

#define BUFSIZE 256 

#define US_PARITY_EVEN    'e'       	/* Even Parity */
#define US_PARITY_ODD     'o'      	 /* Odd Parity */
#define US_PARITY_SPACE   's'       	/* Space Parity to 0 */
#define US_PARITY_MARK    'm'       	/* Marked Parity to 1 */
#define US_PARITY_NO       'n'      	/* No Parity */

#define US_DATA_BIT_5		5		/* 5 bits */
#define US_DATA_BIT_6		6		/* 6 bits */
#define US_DATA_BIT_7		7		/* 7 bits */
#define US_DATA_BIT_8		8		/* 8 bits */

#define US_STOP_BIT_1     1       	/* 1 Stop Bit */
#define US_STOP_BIT_1_5   1       	/* 1.5 Stop Bits */
#define US_STOP_BIT_2     2       	/* 2 Stop Bits */

typedef int (*datacallback) (unsigned int command, unsigned char *data, int size);

struct bodyCom_msg 
{	
	int ret;
	unsigned int sequence;
	unsigned int port;
	int flags;
	int len;
	int status;
	unsigned int command;
	pthread_mutex_t lock;
	pthread_cond_t wait;
	long long entry_times;
	struct list_head cmd_node;
	int is_sync;
	unsigned char data[1];
};

int mindpush_bodyCom_command_send (int command, unsigned char *buf, int len);
int minpush_bodyCom_init(int port, int baud, datacallback function);
int mindpush_bodyCom_exit(void);
#endif
