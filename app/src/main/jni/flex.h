

#ifndef _flex_h_
#define _flex_h_

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/select.h>
static int gpio_export(int pin);
static int gpio_direction(int pin, int dir);
static int gpio_write(int pin, int value);
static int gpio_read(int pin);
static int gpio_edge(int pin, int edge);

int body_flexible_init();
int body_flexible(int flexible);
int flexible_state();


#endif