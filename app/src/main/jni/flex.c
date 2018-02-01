#include <utils/android_log_print.h>
#include "flex.h"
#include "os_support.h"

#ifdef ANDROID_JNI
#define #define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG, __VA_ARGS__))

#endif
/**
 * @name GPIO申请函数
 * @param pin 引脚号
 * @return 0 申请成功 -1 申请失败
 */
static int gpio_export(int pin)
{
	char buffer[64];
	int len;
	int fd;
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		LOGD("Failed to open export gpio %d for writing!\n",pin);
		return (-1);
	}
	len = snprintf(buffer, sizeof(buffer), "%d", pin);
	if (write(fd, buffer, len) < 0) {
		LOGD("Failed to export gpio %d !",pin);
		return -1;
	}
	close(fd);
	return 0;
}
/**
 * @name 设置GPIO输入输出功能
 * @param pin gpio引脚标号
 * @param dir  设置功能  dir: 0-->IN, 1-->OUT
 * @return 0：成功  -1：失败
 */
static int gpio_direction(int pin, int dir)
{
	static const char dir_str[] = "in\0out";
	char path[64];
	int fd;
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (fd < 0) {
		LOGD("Failed to open gpio %d direction for writing!\n",pin);
		return -1;
	}
	if (write(fd, &dir_str[dir == 0 ? 0 : 3], dir == 0 ? 2 : 3) < 0) {
		LOGD("Failed to set  gpio %d  direction!  \n",pin);
		return -1;
	}
	close(fd);
	return 0;
}
/**
 * @name  设置gpio高低电平
 * @param pin 引脚编号
 * @param value 电平值 value: 0-->LOW, 1-->HIGH
 * @return 0 成功  -1 失败
 */
static int gpio_write(int pin, int value)
{
	static const char values_str[] = "01";
	char path[64];
	int fd;
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (fd < 0) {
		LOGD("Failed to open gpio %d value for writing!\n",pin);
		return -1;
	}
	if (write(fd, &values_str[value == 0 ? 0 : 1], 1) < 0) {
		LOGD("Failed to write gpio %d value!\n",pin);
		return -1;
	}
	close(fd);
	return 0;
}
/**
 * @name gpio电平值读取
 * @param pin 引脚标号
 * @return   0 成功 -1 失败
 */
static int gpio_read(int pin)
{
    char path[64];
    char value_str[3];
    int fd;
    snprintf(path, sizeof(path),"sys/class/gpio/gpio%d/value",pin);
    fd = open(path,O_RDONLY);
    if(fd < 0)
    {
        LOGD("open the gpio %d file error\n",pin);
        return -1;

    }
    if(read(fd,value_str,3)<0)
    {
        LOGD("Read the gpio %d value error\n",pin);
        return  -1;
    }
    close(fd);
    return (atoi(value_str));

}
/* none表示引脚为输入，不是中断引脚

 rising表示引脚为中断输入，上升沿触发

 falling表示引脚为中断输入，下降沿触发

 both表示引脚为中断输入，边沿触发
*/
/**
 * @name gpio中断函数
 * @param pin 引脚标号
 * @param edge 触发方式 0-->none, 1-->rising, 2-->falling, 3-->both
 * @return 0 成功  -1 失败
 */

static int gpio_edge(int pin, int edge)

{
	const char dir_str[] = "none\0rising\0falling\0both";
	char ptr;
	char path[64];
	int fd;
	switch (edge) {
	case 0:
		ptr = 0;
		break;
	case 1:
		ptr = 5;
		break;
	case 2:
		ptr = 12;
		break;
	case 3:
		ptr = 20;
		break;
	default:
		ptr = 0;
	}
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/edge", pin);
	fd = open(path, O_WRONLY);
	if (fd < 0) {
		LOGD("Failed to open gpio %d  edge for writing!\n",pin);
		return -1;
	}
	if (write(fd, &dir_str[ptr], strlen(&dir_str[ptr])) < 0) {
		LOGD("Failed to set gpio %d edge!\n",pin);
		return -1;
	}
	close(fd);
	return 0;
}
/**
 * @name            @example
 * pthread_t         线程id
 * pthread_mutex_t   线程锁
 * a                 循环标志
 * gpio_fd_0         gpio89
 * gpio_fd_1         gpio89
 * ret               判断异常标志
 * pollfd
 * buff[]
 */
static pthread_t tid;
static pthread_mutex_t dev_lock;
static int a = 0;
int gpio_fd_0,gpio_fd_1,ret;
struct pollfd fds[3];
char buff[10];
/**
 * @name  监测光藕限位
 * @param arg
 * @return  1 down end ; 0 up end;
 */
