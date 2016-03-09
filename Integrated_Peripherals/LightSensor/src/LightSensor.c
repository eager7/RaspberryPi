/*************************************************************************
	> File Name: LightSensor.c
	> Author: PCT
	> Mail: 
	> Created Time: 2016年3月6日 14时55分14秒
 ************************************************************************/
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "LightSensor.h"
#include "i2c_simulation.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
/****************************************************************************/
/***        Local    Functions                                            ***/
/****************************************************************************/
static int __init sensor_init(void)
{
	dev_t dev_no = MKDEV(sensor_major, sensor_minor);
	int result = 0;
    printk(KERN_DEBUG "Hello %s Kernel Init!\n", SENSOR_NAME);
	
	/*region device version*/
	if(sensor_major){
		printk(KERN_DEBUG "static region device\n");
		result = register_chrdev_region(dev_no, SENSOR_NUM, SENSOR_NAME);//static region
	}else{
		printk(KERN_DEBUG "alloc region device\n");
		result = alloc_chrdev_region(&dev_no, sensor_minor, SENSOR_NUM, SENSOR_NAME);//alloc region
		sensor_major = MAJOR(dev_no);//get major
	}
	if(result < 0){
		printk(KERN_ERR "Region Device Error, %d\n", result);
		return result;
	}
	
	cdev_init(&cdev, &sensor_optns);//init cdev
	cdev.owner = THIS_MODULE;
	cdev.ops = &sensor_optns;
	
	printk(KERN_DEBUG "cdev_add\n");
	cdev_add(&cdev, MKDEV(sensor_major, sensor_minor), SENSOR_NUM);//regedit the device
	
	i2c_init();//I2C
	sda_off();
	scl_off();
    return 0;
}

static void sensor_exit(void)
{
    printk(KERN_ALERT "unregion the sensor device\n");
	cdev_del(&cdev);//delete the device
	unregister_chrdev_region(MKDEV(sensor_major, sensor_minor), SENSOR_NUM);//unregion device
}

static int sensor_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	printk(KERN_DEBUG "sensor_write\n");
	
	return 0;
}

static int sensor_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	printk(KERN_DEBUG "sensor_read\n");
	return 0;
}

static int sensor_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev = NULL;
	printk(KERN_DEBUG "sensor_open\n");
	/*
	dev = container_of(inode->i_cdev, struct scull_dev, dev);
	filp->private_data = dev;
	
	if(O_WRONLY == (filp->f_flags & O_ACCMODE)){//trim to 0 the length of device if open was write-only
		printk(KERN_ALERT "scull_trim 0\n");
		scull_trim(dev);//ignore error
	}*/
	return 0;
}

inline static unsigned sensor_poll(struct file *filp, poll_table *pwait)
{
	printk(KERN_DEBUG "gpio_poll\n");
	
	return 0;
}

long sensor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	printk(KERN_DEBUG "sensor_ioctl\n");
	switch(cmd){
		case E_NUM_INIT:{
			printk(KERN_DEBUG "E_NUM_INIT 1\n");
			sda_off();
		}
		case E_NUM_SLEEP:{
			printk(KERN_DEBUG "E_NUM_SLEEP 2\n");
		}
		case E_NUM_EXIT:{
			printk(KERN_DEBUG "E_NUM_EXIT 3\n");
			scl_off();
		}

	}
	return 0;
}

int mem_release(struct inode *inode, struct file *filp)
{
	printk(KERN_DEBUG "mem_release\n");
	return 0;
}

/****************************************************************************/
/***        Kernel    Module                                              ***/
/****************************************************************************/
module_init(sensor_init);
module_exit(sensor_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Pan Chang Tao");
MODULE_DESCRIPTION("The LightSensor Driver");
MODULE_ALIAS("A Sensor Driver Module");
