#ifndef __OS_SUPPORT_H__
#define __OS_SUPPORT_H__

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

#define D_PRINT(fmt, args...) do {\
                                  printf("file:%s, line:%d\n", __FILE__, __LINE__);\
                                  printf(fmt, ##args);\
                              } while(0)

static inline int PTHREAD_MUTEX_INIT(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	int ret;

	if ((ret = pthread_mutex_init(mutex, attr)))
		D_PRINT("\n");

	return ret;
}

static inline int PTHREAD_MUTEX_DESTROY(pthread_mutex_t *mutex)
{
	int ret;

	if ((ret = pthread_mutex_destroy(mutex)))
		D_PRINT("\n");

	return ret;
}

static inline int PTHREAD_MUTEX_LOCK(pthread_mutex_t *mutex)
{
	int ret;

	if ((ret = pthread_mutex_lock(mutex)))
		D_PRINT("\n");

	return ret;
}

static inline int PTHREAD_MUTEX_UNLOCK(pthread_mutex_t *mutex)
{
	int ret;

	if ((ret = pthread_mutex_unlock(mutex)))
		D_PRINT("\n");

	return ret;
}

static inline int PTHREAD_MUTEX_TRYLOCK(pthread_mutex_t *mutex)
{
	int ret;

	if ((ret = pthread_mutex_trylock(mutex)))
		D_PRINT("\n");

	return ret;
}

static inline int PTHREAD_COND_INIT(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
	int ret;

	if ((ret = pthread_cond_init(cond, attr)))
		D_PRINT("\n");

	return ret;
}

static inline int PTHREAD_COND_DESTROY(pthread_cond_t *cond)
{
	int ret;

	if ((ret = pthread_cond_destroy(cond)))
		D_PRINT("\n");

	return ret;
}

static inline int PTHREAD_COND_WAIT(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	int ret;

	if ((ret = pthread_cond_wait(cond, mutex)))
		D_PRINT("\n");

	return ret;
}

static inline int PTHREAD_COND_TIMEDWAIT(pthread_cond_t *cond, pthread_mutex_t *mutex,
																unsigned long uTimeOutMsec)
{
	struct timeval now; 
	struct timespec timeout;
   	unsigned timeout_us;
	int ret;

	/* Calculate uTimeOutMsec in terms of the absolute time. uTimeOutMsec is in milliseconds*/
	 gettimeofday(&now, NULL); 
	timeout_us = now.tv_usec + 1000 * uTimeOutMsec;
	timeout.tv_sec = now.tv_sec + timeout_us / 1000000;
	timeout.tv_nsec = (timeout_us % 1000000) * 1000;
	
	if ((ret = pthread_cond_timedwait(cond, mutex, &timeout)) != 0)
		D_PRINT(" pthread_cond_timedwait timeout\n");

	return ret;
}

static inline int PTHREAD_COND_SIGNAL(pthread_cond_t *cond)
{
	int ret;

	if ((ret = pthread_cond_signal(cond)))
		D_PRINT("\n");

	return ret;
}

static inline int PTHREAD_COND_BROADCAST(pthread_cond_t *cond)
{
	int ret;

	if ((ret = pthread_cond_broadcast(cond)))
		D_PRINT("\n");

	return ret;
}

static inline long long get_sysclock_millis(void)
{
	long long ts;
	struct timeval tv;

	gettimeofday (&tv, NULL);
	ts = tv.tv_sec;
	ts = ts * 1000;
	ts += tv.tv_usec / 1000;

	return ts;
}

static inline long long get_sysclock_micros(void)
{
	long long ts;
	struct timeval tv;

	gettimeofday (&tv, NULL);
	ts = tv.tv_sec;
	ts = ts * 1000000;
	ts += tv.tv_usec;

	return ts;
}

#endif//__OS_SUPPORT_H__