void *flex_pipeLine_process (void *arg)
{
    while(a)
    {
        ret = poll(fds, 1, 0);
        //gpio89 = 1 ?  down to the end
        if (fds[0].revents & POLLPRI)
        {
            LOGD("******************** down end stop*************");
            ret = lseek(gpio_fd_0, 0, SEEK_SET);
            ret = read(gpio_fd_0, buff, 10);
            gpio_write(21, 0);
            gpio_write(22, 0);
            memset(buff,0,10);
            return 1;
        }

        //gpio41 = 1 ?  up to the end
        ret = poll(fds, 2, 0);
        if (fds[1].revents & POLLPRI)
        {
            LOGD("********************up  end  stop*************");
            ret = lseek(gpio_fd_1, 0, SEEK_SET);
            ret = read(gpio_fd_1, buff, 10);
            gpio_write(22, 0);
            gpio_write(21, 0);
            memset(buff,0,10);
            return 0;
        }

    }
}
/**
 * @name 电机初始化函数
 * @return 0 初始化完成
 */
int body_flexible_init()
{
    gpio_export(75);
    gpio_direction(75, 1);
    gpio_write(75, 1);//

    gpio_export(28);
    gpio_direction(28, 1);
    gpio_write(28, 1);//

    gpio_export(88);
    gpio_direction(88, 1);
    gpio_write(88, 1);//

    //motor1A
    gpio_export(21);
    gpio_direction(21, 1);
    gpio_write(21, 0);//
    //motor1B
    gpio_export(22);
    gpio_direction(22, 1);
    gpio_write(22, 0);

    //xwkRX1
    gpio_export(41);
    gpio_direction(41, 0);
    gpio_edge(41, 1);
    gpio_fd_1 = open("/sys/class/gpio/gpio41/value", O_RDONLY);
    //xwkRX2
    gpio_export(89);
    gpio_direction(89, 0);
    gpio_edge(89, 1);
    gpio_fd_0 = open("/sys/class/gpio/gpio89/value", O_RDONLY);

    fds[0].fd = gpio_fd_0;
    fds[0].events = POLLPRI;
    fds[1].fd = gpio_fd_1;
    fds[1].events = POLLPRI;
    LOGD("-----------------------FLEX--------------------------------");
    return 0;

}
/**
 * @name 直流电机 控制升降函数
 * @param flexible 升降方向  0->升 1->降
 * @return 0成功 -1 失败
 */
int body_flexible(int flexible)
{

    PTHREAD_MUTEX_INIT(&dev_lock, NULL);
    //gpio89/value
    ret = lseek(gpio_fd_0, 0, SEEK_SET);
    ret = read(gpio_fd_0, buff, 10);
    //down  
  if(flexible == 1 && !memcmp(buff,"0",strlen("1")))
	{
		LOGD("------------------------------------------\n");
		gpio_write(22, 0);
		gpio_write(21, 1);
		memset(buff,0,10);
		a = 1;
	}
	//gpio41 
	ret = lseek(gpio_fd_1, 0, SEEK_SET);
	ret = read(gpio_fd_1, buff, 10);
	//up 
	if(flexible == 0 && !memcmp(buff,"0",strlen("1")))
	{
		LOGD("r+++++++++++++++++++++++++++++++++++-\n");
		gpio_write(22, 1);
		gpio_write(21, 0);
		memset(buff,0,10);
		a = 1;
	}
    if(pthread_create(&tid, NULL, flex_pipeLine_process, (void *)NULL) != 0)
    {
        LOGD("body com do not create task:\n");

        return -1;
    }
	return 2;
}
/**
 * @name 升降位置 状态查询
 * @return 0 上升到限位  1 下降到限位  -1 中间位置
 */
int flexible_state()
{
    int i = 0;
    int down_value,up_value,up,down;
    char up_buff[10];
    char down_buff[10];

    up_value = open("/sys/class/gpio/gpio41/value", O_RDONLY);
    //xwkRX2
    down_value = open("/sys/class/gpio/gpio89/value", O_RDONLY);
    up = lseek(up_value, 0, SEEK_SET);
    up = read(up_value, up_buff, 10);
    down = lseek(down_value, 0, SEEK_SET);
    down = read(down_value, down_buff, 10);
    LOGD("up_buff = %d\n",up_buff);
    if(memcmp(up_buff,"1",strlen("1"))==0 && memcmp(down_buff,"0",strlen("1"))==0)
    {
        LOGD(" Rise to the top");
        return 0;
    }
    else if (memcmp(down_buff,"1",strlen("1"))==0 && memcmp(up_buff,"0",strlen("1"))==0)
    {
        LOGD("Down to the bottom ");
        return 1;
    } else
    {
        LOGD("Don't  know current location ");
        return  -1;
    }


}

