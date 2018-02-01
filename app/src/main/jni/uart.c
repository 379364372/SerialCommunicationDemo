/*
 * fio - the flexible io tester
 *
 * Copyright (C) 2005 Jens Axboe <axboe@suse.de>
 * Copyright (C) 2006-2012 Jens Axboe <axboe@kernel.dk>
 *
 * The license below covers all files distributed with fio unless otherwise
 * noted in the file itself.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <stdio.h>  
#include <stdlib.h> 
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include "uart.h"

#define ANDROID_JNI
#ifdef ANDROID_JNI 
#include "./utils/android_log_print.h"
//#define SERIAL_DEVICE_NAME "/dev/ttyAMA"
#define SERIAL_DEVICE_NAME "/dev/ttyNSC"
#else
	#define LOGD(fmt, args...) do {\
		printf (fmt, ##args);\
	} while(0)

#define SERIAL_DEVICE_NAME "/dev/ttyUSB"
#endif

#define MAX_SERIAL_DEVICE  8 //move to configure file
#define TIMEOUT_SEC(buflen, baudrate)    ((buflen)*20/(baudrate) + 2)
#define TIMEOUT_USEC    0

static int serial_fd[MAX_SERIAL_DEVICE];
static struct termios oldOpt[MAX_SERIAL_DEVICE];

static int speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300 };
static int name_arr[] = { 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300 };

static void SetSpeed (int fd, int speed)
{
    unsigned int i;
    struct termios Opt;

    if (tcgetattr (fd, &Opt) != 0)
    {
        perror ("tcgetattr:");
	return;
    }

    for (i = 0; i < sizeof (speed_arr) / sizeof (int); i++)
    {
        if (speed == name_arr[i])
	{
            tcflush (fd, TCIOFLUSH);
	    cfsetispeed (&Opt, speed_arr[i]);
	    cfsetospeed (&Opt, speed_arr[i]);
 	    if (tcsetattr (fd, TCSANOW, &Opt) != 0)
	    {
	        perror ("tcsetattr:");
		return;
	    }
	    tcflush (fd, TCIOFLUSH);
	}
   }
}

static int SetParity (int fd, int databits, int stopbits, int parity)
{
    struct termios options;

    if (tcgetattr (fd, &options) != 0)
    {
        perror ("SetupSerial 1");
	return (0);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits)	/*\u8bbe\u7f6e\u6570\u636e\u4f4d\u6570 */
    {
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
	options.c_cflag |= CS8;
	break;
    default:
	fprintf (stderr, "Unsupported data size\n");
	return (0);
    }
    switch (parity)
    {
    case 'n':
    case 'N':
	options.c_cflag &= ~PARENB;	/* Clear parity enable */
	options.c_iflag &= ~INPCK;	/* Enable parity checking */
	break;
    case 'o':
	case 'O':
	options.c_cflag |= (PARODD | PARENB);	/* \u8bbe\u7f6e\u4e3a\u5947\u6548\u9a8c */
	options.c_iflag |= INPCK;	/* Disnable parity checking */
	break;
    case 'e':
    case 'E':
	options.c_cflag |= PARENB;	/* Enable parity */
	options.c_cflag &= ~PARODD;	/* \u8f6c\u6362\u4e3a\u5076\u6548\u9a8c */
	options.c_iflag |= INPCK;	/* Disnable parity checking */
	break;
    case 'S':
    case 's':	/*as no parity */
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	break;
    default:
	fprintf (stderr, "Unsupported parity\n");
 	return (0);
    }

    switch (stopbits)
    {
    case 1:
	options.c_cflag &= ~CSTOPB;
	break;
    case 2:
	options.c_cflag |= CSTOPB;
	break;
    default:
	fprintf (stderr, "Unsupported stop bits\n");
	return (0);
    }
    /* Set input parity option */
    if (parity != 'n')
	options.c_iflag |= INPCK;
    tcflush (fd, TCIFLUSH);
    options.c_cc[VTIME] = 0;	//150;  /* \u8bbe\u7f6e\u8d85\u65f615 seconds */
    options.c_cc[VMIN] = 1;	/* Update the options and do it NOW */

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	/*Input */
    options.c_oflag &= ~OPOST;	/*Output */

    /*motor*********************** */
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    options.c_oflag &= ~(ONLCR | OCRNL);
    if (tcsetattr (fd, TCSANOW, &options) != 0)
    {
        perror ("SetupSerial 3");
	return (0);
    }
    return (1);
}

static int uart_open (int nPort, int nBaud, int databits,  int stopbits, char parity)
{
    int fd;
    char szPort[32] = { 0 };
	
    snprintf(szPort,32,"%s%d", SERIAL_DEVICE_NAME, nPort);
    LOGD("open uart[%s]  \n", szPort);
    fd = open (szPort, O_RDWR | O_NOCTTY);
    if (-1 == fd)
    {
	LOGD ("[CSerial] open failed: %s\n", szPort);
	return -1;
    }

    tcgetattr (fd, &oldOpt[nPort]);

    SetSpeed (fd, nBaud);
    SetParity (fd, databits, stopbits, parity);

    LOGD ("configure complete\n");


    return fd;
}

