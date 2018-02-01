#ifndef _UART_H_
#define _UART_H_

#define NO_ACK				0
#define ACK_1				1
#define ACK_3				2

int mindpush_serialWrite (int fd, int baud, unsigned char * data, int size);
int mindpush_serialRead(int fd, int baud, unsigned char *data, int size, int option);
int uart_init (int port, int baudrate, int databits, int stopbits, char parity);
void uart_exit (int port);

#endif
