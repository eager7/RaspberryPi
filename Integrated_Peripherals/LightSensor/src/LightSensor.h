/*************************************************************************
	> File Name: LightSensor.h
	> Author: PCT
	> Mail: 
	> Created Time: 2016年3月6日 14时55分14秒
 ************************************************************************/
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define SENSOR_MAJOR 0	//major version
#define SENSOR_MINOR 0

#define SENSOR_NUM 1
#define SENSOR_NAME "light_sensor"
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum _tCmd
{
	E_NUM_INIT,
	E_NUM_SLEEP,
	E_NUM_EXIT,
}tCmd;
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
static int sensor_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos);
static int sensor_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos);
inline static unsigned sensor_poll(struct file *filp, poll_table *pwait);
static int sensor_open(struct inode *inode, struct file *filp);
long sensor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
int mem_release(struct inode *inode, struct file *filp);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
static struct file_operations sensor_optns =
{
	.owner 			= THIS_MODULE,	//the module's owner
	.llseek			= NULL,			//set the pointer of file location
	.read  			= sensor_read,	//read data
	.aio_read		= NULL,			//asynchronous read
	.write 			= sensor_write,	//write data
	.aio_write		= NULL,			//asynchronous write
	.readdir		= NULL,			//read dir, only used for filesystem
	.poll  			= sensor_poll,	//poll to judge the device whether it can non blocking read & write
	//.ioctl 			= NULL,			//executive the cmd, int the later version of linux, used fun unlocked_ioctl replace this fun
	.unlocked_ioctl = sensor_ioctl,	//if system doens't use BLK filesystem ,the use this fun indeeded iotcl
	.compat_ioctl 	= NULL,			//the 32bit program will use this fun replace the ioctl in the 64bit platform
	.mmap			= NULL,			//memory mapping
	.open  			= sensor_open,	//open device
	.flush			= NULL,			//flush device
	.release		= mem_release,			//close the device
	//.synch			= NULL,			//refresh the data
	.aio_fsync		= NULL,			//asynchronouse .synch
	.fasync			= NULL,			//notifacation the device's Flag changed
};

static int sensor_major = SENSOR_MAJOR;
static int sensor_minor = SENSOR_MINOR;
struct cdev cdev;
/****************************************************************************/
/***        Local    Functions                                            ***/
/****************************************************************************/

