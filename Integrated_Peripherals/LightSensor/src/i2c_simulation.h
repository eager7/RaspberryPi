/*************************************************************************
	> File Name: i2c_simulation
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
#include <asm/rt2880/rt_mmap.h>//MTK
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
//////////////////////////REGISTER ADDRESS////////////////////////////
#define RALINK_SYSCTL_ADDR		RALINK_SYSCTL_BASE	// system control
#define RALINK_REG_GPIOMODE		(RALINK_SYSCTL_ADDR + 0x60)
#define RALINK_REG_GPIOMODE2		(RALINK_SYSCTL_ADDR + 0x64)
#define RALINK_IRQ_ADDR			RALINK_INTCL_BASE
#define RALINK_PRGIO_ADDR		RALINK_PIO_BASE // Programmable I/O
#define RALINK_REG_PIODIR		(RALINK_PRGIO_ADDR + 0x00)
#define RALINK_REG_PIOEDGE		(RALINK_PRGIO_ADDR + 0x04)
#define RALINK_REG_PIODATA		(RALINK_PRGIO_ADDR + 0x20)
#define RALINK_REG_PIODATA2		(RALINK_PRGIO_ADDR + 0x24)

#define I2C_ADDRESS 0x44
#define I2C_ACK_IO 0x02
#define I2C_W(x) (x<<7|0x01)
#define I2C_R(x) (x<<7&0xfe)
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum {
	E_NUM_CONFIGURE = 0x01,
	E_NUM_INTERRUPT	= 0x02,
	E_NUM_PXS_LT	= 0x03,
	E_NUM_PXS_HT	= 0x04,
	E_NUM_ALS_TH1	= 0x05,
	E_NUM_ALS_TH2	= 0x06,
	E_NUM_ALS_TH3	= 0x07,
	E_NUM_PXS_DATA	= 0x08,
	E_NUM_ALS_DT1	= 0x09,
	E_NUM_ALS_DT2	= 0x0A,
	E_NUM_ALS_RNG	= 0x0B,
}t_register;
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
static void i2c_init()
{
	//GPIO#1, set IO mode and output
	(*(volatile u32 *)RALINK_REG_GPIOMODE) &= cpu_to_le32(~(0x03<<6));//clear the 6&7 bit
	(*(volatile u32 *)RALINK_REG_GPIOMODE) |=  cpu_to_le32((0x01<<6));//set the 6&7 bit
	(*(volatile u32 *)RALINK_REG_PIODIR) &= cpu_to_le32(~(0x01<<1));
	(*(volatile u32 *)RALINK_REG_PIODIR) |=  cpu_to_le32((0x01<<1));
	
	//RESET,GPIO#36, set IO mode and output
	(*(volatile u32 *)RALINK_REG_GPIOMODE) &= cpu_to_le32(~(0x01<<16));//clear the 16 bit
	(*(volatile u32 *)RALINK_REG_GPIOMODE) |=  cpu_to_le32((0x01<<16));//set the 16 bit
	(*(volatile u32 *)RALINK_REG_PIOEDGE) &= cpu_to_le32(~(0x01<<4));
	(*(volatile u32 *)RALINK_REG_PIOEDGE) |=  cpu_to_le32((0x01<<4));
	
	//MISO,GPIO#6, set IO mode and output
	(*(volatile u32 *)RALINK_REG_GPIOMODE) &= cpu_to_le32(~(0x03<<4));//clear the 4&5 bit
	(*(volatile u32 *)RALINK_REG_GPIOMODE) |=  cpu_to_le32((0x01<<4));//set the 4&5 bit
	(*(volatile u32 *)RALINK_REG_PIODIR) &= cpu_to_le32(~(0x01<<6));
	(*(volatile u32 *)RALINK_REG_PIODIR) |=  cpu_to_le32((0x01<<6));
}

static inline void sda_on()
{
	(*(volatile u32 *)RALINK_REG_PIODATA) |= cpu_to_le32(0x01<<1);
}
static inline void sda_off()
{
	(*(volatile u32 *)RALINK_REG_PIODATA) &= cpu_to_le32(~(0x01<<1));
}

static inline void scl_on()
{
	(*(volatile u32 *)RALINK_REG_PIODATA2) |= cpu_to_le32(0x01<<4);
}
static inline void scl_off()
{
	(*(volatile u32 *)RALINK_REG_PIODATA2) &= cpu_to_le32(~(0x01<<4));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void transport_byte(char bData)
{
	unsigned char i = 0;
	scl_off();
	for(i = 0; i < 8; i ++){
		if(bData){
			sda_on();
		}else{
			sda_off();
		}
		scl_on();
		scl_off();
	}
}
static void i2c_start()
{
	scl_on();
	sda_on();
	sda_off();
	scl_off();
}
static void i2c_end()
{
	scl_on();
	sda_off();
	sda_on();
	scl_off();	
}
static bool i2c_ack()
{
	//read ack
	sda_on();
	scl_on();
	//ack:TODO
	bool iAck = (*(volatile u32 *)(RALINK_REG_PIODATA)& I2C_ACK_IO);
	scl_off();
	
	return iAck;
}
static unsigned char i2c_wirte(char address, char cData)
{
	i2c_start();
	
	transport_byte(I2C_W(I2C_ADDRESS));
	if(i2c_ack()) return 1;
	
	transport_byte(I2C_W(address));
	if(i2c_ack()) return 1;
	
	transport_byte(I2C_W(cData));
	if(i2c_ack()) return 1;
	
	i2c_end();
	return 0;
}
static unsigned char i2c_read(char address, char *pData)
{
	if(pData == NULL) return 1;
	i2c_start();
	
	transport_byte(I2C_W(I2C_ADDRESS));
	if(i2c_ack()) return 1;
	
	transport_byte(I2C_R(address));
	if(i2c_ack()) return 1;
	
	char iRead = 0;
	unsigned char i = 0;
	for(i = 0; i < 8; i ++)
	{
		scl_on();
		iRead = iRead | (*(volatile u32 *)(RALINK_REG_PIODATA)& I2C_ACK_IO);
		iRead<<0x01;
	}
	//send ack
	sda_off();
	i2c_end();
	return 0;	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void device_init()
{
	i2c_wirte(E_NUM_CONFIGURE, 0x84);//enable PXS & ALS
}
static int device_read_als()
{
	int iRet = 0;
	return iRet;
}

static int device_read_pxs()
{
	int iRet = 0;
	return iRet;
}