static int uart_close (int port)
{
    int fd;
    fd = serial_fd[port]; 
    tcsetattr (fd, TCSANOW, &oldOpt[port]);
    close (fd);

    return 0;
}

int uart_init (int port, int baudrate, int databits, int stopbits, char parity)
{

	if((port < 0) || (port >= MAX_SERIAL_DEVICE))
	{
		fprintf (stderr, "input param open port %d err\n", port);
        	return -1;
	}

	serial_fd[port] = uart_open (port, baudrate, databits, stopbits, parity);
	if (serial_fd[port] < 0)
	{
		LOGD("uart_open err\n");
		return -1;
	}

	return serial_fd[port];	
}

void uart_exit (int port)
{
	if(port >= 0 && port < MAX_SERIAL_DEVICE)
		uart_close (port);
}

int mindpush_serialRead(int fd, int baud, unsigned char *data, int size, int option)	
{
	int    retval = 0;
	struct timeval tv_timeout;
	fd_set fs_read;

	FD_ZERO (&fs_read);
	FD_SET (fd, &fs_read);
	
	baud = baud;
	switch(option)
	{
		case NO_ACK:
			retval = select (fd + 1, &fs_read, NULL, NULL, NULL);	
			break;
		case ACK_1:
			tv_timeout.tv_sec = 0;
			tv_timeout.tv_usec = 10 * 1000;
			//tv_timeout.tv_sec = 0.1;
			//tv_timeout.tv_usec = 0;
			retval = select (fd + 1, &fs_read, NULL, NULL, &tv_timeout);	
			break;
		case ACK_3:
			tv_timeout.tv_sec = 0.3;
			tv_timeout.tv_usec = 0;
			retval = select (fd + 1, &fs_read, NULL, NULL, &tv_timeout);	
			break;
		default:
			retval = select (fd + 1, &fs_read, NULL, NULL, NULL);	
			break;
	}

	if (retval)
	{
	    return(read (fd, data, size));
	}
	else if(retval == 0)
	{
		//LOGD("ReadComPort  timeout \n");
		return -1;
	}
	else
	{
		LOGD("selct() call failed : %s \n",strerror(errno));
	    	return -1;
	}
}

int mindpush_serialWrite (int fd, int baud, unsigned char * data, int size)
{
	unsigned char *write_data = data;
	int total_len = 0, retval = 0, len = 0;	
	fd_set fsWrite; 
	struct timeval tvTimeOut;
	int id;

	//memset(&tvTimeOut, 0x00, sizeof(tvTimeOut));
	FD_ZERO(&fsWrite);
	FD_SET(fd, &fsWrite);

	tvTimeOut.tv_sec  = TIMEOUT_SEC(size, baud);
	tvTimeOut.tv_usec = TIMEOUT_USEC;

	for (total_len = 0; total_len < size;) 
	{
		retval = select(fd + 1, NULL, &fsWrite, NULL, &tvTimeOut);
		if (retval < 0) 
		{
			return -1;
		}
		else if (retval && FD_ISSET(fd, &fsWrite)) 
		{
			len = write(fd, &write_data[total_len], size - total_len);
			LOGD("uart write len %d\n", len);
			if (len > 0) 
			{
				total_len += len;
			}
			else if (len < 0) 
			{
				if (total_len == 0) 
				{
					tcflush(fd, TCOFLUSH);

					return -1;	
				}
				return total_len;
			}
		}
		else 
		{
			if (total_len == 0) 
			{
				tcflush(fd, TCOFLUSH);
																						
				return -1;
			}
			break;
		}
	}
	return total_len;
}	

#if 0
int main(int argc, char *argv[], char *envp[])
{

    int serial_handle, i;
    int sendCnt = 10, rcvCnt = 100;
    unsigned char sendBuf[32], rcvBuf[32];    
    int ret;

    LOGD("hello uart demo\n");
	
    serial_handle = uart_init(2, 9600);
    memset(sendBuf, 0, 32);
    memset(rcvBuf, 0, 32);
 
    while(sendCnt--)
	{
		LOGD("%d send data\n", sendCnt);
		sleep(1);
		for(i=0; i < 12; i++)
		{
			sendBuf[i] = i;
		}
		//ret = mindpush_serialWrite(serial_handle, 9600, sendBuf, 12);
		ret = mindpush_serialWrite(serial_handle, 9600, "haojianhua", 12);
		if(ret == -1)
		{
			LOGD("nucp_serialWrite \n");
		}
			
	}
   while(rcvCnt)
	{
		int i;
		int len;

		len = mindpush_serialRead(serial_handle, 9600, rcvBuf, 32, NO_ACK);
		LOGD("rcv data: len = %d\n", len);
		for (i = 0; i < len; i++)
			LOGD(" %#x ", rcvBuf[i]);
		LOGD ("\n");
		memset(rcvBuf, 0, 32);
	}

    return 0;
}
#endif
