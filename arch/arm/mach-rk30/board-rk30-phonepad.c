/* arch/arm/mach-rk30/board-rk30-sdk.c
 *
 * Copyright (C) 2012 ROCKCHIP, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/skbuff.h>
#include <linux/spi/spi.h>
#include <linux/mmc/host.h>
#include <linux/ion.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>
#include <mach/dvfs.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/hardware/gic.h>

#include <mach/board.h>
#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#include <linux/fb.h>
#include <linux/regulator/machine.h>
#include <linux/rfkill-rk.h>
#include <linux/sensor-dev.h>
#include <linux/mfd/tps65910.h>
#include <linux/regulator/rk29-pwm-regulator.h>
#include <plat/rk_fiq_debugger.h>
#include "../../../drivers/headset_observe/rk_headset.h"

#if defined(CONFIG_HDMI_RK30)
	#include "../../../drivers/video/rockchip/hdmi/rk_hdmi.h"
#endif

#ifdef CONFIG_TOUCHSCREEN_GT82X_IIC
#include <linux/goodix_touch_82x.h>
#endif
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_S3202
#include <linux/interrupt.h>
#include <linux/rmi.h>
#endif

#if defined(CONFIG_SPIM_RK29)
#include "../../../drivers/spi/rk29_spim.h"
#endif
#if defined(CONFIG_MU509)
#include <linux/mu509.h>
#endif
#if defined(CONFIG_MW100)
#include <linux/mw100.h>
#endif
#if defined(CONFIG_MT6229)
#include <linux/mt6229.h>
#endif
#if defined(CONFIG_SEW868)
#include <linux/sew868.h>
#endif

#if defined(CONFIG_MI700)
#include <linux/mi700.h>
#endif

#if defined(CONFIG_ANDROID_TIMED_GPIO)
#include "../../../drivers/staging/android/timed_gpio.h"
#endif
#if defined(CONFIG_TOUCHSCREEN_GT8110)
#include <linux/gt8110.h>
#endif

#if defined(CONFIG_MT6620)
#include <linux/gps.h>
#endif

#if defined (CONFIG_TS_AUTO)
#include <linux/ts-auto.h>
#endif

#if defined (CONFIG_BP_AUTO)
#include <linux/bp-auto.h>
#endif
#include <linux/rk_board_id.h>

#ifdef  CONFIG_THREE_FB_BUFFER
#define RK30_FB0_MEM_SIZE 12*SZ_1M
#else
#define RK30_FB0_MEM_SIZE 8*SZ_1M
#endif
#define PMIC_IS_WM831X  0

#ifdef CONFIG_VIDEO_RK29
/*---------------- Camera Sensor Macro Define Begin  ------------------------*/
/*---------------- Camera Sensor Configuration Macro Begin ------------------------*/
#if defined(CONFIG_SOC_CAMERA_SID130B)
#define CONFIG_SENSOR_0 RK29_CAM_SENSOR_SID130B						/* back camera sensor */
#define CONFIG_SENSOR_IIC_ADDR_0		0x6e
#define CONFIG_SENSOR_IIC_ADAPTER_ID_0	  3
#define CONFIG_SENSOR_CIF_INDEX_0          0         // 1
#define CONFIG_SENSOR_ORIENTATION_0 	  90
#define CONFIG_SENSOR_POWER_PIN_0		  INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_0		  INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_0 	  RK30_PIN1_PB6
#define CONFIG_SENSOR_FALSH_PIN_0		  INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_0 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_0 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_0 RK29_CAM_POWERDNACTIVE_L
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_0 RK29_CAM_FLASHACTIVE_L

#define CONFIG_SENSOR_QCIF_FPS_FIXED_0		15000
#define CONFIG_SENSOR_240X160_FPS_FIXED_0   15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_0		15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_0		15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_0		15000
#define CONFIG_SENSOR_480P_FPS_FIXED_0		15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_0		15000
#define CONFIG_SENSOR_720P_FPS_FIXED_0		30000
#endif

#if defined(CONFIG_SOC_CAMERA_GT2005)
#define CONFIG_SENSOR_0 RK29_CAM_SENSOR_GT2005						/* back camera sensor */
#define CONFIG_SENSOR_IIC_ADDR_0		0x78
#define CONFIG_SENSOR_IIC_ADAPTER_ID_0	  3
#define CONFIG_SENSOR_CIF_INDEX_0          0         // 1
#define CONFIG_SENSOR_ORIENTATION_0 	  90
#define CONFIG_SENSOR_POWER_PIN_0		  INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_0		  INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_0 	  RK30_PIN1_PB6
#define CONFIG_SENSOR_FALSH_PIN_0		  INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_0 RK29_CAM_POWERACTIVE_H
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_0 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_0 RK29_CAM_POWERDNACTIVE_L
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_0 RK29_CAM_FLASHACTIVE_L


#define CONFIG_SENSOR_QCIF_FPS_FIXED_0		15000
#define CONFIG_SENSOR_240X160_FPS_FIXED_0   15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_0		15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_0		15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_0		15000
#define CONFIG_SENSOR_480P_FPS_FIXED_0		15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_0		15000
#define CONFIG_SENSOR_720P_FPS_FIXED_0		30000
#endif

#define CONFIG_SENSOR_01  RK29_CAM_SENSOR_HI253                   /* back camera sensor 1 */
#define CONFIG_SENSOR_IIC_ADDR_01 	    0x40
#define CONFIG_SENSOR_CIF_INDEX_01                    0
#define CONFIG_SENSOR_IIC_ADAPTER_ID_01    3
#define CONFIG_SENSOR_ORIENTATION_01       90
#define CONFIG_SENSOR_POWER_PIN_01         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_01         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_01       RK30_PIN1_PB6
#define CONFIG_SENSOR_FALSH_PIN_01         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_01 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_01 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_01 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_01 RK29_CAM_FLASHACTIVE_L

#define CONFIG_SENSOR_QCIF_FPS_FIXED_01      15000
#define CONFIG_SENSOR_240X160_FPS_FIXED_01   15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_01      15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_01       15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_01       15000
#define CONFIG_SENSOR_480P_FPS_FIXED_01      15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_01      15000
#define CONFIG_SENSOR_720P_FPS_FIXED_01     30000

#define CONFIG_SENSOR_02 RK29_CAM_SENSOR_OV5640                      /* back camera sensor 2 */
#define CONFIG_SENSOR_IIC_ADDR_02 	    0x78
#define CONFIG_SENSOR_CIF_INDEX_02                    0
#define CONFIG_SENSOR_IIC_ADAPTER_ID_02    3
#define CONFIG_SENSOR_ORIENTATION_02       90
#define CONFIG_SENSOR_POWER_PIN_02         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_02         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_02       RK30_PIN1_PB6
#define CONFIG_SENSOR_FALSH_PIN_02         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_02 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_02 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_02 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_02 RK29_CAM_FLASHACTIVE_L

#define CONFIG_SENSOR_QCIF_FPS_FIXED_02      15000
#define CONFIG_SENSOR_240X160_FPS_FIXED_02   15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_02      15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_02       15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_02       15000
#define CONFIG_SENSOR_480P_FPS_FIXED_02      15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_02      15000
#define CONFIG_SENSOR_720P_FPS_FIXED_02      30000

#if defined(CONFIG_SOC_CAMERA_SIV121D)
#define CONFIG_SENSOR_1 RK29_CAM_SENSOR_SIV121D                      /* front camera sensor 0 */
#define CONFIG_SENSOR_IIC_ADDR_1 	    0x66
#define CONFIG_SENSOR_IIC_ADAPTER_ID_1	  3
#define CONFIG_SENSOR_CIF_INDEX_1			0	
#define CONFIG_SENSOR_ORIENTATION_1       270
#define CONFIG_SENSOR_POWER_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_1 	  RK30_PIN1_PB7
#define CONFIG_SENSOR_FALSH_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_1 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_1 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_1 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_1 RK29_CAM_FLASHACTIVE_L

#define CONFIG_SENSOR_QCIF_FPS_FIXED_1		15000
#define CONFIG_SENSOR_240X160_FPS_FIXED_1   15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_1		15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_1		15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_1		15000
#define CONFIG_SENSOR_480P_FPS_FIXED_1		15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_1		15000
#define CONFIG_SENSOR_720P_FPS_FIXED_1		30000
#endif

#if defined(CONFIG_SOC_CAMERA_GC0308)
#define CONFIG_SENSOR_1 RK29_CAM_SENSOR_GC0308		/* front camera sensor 0 */
#define CONFIG_SENSOR_IIC_ADDR_1 	    0x42
#define CONFIG_SENSOR_IIC_ADAPTER_ID_1	  3
#define CONFIG_SENSOR_CIF_INDEX_1			0	
#define CONFIG_SENSOR_ORIENTATION_1       270
#define CONFIG_SENSOR_POWER_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_1 	  RK30_PIN1_PB7
#define CONFIG_SENSOR_FALSH_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_1 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_1 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_1 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_1 RK29_CAM_FLASHACTIVE_L


#define CONFIG_SENSOR_QCIF_FPS_FIXED_1		15000
#define CONFIG_SENSOR_240X160_FPS_FIXED_1   15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_1		15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_1		15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_1		15000
#define CONFIG_SENSOR_480P_FPS_FIXED_1		15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_1		15000
#define CONFIG_SENSOR_720P_FPS_FIXED_1		30000
#endif

#define CONFIG_SENSOR_11 RK29_CAM_SENSOR_HI704                      /* front camera sensor 1 */
#define CONFIG_SENSOR_IIC_ADDR_11 	    0x60
#define CONFIG_SENSOR_IIC_ADAPTER_ID_11    3
#define CONFIG_SENSOR_CIF_INDEX_11				  0
#define CONFIG_SENSOR_ORIENTATION_11       270
#define CONFIG_SENSOR_POWER_PIN_11         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_11         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_11       RK30_PIN1_PB7
#define CONFIG_SENSOR_FALSH_PIN_11         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_11 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_11 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_11 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_11 RK29_CAM_FLASHACTIVE_L

#define CONFIG_SENSOR_QCIF_FPS_FIXED_11      15000
#define CONFIG_SENSOR_240X160_FPS_FIXED_11   15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_11      15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_11       15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_11       15000
#define CONFIG_SENSOR_480P_FPS_FIXED_11      15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_11      15000
#define CONFIG_SENSOR_720P_FPS_FIXED_11      30000

#define CONFIG_SENSOR_12 RK29_CAM_SENSOR_OV2659//RK29_CAM_SENSOR_OV2655                      /* front camera sensor 2 */
#define CONFIG_SENSOR_IIC_ADDR_12 	   0x60
#define CONFIG_SENSOR_IIC_ADAPTER_ID_12    3
#define CONFIG_SENSOR_CIF_INDEX_12				  0
#define CONFIG_SENSOR_ORIENTATION_12       270
#define CONFIG_SENSOR_POWER_PIN_12         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_12         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_12       RK30_PIN1_PB7
#define CONFIG_SENSOR_FALSH_PIN_12         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_12 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_12 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_12 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_12 RK29_CAM_FLASHACTIVE_L

#define CONFIG_SENSOR_QCIF_FPS_FIXED_12      15000
#define CONFIG_SENSOR_240X160_FPS_FIXED_12   15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_12      15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_12       15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_12       15000
#define CONFIG_SENSOR_480P_FPS_FIXED_12      15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_12      15000
#define CONFIG_SENSOR_720P_FPS_FIXED_12      30000


#endif  //#ifdef CONFIG_VIDEO_RK29
/*---------------- Camera Sensor Configuration Macro End------------------------*/
#include "../../../drivers/media/video/rk30_camera.c"
/*---------------- Camera Sensor Macro Define End  ---------*/

#define PMEM_CAM_SIZE PMEM_CAM_NECESSARY
/*****************************************************************************************
 * camera  devices
 * author: ddl@rock-chips.com
 *****************************************************************************************/
#ifdef CONFIG_VIDEO_RK29
#define CONFIG_SENSOR_POWER_IOCTL_USR	   1 //define this refer to your board layout
#define CONFIG_SENSOR_RESET_IOCTL_USR	   0
#define CONFIG_SENSOR_POWERDOWN_IOCTL_USR	   0
#define CONFIG_SENSOR_FLASH_IOCTL_USR	   0

static void rk_cif_power(int on)
{
    struct regulator *ldo_18,*ldo_28;
	ldo_28 = regulator_get(NULL, "vmmc");	// vcc28_cif   ldo7
	ldo_18 = regulator_get(NULL, "vdig1");	// vcc18_cif   ldo1
	if (ldo_28 == NULL || IS_ERR(ldo_28) || ldo_18 == NULL || IS_ERR(ldo_18)){
        printk("get cif ldo failed!\n");
		return;
	    }
    if(on == 0){	
    	regulator_disable(ldo_28);
    	regulator_put(ldo_28);
    	regulator_disable(ldo_18);
    	regulator_put(ldo_18);
    	mdelay(500);
        }
    else{
    	regulator_set_voltage(ldo_28, 2800000, 2800000);
    	regulator_enable(ldo_28);
   // 	printk("%s set ldo7 vcc28_cif=%dmV end\n", __func__, regulator_get_voltage(ldo_28));
    	regulator_put(ldo_28);

    	regulator_set_voltage(ldo_18, 1800000, 1800000);
    //	regulator_set_suspend_voltage(ldo, 1800000);
    	regulator_enable(ldo_18);
    //	printk("%s set ldo1 vcc18_cif=%dmV end\n", __func__, regulator_get_voltage(ldo_18));
    	regulator_put(ldo_18);
        }
}

#if CONFIG_SENSOR_POWER_IOCTL_USR
static int sensor_power_usr_cb (struct rk29camera_gpio_res *res,int on)
{
	//#error "CONFIG_SENSOR_POWER_IOCTL_USR is 1, sensor_power_usr_cb function must be writed!!";
    rk_cif_power(on);
}
#endif

#if CONFIG_SENSOR_RESET_IOCTL_USR
static int sensor_reset_usr_cb (struct rk29camera_gpio_res *res,int on)
{
	#error "CONFIG_SENSOR_RESET_IOCTL_USR is 1, sensor_reset_usr_cb function must be writed!!";
}
#endif

#if CONFIG_SENSOR_POWERDOWN_IOCTL_USR
static int sensor_powerdown_usr_cb (struct rk29camera_gpio_res *res,int on)
{
	#error "CONFIG_SENSOR_POWERDOWN_IOCTL_USR is 1, sensor_powerdown_usr_cb function must be writed!!";
}
#endif

#if CONFIG_SENSOR_FLASH_IOCTL_USR
static int sensor_flash_usr_cb (struct rk29camera_gpio_res *res,int on)
{
	#error "CONFIG_SENSOR_FLASH_IOCTL_USR is 1, sensor_flash_usr_cb function must be writed!!";
}
#endif

static struct rk29camera_platform_ioctl_cb	sensor_ioctl_cb = {
	#if CONFIG_SENSOR_POWER_IOCTL_USR
	.sensor_power_cb = sensor_power_usr_cb,
	#else
	.sensor_power_cb = NULL,
	#endif

	#if CONFIG_SENSOR_RESET_IOCTL_USR
	.sensor_reset_cb = sensor_reset_usr_cb,
	#else
	.sensor_reset_cb = NULL,
	#endif

	#if CONFIG_SENSOR_POWERDOWN_IOCTL_USR
	.sensor_powerdown_cb = sensor_powerdown_usr_cb,
	#else
	.sensor_powerdown_cb = NULL,
	#endif

	#if CONFIG_SENSOR_FLASH_IOCTL_USR
	.sensor_flash_cb = sensor_flash_usr_cb,
	#else
	.sensor_flash_cb = NULL,
	#endif
};

#if CONFIG_SENSOR_IIC_ADDR_0
static struct reginfo_t rk_init_data_sensor_reg_0[] =
{
		{0x0000, 0x00,0,0}
	};
static struct reginfo_t rk_init_data_sensor_winseqreg_0[] ={
	{0x0000, 0x00,0,0}
	};
#endif

#if CONFIG_SENSOR_IIC_ADDR_1
static struct reginfo_t rk_init_data_sensor_reg_1[] =
{
    {0x0000, 0x00,0,0}
};
static struct reginfo_t rk_init_data_sensor_winseqreg_1[] =
{
       {0x0000, 0x00,0,0}
};
#endif
#if CONFIG_SENSOR_IIC_ADDR_01
static struct reginfo_t rk_init_data_sensor_reg_01[] =
{
    {0x0000, 0x00,0,0}
};
static struct reginfo_t rk_init_data_sensor_winseqreg_01[] =
{
       {0x0000, 0x00,0,0}
};
#endif
#if CONFIG_SENSOR_IIC_ADDR_02
static struct reginfo_t rk_init_data_sensor_reg_02[] =
{
    {0x0000, 0x00,0,0}
};
static struct reginfo_t rk_init_data_sensor_winseqreg_02[] =
{
       {0x0000, 0x00,0,0}
};
#endif
#if CONFIG_SENSOR_IIC_ADDR_11
static struct reginfo_t rk_init_data_sensor_reg_11[] =
{
    {0x0000, 0x00,0,0}
};
static struct reginfo_t rk_init_data_sensor_winseqreg_11[] =
{
       {0x0000, 0x00,0,0}
};
#endif
#if CONFIG_SENSOR_IIC_ADDR_12
static struct reginfo_t rk_init_data_sensor_reg_12[] =
{
    {0x0000, 0x00,0,0}
};
static struct reginfo_t rk_init_data_sensor_winseqreg_12[] =
{
       {0x0000, 0x00,0,0}
};
#endif
static rk_sensor_user_init_data_s rk_init_data_sensor[RK_CAM_NUM] = 
{
    #if CONFIG_SENSOR_IIC_ADDR_0
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = rk_init_data_sensor_reg_0,
       .rk_sensor_init_winseq = rk_init_data_sensor_winseqreg_0,
       .rk_sensor_winseq_size = sizeof(rk_init_data_sensor_winseqreg_0) / sizeof(struct reginfo_t),
       .rk_sensor_init_data_size = sizeof(rk_init_data_sensor_reg_0) / sizeof(struct reginfo_t),
    },
    #else
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = NULL,
       .rk_sensor_init_winseq = NULL,
       .rk_sensor_winseq_size = 0,
       .rk_sensor_init_data_size = 0,
    },
    #endif
    #if CONFIG_SENSOR_IIC_ADDR_1
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = rk_init_data_sensor_reg_1,
       .rk_sensor_init_winseq = rk_init_data_sensor_winseqreg_1,
       .rk_sensor_winseq_size = sizeof(rk_init_data_sensor_winseqreg_1) / sizeof(struct reginfo_t),
       .rk_sensor_init_data_size = sizeof(rk_init_data_sensor_reg_1) / sizeof(struct reginfo_t),
    },
    #else
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = NULL,
       .rk_sensor_init_winseq = NULL,
       .rk_sensor_winseq_size = 0,
       .rk_sensor_init_data_size = 0,
    },
    #endif
    #if CONFIG_SENSOR_IIC_ADDR_01
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = rk_init_data_sensor_reg_01,
       .rk_sensor_init_winseq = rk_init_data_sensor_winseqreg_01,
       .rk_sensor_winseq_size = sizeof(rk_init_data_sensor_winseqreg_01) / sizeof(struct reginfo_t),
       .rk_sensor_init_data_size = sizeof(rk_init_data_sensor_reg_01) / sizeof(struct reginfo_t),
    },
    #else
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = NULL,
       .rk_sensor_init_winseq = NULL,
       .rk_sensor_winseq_size = 0,
       .rk_sensor_init_data_size = 0,
    },
    #endif
    #if CONFIG_SENSOR_IIC_ADDR_02
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = rk_init_data_sensor_reg_02,
       .rk_sensor_init_winseq = rk_init_data_sensor_winseqreg_02,
       .rk_sensor_winseq_size = sizeof(rk_init_data_sensor_winseqreg_02) / sizeof(struct reginfo_t),
       .rk_sensor_init_data_size = sizeof(rk_init_data_sensor_reg_02) / sizeof(struct reginfo_t),
    },
    #else
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = NULL,
       .rk_sensor_init_winseq = NULL,
       .rk_sensor_winseq_size = 0,
       .rk_sensor_init_data_size = 0,
    },
    #endif
    #if CONFIG_SENSOR_IIC_ADDR_11
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = rk_init_data_sensor_reg_11,
       .rk_sensor_init_winseq = rk_init_data_sensor_winseqreg_11,
       .rk_sensor_winseq_size = sizeof(rk_init_data_sensor_winseqreg_11) / sizeof(struct reginfo_t),
       .rk_sensor_init_data_size = sizeof(rk_init_data_sensor_reg_11) / sizeof(struct reginfo_t),
    },
    #else
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = NULL,
       .rk_sensor_init_winseq = NULL,
       .rk_sensor_winseq_size = 0,
       .rk_sensor_init_data_size = 0,
    },
    #endif
    #if CONFIG_SENSOR_IIC_ADDR_12
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = rk_init_data_sensor_reg_12,
       .rk_sensor_init_winseq = rk_init_data_sensor_winseqreg_12,
       .rk_sensor_winseq_size = sizeof(rk_init_data_sensor_winseqreg_12) / sizeof(struct reginfo_t),
       .rk_sensor_init_data_size = sizeof(rk_init_data_sensor_reg_12) / sizeof(struct reginfo_t),
    },
    #else
    {
       .rk_sensor_init_width = INVALID_VALUE,
       .rk_sensor_init_height = INVALID_VALUE,
       .rk_sensor_init_bus_param = INVALID_VALUE,
       .rk_sensor_init_pixelcode = INVALID_VALUE,
       .rk_sensor_init_data = NULL,
       .rk_sensor_init_winseq = NULL,
       .rk_sensor_winseq_size = 0,
       .rk_sensor_init_data_size = 0,
    },
    #endif

 };
#include "../../../drivers/media/video/rk30_camera.c"

#endif /* CONFIG_VIDEO_RK29 */

#ifdef  CONFIG_TOUCHSCREEN_GT82X_IIC
#define TOUCH_ENABLE_PIN	INVALID_GPIO
#define TOUCH_RESET_PIN  RK30_PIN4_PD0
#define TOUCH_INT_PIN    RK30_PIN4_PC2
int goodix_init_platform_hw(void)
{
	int ret;
	
	rk30_mux_api_set(GPIO4D0_SMCDATA8_TRACEDATA8_NAME, GPIO4D_GPIO4D0);
	rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, GPIO4C_GPIO4C2);
	//printk("%s:0x%x,0x%x\n",__func__,rk30_mux_api_get(GPIO4D0_SMCDATA8_TRACEDATA8_NAME),rk30_mux_api_get(GPIO4C2_SMCDATA2_TRACEDATA2_NAME));
	if (TOUCH_ENABLE_PIN != INVALID_GPIO) {
		ret = gpio_request(TOUCH_ENABLE_PIN, "goodix power pin");
		if (ret != 0) {
			gpio_free(TOUCH_ENABLE_PIN);
			printk("goodix power error\n");
			return -EIO;
		}
		gpio_direction_output(TOUCH_ENABLE_PIN, 0);
		gpio_set_value(TOUCH_ENABLE_PIN, GPIO_LOW);
		msleep(100);
	}

	if (TOUCH_RESET_PIN != INVALID_GPIO) {
		ret = gpio_request(TOUCH_RESET_PIN, "goodix reset pin");
		if (ret != 0) {
			gpio_free(TOUCH_RESET_PIN);
			printk("goodix gpio_request error\n");
			return -EIO;
		}
		gpio_direction_output(TOUCH_RESET_PIN, 0);
		gpio_set_value(TOUCH_RESET_PIN, GPIO_LOW);
		msleep(10);
		gpio_set_value(TOUCH_RESET_PIN, GPIO_HIGH);
		msleep(500);
	}
	return 0;
}
u8 ts82x_config_data[] = {
	0x65,0x00,0x04,0x00,0x03,0x00,0x0A,0x0D,0x1E,0xE7,
	0x32,0x03,0x08,0x10,0x48,0x42,0x42,0x20,0x00,0x01,
	0x60,0x60,0x4B,0x6E,0x0E,0x0D,0x0C,0x0B,0x0A,0x09,
	0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00,0x1D,
	0x1C,0x1B,0x1A,0x19,0x18,0x17,0x16,0x15,0x14,0x13,
	0x12,0x11,0x10,0x0F,0x50,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2B,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
};
static struct goodix_i2c_rmi_platform_data ts82x_pdata = {
    .gpio_shutdown = TOUCH_ENABLE_PIN,
    .gpio_irq = TOUCH_INT_PIN,
    .gpio_reset = TOUCH_RESET_PIN,
    .irq_edge = 1, /* 0:rising edge, 1:falling edge */

    .ypol = 1,
	.swap_xy = 1,
	.xpol = 0,
	.xmax = 1024,
    .ymax = 600,
    .config_info_len =ARRAY_SIZE(ts82x_config_data),
    .config_info = ts82x_config_data,
	.init_platform_hw= goodix_init_platform_hw,
};
#endif

#if defined (CONFIG_RK_HEADSET_DET) || defined (CONFIG_RK_HEADSET_IRQ_HOOK_ADC_DET)

static int rk_headset_io_init(int gpio)
{
		int ret;
		ret = gpio_request(gpio, "headset_input");
		if(ret) 
			return ret;

		rk30_mux_api_set(GPIO0C7_TRACECTL_SMCADDR3_NAME, GPIO0C_GPIO0C7);
		gpio_pull_updown(gpio, PullDisable);
		gpio_direction_input(gpio);
		mdelay(50);
		return 0;
};

struct rk_headset_pdata rk_headset_info = {
		.Headset_gpio		= RK30_PIN0_PC7,
		.headset_in_type = HEADSET_IN_LOW,
		.Hook_adc_chn = 2,
		.hook_key_code = KEY_MEDIA,
		.headset_io_init = rk_headset_io_init,
};

struct platform_device rk_device_headset = {
		.name	= "rk_headsetdet",
		.id 	= 0,
		.dev    = {
			    .platform_data = &rk_headset_info,
		}
};
#endif

#if defined (CONFIG_TOUCHSCREEN_SYNAPTICS_S3202)

#define TOUCH_RESET_PIN RK30_PIN4_PD0
#define TOUCH_INT_PIN	RK30_PIN4_PC2
#define TOUCH_POWER_PIN -1
#define TOUCH_IO_POWER_PIN -1

struct syna_gpio_data {
	u16 gpio_number;
	char* gpio_name;
};

int syna_init_platform_hw(void)
{
    return 0;
}

static int synaptics_touchpad_gpio_setup(void *gpio_data, bool configure)
{
	int retval=0;
	struct syna_gpio_data *data = gpio_data;

	if (configure) {
		retval = gpio_request(data->gpio_number, "rmi4_attn");
		if (retval) {
			pr_err("%s: Failed to get attn gpio %d. Code: %d.",
			       __func__, data->gpio_number, retval);
			return retval;
		}
		rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, 0);
		retval = gpio_direction_input(data->gpio_number);
		if (retval) {
			pr_err("%s: Failed to setup attn gpio %d. Code: %d.",
			       __func__, data->gpio_number, retval);
			gpio_free(data->gpio_number);
		}
	} else {
		printk("%s: No way to deconfigure gpio %d.",
		       __func__, data->gpio_number);
	}

	return retval;

}

static struct syna_gpio_data s3202_gpiodata = {
	.gpio_number = TOUCH_INT_PIN,
	.gpio_name = "GPIO4_C2",
};
static unsigned char s3202_key_array[4]={ KEY_BACK, KEY_MENU, KEY_HOMEPAGE, KEY_SEARCH };

struct rmi_f1a_button_map s3202_buttons = {
		.nbuttons = 0,
		.map = s3202_key_array,
};

static struct rmi_device_platform_data s3202_platformdata = {
	.sensor_name = "Espresso",
	.driver_name = "rmi_generic",
	.attn_gpio = TOUCH_INT_PIN,
	.attn_polarity = RMI_ATTN_ACTIVE_LOW,
	.level_triggered = false,	/* For testing */
	.gpio_data = &s3202_gpiodata,
	.gpio_config = synaptics_touchpad_gpio_setup,
	.init_hw = syna_init_platform_hw,
	.axis_align = {
		.swap_axes = 1,
		.flip_x = 0,
		.flip_y = 0,
		.clip_X_low = 0,
		.clip_Y_low = 0,
		.clip_X_high = 0,
		.clip_Y_high = 0,
	},
	.f1a_button_map = &s3202_buttons,
};

#endif


#if defined(CONFIG_TOUCHSCREEN_GT8XX)
#define TOUCH_RESET_PIN  RK30_PIN4_PD0
#define TOUCH_PWR_PIN    INVALID_GPIO

int goodix_init_platform_hw(void)
{
	int ret;
	
	rk30_mux_api_set(GPIO4D0_SMCDATA8_TRACEDATA8_NAME, GPIO4D_GPIO4D0);
	rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, GPIO4C_GPIO4C2);
	printk("%s:0x%x,0x%x\n",__func__,rk30_mux_api_get(GPIO4D0_SMCDATA8_TRACEDATA8_NAME),rk30_mux_api_get(GPIO4C2_SMCDATA2_TRACEDATA2_NAME));

	if (TOUCH_PWR_PIN != INVALID_GPIO) {
		ret = gpio_request(TOUCH_PWR_PIN, "goodix power pin");
		if (ret != 0) {
			gpio_free(TOUCH_PWR_PIN);
			printk("goodix power error\n");
			return -EIO;
		}
		gpio_direction_output(TOUCH_PWR_PIN, 0);
		gpio_set_value(TOUCH_PWR_PIN, GPIO_LOW);
		msleep(100);
	}

	if (TOUCH_RESET_PIN != INVALID_GPIO) {
		ret = gpio_request(TOUCH_RESET_PIN, "goodix reset pin");
		if (ret != 0) {
			gpio_free(TOUCH_RESET_PIN);
			printk("goodix gpio_request error\n");
			return -EIO;
		}
		gpio_direction_output(TOUCH_RESET_PIN, 1);
                msleep(100);
		//gpio_set_value(TOUCH_RESET_PIN, GPIO_LOW);
		//msleep(100);
		//gpio_set_value(TOUCH_RESET_PIN, GPIO_HIGH);
		//msleep(500);
	}
	return 0;
}

struct goodix_platform_data goodix_info = {
	.model = 8105,
	.irq_pin = RK30_PIN4_PC2,
	.rest_pin = TOUCH_RESET_PIN,
	.init_platform_hw = goodix_init_platform_hw,
};
#endif

#if defined(CONFIG_TOUCHSCREEN_FT5306)

#define TOUCH_RESET_PIN	 RK30_PIN4_PD0
#define TOUCH_INT_PIN 	 RK30_PIN4_PC2
int ft5306_init_platform_hw(void)
{
	printk("ft5306_init_platform_hw\n");

	rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, 0);
	if(gpio_request(TOUCH_RESET_PIN,NULL) != 0){
		gpio_free(TOUCH_RESET_PIN);
		printk("ft5406_init_platform_hw gpio_request error\n");
		return -EIO;
	}

	if(gpio_request(TOUCH_INT_PIN,NULL) != 0){
		gpio_free(TOUCH_INT_PIN);
		printk("ift5406_init_platform_hw gpio_request error\n");
		return -EIO;
	}
	
	rk30_mux_api_set(GPIO4D0_SMCDATA8_TRACEDATA8_NAME, 0);

	gpio_direction_output(TOUCH_RESET_PIN, 0);
	gpio_set_value(TOUCH_RESET_PIN,GPIO_LOW);
	mdelay(10);
	gpio_direction_input(TOUCH_INT_PIN);
	mdelay(10);
	gpio_set_value(TOUCH_RESET_PIN,GPIO_HIGH);
	msleep(300);
	return 0;

}

void ft5306_exit_platform_hw(void)
{
	printk("ft5306_exit_platform_hw\n");
	gpio_free(TOUCH_RESET_PIN);
	gpio_free(TOUCH_INT_PIN);
}

int ft5306_platform_sleep(void)
{
	//printk("ft5306_platform_sleep\n");
	gpio_set_value(TOUCH_RESET_PIN,GPIO_LOW);
	return 0;
}

int ft5306_platform_wakeup(void)
{
	//printk("ft5306_platform_wakeup\n");
	gpio_set_value(TOUCH_RESET_PIN,GPIO_HIGH);
	msleep(300);
	return 0;
}

struct ft5x0x_platform_data ft5306_info = {
  .model = 5306,
  .max_x = 1024,
  .max_y = 768,
  .key_min_x = 1024,
  .xy_swap = 1,
  .x_revert = 1,
  .y_revert = 0,
  .init_platform_hw= ft5306_init_platform_hw,
  .exit_platform_hw= ft5306_exit_platform_hw,
  .ft5x0x_platform_sleep  = ft5306_platform_sleep,
  .ft5x0x_platform_wakeup = ft5306_platform_wakeup,
};


#endif

#if defined(CONFIG_TOUCHSCREEN_CT360_IIC)
#define TOUCH_RESET_PIN	 RK30_PIN4_PD0
#define TOUCH_INT_PIN 	 RK30_PIN4_PC2

static void ct360_hw_init(void)
{
	int ret;

	printk("%s\n", __FUNCTION__);

	rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, 0);
	rk30_mux_api_set(GPIO4D0_SMCDATA8_TRACEDATA8_NAME, 0);
	
	if(TOUCH_RESET_PIN != INVALID_GPIO){
		gpio_request(TOUCH_RESET_PIN, "ct360_reset");
		gpio_direction_output(TOUCH_RESET_PIN, GPIO_HIGH);
	}

	if(TOUCH_INT_PIN != INVALID_GPIO){
		ret = gpio_request(TOUCH_INT_PIN, "ct360_irq");
		if(ret != 0){
			gpio_free(TOUCH_INT_PIN);
			printk("%s: ct360 irq request err\n", __func__);
		}
		else{
			gpio_direction_input(TOUCH_INT_PIN);
			gpio_pull_updown(TOUCH_INT_PIN, PullEnable);
		}
	}
}

static void ct360_hw_shutdown(int reset)
{
	if(TOUCH_RESET_PIN != INVALID_GPIO){
		if(reset){
			gpio_set_value(TOUCH_RESET_PIN, GPIO_HIGH);
		}
		else{
			gpio_set_value(TOUCH_RESET_PIN, GPIO_LOW);
		}
	}
}


static struct ct360_platform_data ct360_info = {
	.model	 = 360,
	.x_max	 = 800,
	.y_max	 = 480,
	.hw_init = ct360_hw_init,
	.shutdown = ct360_hw_shutdown,
};

#endif


#ifdef CONFIG_TOUCHSCREEN_GT8110 
#define TOUCH_ENABLE_PIN	INVALID_GPIO
#define TOUCH_INT_PIN		RK30_PIN4_PC2
#define TOUCH_RESET_PIN		RK30_PIN4_PD0
int goodix_init_platform_hw(void)
{

	int ret;
	rk30_mux_api_set(GPIO4D0_SMCDATA8_TRACEDATA8_NAME, GPIO4D_GPIO4D0);
	rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, GPIO4C_GPIO4C2);
	printk("%s:0x%x,0x%x\n",__func__,rk30_mux_api_get(GPIO4D0_SMCDATA8_TRACEDATA8_NAME),rk30_mux_api_get(GPIO4C2_SMCDATA2_TRACEDATA2_NAME));

	if (TOUCH_ENABLE_PIN != INVALID_GPIO) {
		ret = gpio_request(TOUCH_ENABLE_PIN, "goodix power pin");
		if (ret != 0) {
			gpio_free(TOUCH_ENABLE_PIN);
			printk("goodix power error\n");
			return -EIO;
		}
		gpio_direction_output(TOUCH_ENABLE_PIN, 0);
		gpio_set_value(TOUCH_ENABLE_PIN, GPIO_LOW);
		msleep(100);
	}

	if (TOUCH_RESET_PIN != INVALID_GPIO) {
		ret = gpio_request(TOUCH_RESET_PIN, "goodix reset pin");
		if (ret != 0) {
			gpio_free(TOUCH_RESET_PIN);
			printk("goodix gpio_request error\n");
			return -EIO;
		}
		gpio_direction_output(TOUCH_RESET_PIN, 0);
        	//msleep(100);
		gpio_set_value(TOUCH_RESET_PIN, GPIO_LOW);
		msleep(100);
		//gpio_set_value(TOUCH_RESET_PIN, GPIO_HIGH);
		//msleep(500);
	}
	return 0;
}

int goodix_exit_platform_hw(void)
{
    return 0;
}

struct goodix_8110_platform_data  goodix_info = {
	.irq_pin = TOUCH_INT_PIN,
	.reset= TOUCH_RESET_PIN,
        .hw_init = goodix_init_platform_hw,
        .hw_exit = goodix_exit_platform_hw,
};
#endif

#if defined(CONFIG_TS_AUTO_I2C)
#define TOUCH_RESET_PIN	 RK30_PIN4_PD0
#define TOUCH_INT_PIN 	 RK30_PIN4_PC2
int ts_init_platform_hw(void)
{
	rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, 0);	
	rk30_mux_api_set(GPIO4D0_SMCDATA8_TRACEDATA8_NAME, 0);

	gpio_request(TOUCH_INT_PIN, "ts_irq_pin");
	gpio_request(TOUCH_RESET_PIN, "ts_reset_pin");
	gpio_direction_output(TOUCH_RESET_PIN, GPIO_LOW);
	msleep(10);
	gpio_direction_output(TOUCH_RESET_PIN, GPIO_HIGH);
	msleep(200);
	printk("%s\n",__func__);
	return 0;
}

struct ts_platform_data  auto_ts_info = {
	.irq = TOUCH_INT_PIN,
	.power_pin = INVALID_GPIO,
	.reset_pin = TOUCH_RESET_PIN,
	.init_platform_hw = ts_init_platform_hw,
};

#endif

static struct spi_board_info board_spi_devices[] = {
};

/***********************************************************
*	rk30  backlight
************************************************************/
#ifdef CONFIG_BACKLIGHT_RK29_BL
#define PWM_ID            2
#define PWM_MUX_NAME      GPIO0D6_PWM2_NAME
#define PWM_MUX_MODE      GPIO0D_PWM2
#define PWM_MUX_MODE_GPIO GPIO0A_GPIO0A3
#define PWM_GPIO 	  RK30_PIN0_PD6
#define PWM_EFFECT_VALUE  1

#define LCD_DISP_ON_PIN

#ifdef  LCD_DISP_ON_PIN
//#define BL_EN_MUX_NAME    GPIOF34_UART3_SEL_NAME
//#define BL_EN_MUX_MODE    IOMUXB_GPIO1_B34

#define BL_EN_PIN         RK30_PIN6_PB3
#define BL_EN_VALUE       GPIO_HIGH
#endif
static int rk29_backlight_io_init(void)
{
	int ret = 0;
	//rk30_mux_api_set(GPIO0D6_PWM2_NAME, GPIO0D_GPIO0D6);
	//gpio_request(RK30_PIN0_PD6, NULL);
	//gpio_direction_output(RK30_PIN0_PD6, GPIO_HIGH);

	rk30_mux_api_set(GPIO0D6_PWM2_NAME, GPIO0D_PWM2);
	rk30_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE);
#ifdef  LCD_DISP_ON_PIN
	// rk30_mux_api_set(BL_EN_MUX_NAME, BL_EN_MUX_MODE);

	ret = gpio_request(BL_EN_PIN, NULL);
	if (ret != 0) {
		gpio_free(BL_EN_PIN);
	}

	gpio_direction_output(BL_EN_PIN, BL_EN_VALUE);
	gpio_set_value(BL_EN_PIN, BL_EN_VALUE);
#endif
	return ret;
}

static int rk29_backlight_io_deinit(void)
{
	int ret = 0;
#ifdef  LCD_DISP_ON_PIN
	gpio_free(BL_EN_PIN);
#endif
	rk30_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE_GPIO);
	return ret;
}

static int rk29_backlight_pwm_suspend(void)
{
	int ret = 0;
	rk30_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE_GPIO);
	if (gpio_request(PWM_GPIO, NULL)) {
		printk("func %s, line %d: request gpio fail\n", __FUNCTION__, __LINE__);
		return -1;
	}
	gpio_direction_output(PWM_GPIO, !PWM_EFFECT_VALUE);
#ifdef  LCD_DISP_ON_PIN
	gpio_direction_output(BL_EN_PIN, 0);
	gpio_set_value(BL_EN_PIN, !BL_EN_VALUE);
#endif
	return ret;
}

static int rk29_backlight_pwm_resume(void)
{
	gpio_free(PWM_GPIO);
	rk30_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE);
#ifdef  LCD_DISP_ON_PIN
	msleep(30);
	gpio_direction_output(BL_EN_PIN, 1);
	gpio_set_value(BL_EN_PIN, BL_EN_VALUE);
#endif
	return 0;
}

static struct rk29_bl_info rk29_bl_info = {
	.pwm_id = PWM_ID,
	.bl_ref = PWM_EFFECT_VALUE,
	.io_init = rk29_backlight_io_init,
	.io_deinit = rk29_backlight_io_deinit,
	.pwm_suspend = rk29_backlight_pwm_suspend,
	.pwm_resume = rk29_backlight_pwm_resume,
	.pre_div = 20000,
};

static struct platform_device rk29_device_backlight = {
	.name	= "rk29_backlight",
	.id 	= -1,
	.dev	= {
		.platform_data  = &rk29_bl_info,
	}
};

#endif

#if defined (CONFIG_SND_SOC_RT3224) || defined (CONFIG_SND_SOC_RT3261)

#define DIFFERENTIAL 1
#define SINGLE_END 0
#define TWO_SPK 2
#define ONE_SPK 1

enum {
	SPK_AMPLIFY_ZERO_POINT_FIVE_WATT=1,
	SPK_AMPLIFY_ZERO_POINT_SIX_WATT,
	SPK_AMPLIFY_ZERO_POINT_EIGHT_WATT,
	SPK_AMPLIFY_ONE_WATT,
};

enum {
	LR_NORMAL,
	LR_SWAP,
	LEFT_COPY_TO_RIGHT,
	RIGHT_COPY_LEFT,
};

static int rt3261_io_init(int gpio, char *iomux_name, int iomux_mode)
{
	gpio_request(gpio,NULL);
	rk30_mux_api_set(iomux_name, iomux_mode);
	gpio_direction_output(gpio,1);
	
};

static struct rt3261_platform_data rt3261_info = {
	.codec_en_gpio 			= RK30_PIN4_PD7,
	.codec_en_gpio_info		= {GPIO4D7_SMCDATA15_TRACEDATA15_NAME,GPIO4D_GPIO4D7},
	.io_init			= rt3261_io_init,
	.spk_num 			= TWO_SPK,
	.modem_input_mode		= DIFFERENTIAL,
	.lout_to_modem_mode		= DIFFERENTIAL,
	.spk_amplify			= SPK_AMPLIFY_ZERO_POINT_SIX_WATT,
	.playback_if1_data_control	= LR_NORMAL,
	.playback_if2_data_control	= LR_NORMAL,
};
#endif

#ifdef CONFIG_RK29_SUPPORT_MODEM

#define RK30_MODEM_POWER        RK30_PIN4_PD1
#define RK30_MODEM_POWER_IOMUX  rk29_mux_api_set(GPIO4D1_SMCDATA9_TRACEDATA9_NAME, GPIO4D_GPIO4D1)

static int rk30_modem_io_init(void)
{
    printk("%s\n", __FUNCTION__);
    RK30_MODEM_POWER_IOMUX;

	return 0;
}

static struct rk29_io_t rk30_modem_io = {
    .io_addr    = RK30_MODEM_POWER,
    .enable     = GPIO_HIGH,
    .disable    = GPIO_LOW,
    .io_init    = rk30_modem_io_init,
};

static struct platform_device rk30_device_modem = {
	.name	= "rk30_modem",
	.id 	= -1,
	.dev	= {
		.platform_data  = &rk30_modem_io,
	}
};
#endif
#if defined(CONFIG_MU509)
static int mu509_io_init(void)
{

	rk30_mux_api_set(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME, GPIO2B_GPIO2B6);
        rk30_mux_api_set(GPIO4D2_SMCDATA10_TRACEDATA10_NAME, GPIO4D_GPIO4D2);
	rk30_mux_api_set(GPIO2B7_LCDC1DATA15_SMCADDR19_HSADCDATA7_NAME, GPIO2B_GPIO2B7);
	rk30_mux_api_set(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME, GPIO2C_GPIO2C0);
	return 0;
}

static int mu509_io_deinit(void)
{
	
	return 0;
}
 
struct rk29_mu509_data rk29_mu509_info = {
	.io_init = mu509_io_init,
  	.io_deinit = mu509_io_deinit,
	.modem_power_en = RK30_PIN6_PB2,//RK30_PIN4_PD1,
	.bp_power = RK30_PIN2_PB6,//RK30_PIN4_PD1,
	.bp_reset = RK30_PIN4_PD2,
	.ap_wakeup_bp = RK30_PIN2_PB7,
	.bp_wakeup_ap = RK30_PIN6_PA0, 
};
struct platform_device rk29_device_mu509 = {	
        .name = "mu509",	
    	.id = -1,	
	.dev		= {
		.platform_data = &rk29_mu509_info,
	}    	
    };
#endif
#if defined(CONFIG_MW100)
static int mw100_io_init(void)
{
	 rk30_mux_api_set(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME, GPIO2B_GPIO2B6);
	 rk30_mux_api_set(GPIO4D2_SMCDATA10_TRACEDATA10_NAME, GPIO4D_GPIO4D2);
	 rk30_mux_api_set(GPIO2B7_LCDC1DATA15_SMCADDR19_HSADCDATA7_NAME, GPIO2B_GPIO2B7);
	 rk30_mux_api_set(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME, GPIO2C_GPIO2C0);
	return 0;
}

static int mw100_io_deinit(void)
{
	
	return 0;
}
 
struct rk29_mw100_data rk29_mw100_info = {
	.io_init = mw100_io_init,
  	.io_deinit = mw100_io_deinit,
	.modem_power_en = RK30_PIN6_PB2,
	.bp_power = RK30_PIN2_PB6,
	.bp_reset = RK30_PIN4_PD2,
	.ap_wakeup_bp = RK30_PIN2_PB7,
	.bp_wakeup_ap = RK30_PIN6_PA0,
};
struct platform_device rk29_device_mw100 = {	
        .name = "mw100",	
    	.id = -1,	
	.dev		= {
		.platform_data = &rk29_mw100_info,
	}    	
    };
#endif
#if defined(CONFIG_MT6229)
static int mt6229_io_init(void)
{
	 rk30_mux_api_set(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME, GPIO2B_GPIO2B6);
	 rk30_mux_api_set(GPIO4D2_SMCDATA10_TRACEDATA10_NAME, GPIO4D_GPIO4D2);
	 rk30_mux_api_set(GPIO2B7_LCDC1DATA15_SMCADDR19_HSADCDATA7_NAME, GPIO2B_GPIO2B7);
	 rk30_mux_api_set(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME, GPIO2C_GPIO2C0);
	 rk30_mux_api_set(GPIO2C1_LCDC1DATA17_SMCBLSN0_HSADCDATA6_NAME, GPIO2C_GPIO2C1);
	 rk30_mux_api_set(GPIO2C1_LCDC1DATA17_SMCBLSN0_HSADCDATA6_NAME, GPIO2C_GPIO2C1);

	 return 0;
}

static int mt6229_io_deinit(void)
{
	
	return 0;
}
 
struct rk29_mt6229_data rk29_mt6229_info = {
	.io_init = mt6229_io_init,
	.io_deinit = mt6229_io_deinit,
	.modem_power_en = RK30_PIN6_PB2, // 3g_power
	.bp_power = RK30_PIN2_PB6,	// 3g_en
	.modem_usb_en = RK30_PIN2_PC0, //W_disable
	.modem_uart_en = RK30_PIN2_PC1, //EINT9
	.bp_wakeup_ap = RK30_PIN6_PA1,	//
	.ap_ready = RK30_PIN2_PB7,	//

};
struct platform_device rk29_device_mt6229 = {	
        .name = "mt6229",	
    	.id = -1,	
	.dev		= {
		.platform_data = &rk29_mt6229_info,
	}    	
    };
#endif
#if defined(CONFIG_SEW868)
static int sew868_io_init(void)
{
	rk30_mux_api_set(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME, GPIO2B_GPIO2B6);
    rk30_mux_api_set(GPIO4D2_SMCDATA10_TRACEDATA10_NAME, GPIO4D_GPIO4D2);
	rk30_mux_api_set(GPIO4D4_SMCDATA12_TRACEDATA12_NAME, GPIO4D_GPIO4D4);
	return 0;
}
static int sew868_io_deinit(void)
{
	return 0;
}
struct rk30_sew868_data rk30_sew868_info = {
	.io_init = sew868_io_init,
  	.io_deinit = sew868_io_deinit,
	.bp_power = RK30_PIN6_PB2, 
	.bp_power_active_low = 1,
	.bp_sys = RK30_PIN2_PB6, 
	.bp_reset = RK30_PIN4_PD2, 
	.bp_reset_active_low = 1,
	.bp_wakeup_ap = RK30_PIN4_PD4, 
	.ap_wakeup_bp = NULL,
};

struct platform_device rk30_device_sew868 = {	
        .name = "sew868",	
    	.id = -1,	
	.dev		= {
		.platform_data = &rk30_sew868_info,
	}    	
    };
#endif
#if defined(CONFIG_MI700)
#define BP_POWER        RK29_PIN6_PB1   
#define BP_RESET        RK29_PIN6_PC7
static int mi700_io_init(void)
{
        int result;
        result = gpio_request(BP_RESET, NULL);
        if (result)
        {
                gpio_free(BP_RESET);
                printk("failed to request BP_RESET gpio\n");
        }
        result = gpio_request(BP_POWER, NULL);
        if (result)
        {
                gpio_free(BP_POWER);
                printk("failed to request BP_POWER gpio\n");
        }
        return 0;
}

static int mi700_io_deinit(void)
{
        gpio_free(BP_RESET);
        gpio_free(BP_POWER);

        return 0;
}

struct rk29_mi700_data rk29_mi700_info = {
        .io_init = mi700_io_init,
        .io_deinit = mi700_io_deinit,
        .bp_power = RK29_PIN6_PB1,//RK29_PIN0_PB4,
        .bp_reset = RK29_PIN6_PC7,//RK29_PIN0_PB3,
        .bp_wakeup_ap = RK29_PIN6_PC6,//RK29_PIN0_PC2,
        .ap_wakeup_bp = NULL,//RK29_PIN0_PB0, 
};
struct platform_device rk29_device_mi700 = {
        .name = "MI700",
        .id = -1,
        .dev            = {
                .platform_data = &rk29_mi700_info,
        }
    };
#endif

#if defined(CONFIG_BP_AUTO)
static int bp_io_init(void)
{
	 rk30_mux_api_set(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME, GPIO2B_GPIO2B6);
	 rk30_mux_api_set(GPIO4D2_SMCDATA10_TRACEDATA10_NAME, GPIO4D_GPIO4D2);
	 rk30_mux_api_set(GPIO2B7_LCDC1DATA15_SMCADDR19_HSADCDATA7_NAME, GPIO2B_GPIO2B7);
	 rk30_mux_api_set(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME, GPIO2C_GPIO2C0);
	 rk30_mux_api_set(GPIO2C1_LCDC1DATA17_SMCBLSN0_HSADCDATA6_NAME, GPIO2C_GPIO2C1);
	 rk30_mux_api_set(GPIO2C1_LCDC1DATA17_SMCBLSN0_HSADCDATA6_NAME, GPIO2C_GPIO2C1);

	 return 0;
}

static int bp_io_deinit(void)
{
	
	return 0;
}
 

struct bp_platform_data bp_auto_info = {	
	.board_id = BOARD_ID_C8003,
	.bp_id = BP_ID_MT6229,
	.init_platform_hw = bp_io_init,	
	.exit_platform_hw = bp_io_deinit,	
	.bp_power = RK30_PIN6_PB2, 	// 3g_power
	.bp_en = RK30_PIN2_PB6, 	// 3g_en
	.bp_usb_en = RK30_PIN2_PC0, 	//W_disable
	.bp_uart_en = RK30_PIN2_PC1, 	//EINT9
	.bp_wakeup_ap = RK30_PIN6_PA1,	//
	.ap_ready = RK30_PIN2_PB7,	//
	.gpio_valid = 0,		//don't use this gpios
};

struct platform_device device_bp_auto = {	
        .name = "bp-auto",	
    	.id = -1,	
	.dev		= {
		.platform_data = &bp_auto_info,
	}    	
    };
#endif



/*MMA8452 gsensor*/
#if defined (CONFIG_GS_MMA8452)
#define MMA8452_INT_PIN   RK30_PIN4_PC0

static int mma8452_init_platform_hw(void)
{
	rk30_mux_api_set(GPIO4C0_SMCDATA0_TRACEDATA0_NAME, GPIO4C_GPIO4C0);

	return 0;
}

static struct sensor_platform_data mma8452_info = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
        .init_platform_hw = mma8452_init_platform_hw,
        .orientation = {-1, 0, 0, 0, 0, 1, 0, -1, 0},
};
#endif
#if defined (CONFIG_GS_LIS3DH)
#define LIS3DH_INT_PIN   RK30_PIN4_PC0

static int lis3dh_init_platform_hw(void)
{
        rk30_mux_api_set(GPIO4C0_SMCDATA0_TRACEDATA0_NAME, GPIO4C_GPIO4C0);

        return 0;
}

static struct sensor_platform_data lis3dh_info = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
        .init_platform_hw = lis3dh_init_platform_hw,
	.orientation = {0, -1, 0, 0, 0, -1, -1, 0, 0},
};
#endif
#if defined (CONFIG_GS_KXTIK)
#define KXTIK_INT_PIN   RK30_PIN4_PC0

static int kxtik_init_platform_hw(void)
{
	rk30_mux_api_set(GPIO4C0_SMCDATA0_TRACEDATA0_NAME, GPIO4C_GPIO4C0);

	return 0;
}

static struct sensor_platform_data kxtik_info = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
	.init_platform_hw = kxtik_init_platform_hw,
	.orientation = {0, 1, 0, 0, 0, -1, 1, 0, 0},
};

#endif

/*MMA7660 gsensor*/
#if defined (CONFIG_GS_MMA7660)
#define MMA7660_INT_PIN   RK30_PIN4_PC0

static int mma7660_init_platform_hw(void)
{
	rk30_mux_api_set(GPIO4C0_SMCDATA0_TRACEDATA0_NAME, GPIO4C_GPIO4C0);

	return 0;
}

static struct sensor_platform_data mma7660_info = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
        .init_platform_hw = mma7660_init_platform_hw,
        .orientation = {0, 1, 0, 0, 0, -1, 1, 0, 0},
};
#endif

#if defined (CONFIG_COMPASS_AK8975)
static struct sensor_platform_data akm8975_info =
{
	.type = SENSOR_TYPE_COMPASS,
	.irq_enable = 1,
	.poll_delay_ms = 30,
	.m_layout = 
	{
		{
			{0, 1, 0},
			{1, 0, 0},	
			{0, 0, -1},
		},

		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},

		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},

		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},
	}
};

#endif

#if defined(CONFIG_GYRO_L3G4200D)

#include <linux/l3g4200d.h>
#define L3G4200D_INT_PIN  RK30_PIN4_PC3

static int l3g4200d_init_platform_hw(void)
{
	rk30_mux_api_set(GPIO4C3_SMCDATA3_TRACEDATA3_NAME, GPIO4C_GPIO4C3);
	
	return 0;
}

static struct sensor_platform_data l3g4200d_info = {
	.type = SENSOR_TYPE_GYROSCOPE,
	.irq_enable = 1,
	.poll_delay_ms = 30,
	.orientation = {0, 1, 0, -1, 0, 0, 0, 0, -1},
	.init_platform_hw = l3g4200d_init_platform_hw,
	.x_min = 40,//x_min,y_min,z_min = (0-100) according to hardware
	.y_min = 40,
	.z_min = 20,
};

#endif

#ifdef CONFIG_LS_CM3217
static struct sensor_platform_data cm3217_info = {
	.type = SENSOR_TYPE_LIGHT,
	.irq_enable = 0,
	.poll_delay_ms = 500,
};

#endif


#if defined(CONFIG_PS_AL3006)
static struct sensor_platform_data proximity_al3006_info = {
	.type = SENSOR_TYPE_PROXIMITY,
	.irq_enable = 1,
	.poll_delay_ms = 200,
};
#endif

#if defined(CONFIG_PS_STK3171)
static struct sensor_platform_data proximity_stk3171_info = {
	.type = SENSOR_TYPE_PROXIMITY,
	.irq_enable = 1,
	.poll_delay_ms = 200,
};
#endif


#if defined(CONFIG_LS_AL3006)
static struct sensor_platform_data light_al3006_info = {
	.type = SENSOR_TYPE_LIGHT,
	.irq_enable = 1,
	.poll_delay_ms = 200,
};
#endif

#if defined(CONFIG_LS_STK3171)
static struct sensor_platform_data light_stk3171_info = {
	.type = SENSOR_TYPE_LIGHT,
	.irq_enable = 1,
	.poll_delay_ms = 200,
};
#endif

#if defined(CONFIG_LS_ISL29023)
static struct sensor_platform_data light_isl29023_info = {
	.type = SENSOR_TYPE_LIGHT,
	.irq_enable = 1,
	.poll_delay_ms = 200,
};
#endif




#ifdef CONFIG_FB_ROCKCHIP

#define LCD_CS_MUX_NAME    GPIO4C7_SMCDATA7_TRACEDATA7_NAME
#define LCD_CS_PIN         RK30_PIN4_PC7
#define LCD_CS_VALUE       GPIO_HIGH

#define LCD_STANDBY_MUX_NAME    GPIO4D2_SMCDATA10_TRACEDATA10_NAME
#define LCD_STANDBY_PIN         RK30_PIN4_PD2
#define LCD_STANDBY_VALUE       GPIO_HIGH

#define LCD_RST_MUX_NAME    GPIO4D1_SMCDATA9_TRACEDATA9_NAME
#define LCD_RST_PIN         RK30_PIN4_PD1
#define LCD_RST_VALUE       GPIO_HIGH

#define LCD_EN_MUX_NAME    GPIO4C7_SMCDATA7_TRACEDATA7_NAME
#define LCD_EN_PIN         RK30_PIN6_PB4
#define LCD_EN_VALUE       GPIO_LOW

#define HDMI11_MUX_NAME                GPIO3A6_SDMMC0RSTNOUT_NAME
#define HDMI11_EN_PIN          RK30_PIN3_PA6
#define HDMI11_EN_VALUE        GPIO_HIGH


static int rk_fb_io_init(struct rk29_fb_setting_info *fb_setting)
{
	int ret = 0;

	rk30_mux_api_set(GPIO3A6_SDMMC0RSTNOUT_NAME, GPIO3A_GPIO3A6);
	ret = gpio_request(HDMI11_EN_PIN, NULL);
	if (ret != 0)
	{
		gpio_free(HDMI11_EN_PIN);
		printk(KERN_ERR "hdmi gpio fail!\n");
		return -1;
	}
	else
	{
		gpio_direction_output(HDMI11_EN_PIN, HDMI11_EN_VALUE);
	}

	rk30_mux_api_set(LCD_CS_MUX_NAME, GPIO4C_GPIO4C7);
	ret = gpio_request(LCD_CS_PIN, NULL);
	if (ret != 0)
	{
		gpio_free(LCD_CS_PIN);
		printk(KERN_ERR "request lcd cs pin fail!\n");
		return -1;
	}
	else
	{
		gpio_direction_output(LCD_CS_PIN, LCD_CS_VALUE);
	}
	ret = gpio_request(LCD_EN_PIN, NULL);
	if (ret != 0)
	{
		gpio_free(LCD_EN_PIN);
		printk(KERN_ERR "request lcd en pin fail!\n");
		return -1;
	}
	else
	{
		gpio_direction_output(LCD_EN_PIN, LCD_EN_VALUE);
	}

	rk30_mux_api_set(LCD_STANDBY_MUX_NAME, GPIO4D_GPIO4D2);
	ret = gpio_request(LCD_STANDBY_PIN, NULL);
	if (ret != 0)
	{
		gpio_free(LCD_STANDBY_PIN);
		printk(KERN_ERR "request lcd cs pin fail!\n");
		return -1;
	}
	else
	{
		gpio_direction_output(LCD_STANDBY_PIN, LCD_STANDBY_VALUE);
	}

	rk30_mux_api_set(LCD_RST_MUX_NAME, GPIO4D_GPIO4D1);
	ret = gpio_request(LCD_RST_PIN, NULL);
	if (ret != 0)
	{
		gpio_free(LCD_RST_PIN);
		printk(KERN_ERR "request lcd cs pin fail!\n");
		return -1;
	}
	else
	{
		gpio_direction_output(LCD_RST_PIN, LCD_RST_VALUE);
	}
	
	return 0;
}
static int rk_fb_io_disable(void)
{
	msleep(30);		//Response Time (Rising + Falling)
	gpio_set_value(HDMI11_EN_PIN, HDMI11_EN_VALUE? 0:1);
	gpio_set_value(LCD_CS_PIN, LCD_CS_VALUE? 0:1);
	gpio_set_value(LCD_EN_PIN, LCD_EN_VALUE? 0:1);	
	gpio_set_value(LCD_RST_PIN, LCD_RST_VALUE? 0:1);
	gpio_set_value(LCD_STANDBY_PIN, LCD_STANDBY_VALUE? 0:1);	
	return 0;
}
static int rk_fb_io_enable(void)
{
	gpio_set_value(LCD_EN_PIN, LCD_EN_VALUE);	
	gpio_set_value(LCD_CS_PIN, LCD_CS_VALUE);
	gpio_set_value(HDMI11_EN_PIN, HDMI11_EN_VALUE);
	gpio_set_value(LCD_RST_PIN, LCD_RST_VALUE);		
	gpio_set_value(LCD_STANDBY_PIN, LCD_STANDBY_VALUE);	
	msleep(150);	//wait for power stable
	return 0;
}

#if defined(CONFIG_LCDC0_RK30)
struct rk29fb_info lcdc0_screen_info = {
	.prop	   = PRMRY,		//primary display device
	.io_init   = rk_fb_io_init,
	.io_disable = rk_fb_io_disable,
	.io_enable = rk_fb_io_enable,
	.set_screen_info = set_lcd_info,
};
#endif

#if defined(CONFIG_LCDC1_RK30)
struct rk29fb_info lcdc1_screen_info = {
	#if defined(CONFIG_HDMI_RK30)
	.prop		= EXTEND,	//extend display device
	.lcd_info  = NULL,
	.set_screen_info = hdmi_init_lcdc,
	#endif
};
#endif

static struct resource resource_fb[] = {
	[0] = {
		.name  = "fb0 buf",
		.start = 0,
		.end   = 0,//RK30_FB0_MEM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "ipp buf",  //for rotate
		.start = 0,
		.end   = 0,//RK30_FB0_MEM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[2] = {
		.name  = "fb2 buf",
		.start = 0,
		.end   = 0,//RK30_FB0_MEM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device device_fb = {
	.name		= "rk-fb",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resource_fb),
	.resource	= resource_fb,
};
#endif

#if defined(CONFIG_LCDC0_RK30)
static struct resource resource_lcdc0[] = {
	[0] = {
		.name  = "lcdc0 reg",
		.start = RK30_LCDC0_PHYS,
		.end   = RK30_LCDC0_PHYS + RK30_LCDC0_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	
	[1] = {
		.name  = "lcdc0 irq",
		.start = IRQ_LCDC0,
		.end   = IRQ_LCDC0,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device device_lcdc0 = {
	.name		  = "rk30-lcdc",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(resource_lcdc0),
	.resource	  = resource_lcdc0,
	.dev 		= {
		.platform_data = &lcdc0_screen_info,
	},
};
#endif
#if defined(CONFIG_LCDC1_RK30) 
extern struct rk29fb_info lcdc1_screen_info;
static struct resource resource_lcdc1[] = {
	[0] = {
		.name  = "lcdc1 reg",
		.start = RK30_LCDC1_PHYS,
		.end   = RK30_LCDC1_PHYS + RK30_LCDC1_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "lcdc1 irq",
		.start = IRQ_LCDC1,
		.end   = IRQ_LCDC1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device device_lcdc1 = {
	.name		  = "rk30-lcdc",
	.id		  = 1,
	.num_resources	  = ARRAY_SIZE(resource_lcdc1),
	.resource	  = resource_lcdc1,
	.dev 		= {
		.platform_data = &lcdc1_screen_info,
	},
};
#endif

#ifdef CONFIG_ANDROID_TIMED_GPIO
static struct timed_gpio timed_gpios[] = {
	{
		.name = "vibrator",
		.gpio = RK30_PIN0_PA4,
		.max_timeout = 1000,
		.active_low = 0,
		.adjust_time =20,      //adjust for diff product
	},
};

static struct timed_gpio_platform_data rk29_vibrator_info = {
	.num_gpios = 1,
	.gpios = timed_gpios,
};

static struct platform_device rk29_device_vibrator = {
	.name = "timed-gpio",
	.id = -1,
	.dev = {
		.platform_data = &rk29_vibrator_info,
	},

};
#endif

#ifdef CONFIG_LEDS_GPIO_PLATFORM
static struct gpio_led rk29_leds[] = {
	{
		.name = "button-backlight",
		.gpio = RK30_PIN4_PD7,
		.default_trigger = "timer",
		.active_low = 0,
		.retain_state_suspended = 0,
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
	},
};

static struct gpio_led_platform_data rk29_leds_pdata = {
	.leds = rk29_leds,
	.num_leds = ARRAY_SIZE(rk29_leds),
};

static struct platform_device rk29_device_gpio_leds = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data  = &rk29_leds_pdata,
	},
};
#endif

#ifdef CONFIG_RK_IRDA
#define IRDA_IRQ_PIN           RK30_PIN6_PA1

static int irda_iomux_init(void)
{
	int ret = 0;

	//irda irq pin
	ret = gpio_request(IRDA_IRQ_PIN, NULL);
	if (ret != 0) {
		gpio_free(IRDA_IRQ_PIN);
		printk(">>>>>> IRDA_IRQ_PIN gpio_request err \n ");
	}
	gpio_pull_updown(IRDA_IRQ_PIN, PullDisable);
	gpio_direction_input(IRDA_IRQ_PIN);

	return 0;
}

static int irda_iomux_deinit(void)
{
	gpio_free(IRDA_IRQ_PIN);
	return 0;
}

static struct irda_info rk29_irda_info = {
	.intr_pin = IRDA_IRQ_PIN,
	.iomux_init = irda_iomux_init,
	.iomux_deinit = irda_iomux_deinit,
	//.irda_pwr_ctl = bu92747guw_power_ctl,
};

static struct platform_device irda_device = {
#ifdef CONFIG_RK_IRDA_NET
	.name = "rk_irda",
#else
	.name = "bu92747_irda",
#endif
	.id = -1,
	.dev = {
		.platform_data = &rk29_irda_info,
	}
};
#endif

#ifdef CONFIG_ION
#define ION_RESERVE_SIZE        (80 * SZ_1M)
static struct ion_platform_data rk30_ion_pdata = {
	.nr = 1,
	.heaps = {
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = ION_NOR_HEAP_ID,
			.name = "norheap",
			.size = ION_RESERVE_SIZE,
		}
	},
};

static struct platform_device device_ion = {
	.name = "ion-rockchip",
	.id = 0,
	.dev = {
		.platform_data = &rk30_ion_pdata,
	},
};
#endif

/**************************************************************************************************
 * SDMMC devices,  include the module of SD,MMC,and sdio.noted by xbw at 2012-03-05
**************************************************************************************************/
#ifdef CONFIG_SDMMC_RK29
#include "board-rk30-sdk-sdmmc.c"

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
#define SDMMC0_WRITE_PROTECT_PIN	RK30_PIN3_PB7	//According to your own project to set the value of write-protect-pin.
#endif

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
#define SDMMC1_WRITE_PROTECT_PIN	RK30_PIN3_PC7	//According to your own project to set the value of write-protect-pin.
#endif

#define RK29SDK_WIFI_SDIO_CARD_DETECT_N    RK30_PIN6_PB2
#define RK29SDK_WIFI_SDIO_CARD_INT         RK30_PIN3_PD2

#define RK29SDK_SD_CARD_DETECT_N        RK30_PIN3_PB6  //According to your own project to set the value of card-detect-pin.
#define RK29SDK_SD_CARD_INSERT_LEVEL    GPIO_LOW         // set the voltage of insert-card. Please pay attention to the default setting.
#endif //endif ---#ifdef CONFIG_SDMMC_RK29

#ifdef CONFIG_SDMMC0_RK29
static int rk29_sdmmc0_cfg_gpio(void)
{
#ifdef CONFIG_SDMMC_RK29_OLD
	rk30_mux_api_set(GPIO3B1_SDMMC0CMD_NAME, GPIO3B_SDMMC0_CMD);
	rk30_mux_api_set(GPIO3B0_SDMMC0CLKOUT_NAME, GPIO3B_SDMMC0_CLKOUT);
	rk30_mux_api_set(GPIO3B2_SDMMC0DATA0_NAME, GPIO3B_SDMMC0_DATA0);
	rk30_mux_api_set(GPIO3B3_SDMMC0DATA1_NAME, GPIO3B_SDMMC0_DATA1);
	rk30_mux_api_set(GPIO3B4_SDMMC0DATA2_NAME, GPIO3B_SDMMC0_DATA2);
	rk30_mux_api_set(GPIO3B5_SDMMC0DATA3_NAME, GPIO3B_SDMMC0_DATA3);

	rk30_mux_api_set(GPIO3B6_SDMMC0DETECTN_NAME, GPIO3B_GPIO3B6);

	rk30_mux_api_set(GPIO3A7_SDMMC0PWREN_NAME, GPIO3A_GPIO3A7);
	gpio_request(RK30_PIN3_PA7, "sdmmc-power");
	gpio_direction_output(RK30_PIN3_PA7, GPIO_LOW);

#else
	rk29_sdmmc_set_iomux(0, 0xFFFF);

#if defined(CONFIG_SDMMC0_RK29_SDCARD_DET_FROM_GPIO)
     rk30_mux_api_set(GPIO3B6_SDMMC0DETECTN_NAME, GPIO3B_GPIO3B6);
#else
	rk30_mux_api_set(GPIO3B6_SDMMC0DETECTN_NAME, GPIO3B_SDMMC0_DETECT_N);
#endif	

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
	gpio_request(SDMMC0_WRITE_PROTECT_PIN, "sdmmc-wp");
	gpio_direction_input(SDMMC0_WRITE_PROTECT_PIN);
#endif

#endif

	return 0;
}

#define CONFIG_SDMMC0_USE_DMA
struct rk29_sdmmc_platform_data default_sdmmc0_data = {
	.host_ocr_avail =
	    (MMC_VDD_25_26 | MMC_VDD_26_27 | MMC_VDD_27_28 | MMC_VDD_28_29 |
	     MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 |
	     MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36),
	.host_caps =
	    (MMC_CAP_4_BIT_DATA | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
	.io_init = rk29_sdmmc0_cfg_gpio,

#if !defined(CONFIG_SDMMC_RK29_OLD)
	.set_iomux = rk29_sdmmc_set_iomux,
#endif

	.dma_name = "sd_mmc",
#ifdef CONFIG_SDMMC0_USE_DMA
	.use_dma = 1,
#else
	.use_dma = 0,
#endif

#if defined(CONFIG_WIFI_COMBO_MODULE_CONTROL_FUNC)
    .status = rk29sdk_wifi_mmc0_status,
    .register_status_notify = rk29sdk_wifi_mmc0_status_register,
#endif
#if defined(CONFIG_SDMMC0_RK29_SDCARD_DET_FROM_GPIO)
    .detect_irq = RK29SDK_SD_CARD_DETECT_N,
    .insert_card_level = RK29SDK_SD_CARD_INSERT_LEVEL,
#else
	.detect_irq = RK30_PIN3_PB6,	// INVALID_GPIO
#endif
	.enable_sd_wakeup = 0,

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
	.write_prt = SDMMC0_WRITE_PROTECT_PIN,
#else
	.write_prt = INVALID_GPIO,
#endif
};
#endif // CONFIG_SDMMC0_RK29

#ifdef CONFIG_SDMMC1_RK29
#define CONFIG_SDMMC1_USE_DMA
static int rk29_sdmmc1_cfg_gpio(void)
{
#if defined(CONFIG_SDMMC_RK29_OLD)
	rk30_mux_api_set(GPIO3C0_SMMC1CMD_NAME, GPIO3C_SMMC1_CMD);
	rk30_mux_api_set(GPIO3C5_SDMMC1CLKOUT_NAME, GPIO3C_SDMMC1_CLKOUT);
	rk30_mux_api_set(GPIO3C1_SDMMC1DATA0_NAME, GPIO3C_SDMMC1_DATA0);
	rk30_mux_api_set(GPIO3C2_SDMMC1DATA1_NAME, GPIO3C_SDMMC1_DATA1);
	rk30_mux_api_set(GPIO3C3_SDMMC1DATA2_NAME, GPIO3C_SDMMC1_DATA2);
	rk30_mux_api_set(GPIO3C4_SDMMC1DATA3_NAME, GPIO3C_SDMMC1_DATA3);
	//rk30_mux_api_set(GPIO3C6_SDMMC1DETECTN_NAME, GPIO3C_SDMMC1_DETECT_N);

#else

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
	gpio_request(SDMMC1_WRITE_PROTECT_PIN, "sdio-wp");
	gpio_direction_input(SDMMC1_WRITE_PROTECT_PIN);
#endif

#endif

	return 0;
}

struct rk29_sdmmc_platform_data default_sdmmc1_data = {
	.host_ocr_avail =
	    (MMC_VDD_25_26 | MMC_VDD_26_27 | MMC_VDD_27_28 | MMC_VDD_28_29 |
	     MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 |
	     MMC_VDD_33_34),

#if !defined(CONFIG_USE_SDMMC1_FOR_WIFI_DEVELOP_BOARD)
	.host_caps = (MMC_CAP_4_BIT_DATA | MMC_CAP_SDIO_IRQ |
		      MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
#else
	.host_caps =
	    (MMC_CAP_4_BIT_DATA | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
#endif

	.io_init = rk29_sdmmc1_cfg_gpio,

#if !defined(CONFIG_SDMMC_RK29_OLD)
	.set_iomux = rk29_sdmmc_set_iomux,
#endif

	.dma_name = "sdio",
#ifdef CONFIG_SDMMC1_USE_DMA
	.use_dma = 1,
#else
	.use_dma = 0,
#endif

#if !defined(CONFIG_USE_SDMMC1_FOR_WIFI_DEVELOP_BOARD)
#if defined(CONFIG_WIFI_CONTROL_FUNC) || defined(CONFIG_WIFI_COMBO_MODULE_CONTROL_FUNC)
	.status = rk29sdk_wifi_status,
	.register_status_notify = rk29sdk_wifi_status_register,
#endif
#if defined(CONFIG_SDMMC0_RK29_SDCARD_DET_FROM_GPIO)	
	.detect_irq = INVALID_GPIO,//RK29SDK_WIFI_SDIO_CARD_DETECT_N,
#else
	//.detect_irq = RK29SDK_WIFI_SDIO_CARD_DETECT_N, 	
#endif

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
	.write_prt = SDMMC1_WRITE_PROTECT_PIN,
#else
	.write_prt = INVALID_GPIO,
#endif

#if defined(CONFIG_RK29_SDIO_IRQ_FROM_GPIO)
    .sdio_INT_gpio = RK29SDK_WIFI_SDIO_CARD_INT,
#endif

#else
	.detect_irq = INVALID_GPIO,
	.enable_sd_wakeup = 0,
#endif

};
#endif //endif--#ifdef CONFIG_SDMMC1_RK29

/**************************************************************************************************
 * the end of setting for SDMMC devices
**************************************************************************************************/

#if (defined(CONFIG_BATTERY_RK30_ADC)||defined(CONFIG_BATTERY_RK30_ADC_FAC))
static int batt_table[2*11+6] =
{
	0x4B434F52,0x7461625F,0x79726574,0,200,200,
	3617,3630,3650,3670,3709,3749,3783,3828,3881,3930,4089, 
	3692,3906,3944,3978,4028,4091,4110,4125,4132,4140,4141 
};

void charge_current_set(int on)
{
	int ret = 0, value = 0;
	int charge_current_pin = RK30_PIN0_PC6;

	ret = gpio_request(charge_current_pin, NULL);
	if (ret) {
		printk("failed to request charge_current_pin gpio%d\n", charge_current_pin);
		return;
	}
	value = gpio_get_value(charge_current_pin);
	if(value != on){
		gpio_direction_output(charge_current_pin, on);
	//	printk("charge_current_set %s\n", on ? "2000mA" : "500mA");
	}
	gpio_free(charge_current_pin);
}

static struct rk30_adc_battery_platform_data rk30_adc_battery_platdata = {
        .dc_det_pin      = RK30_PIN6_PA5,
        .batt_low_pin    = RK30_PIN6_PA0,
        .charge_set_pin  = INVALID_GPIO,
        .charge_ok_pin   = RK30_PIN6_PA6,
//	.usb_det_pin     = RK30_PIN6_PA3, 

        .dc_det_level    = GPIO_LOW,
        .charge_ok_level = GPIO_HIGH,
//	.usb_det_level = GPIO_LOW,
        .save_capacity	 = 1,
        .spport_usb_charging = 1,
        .is_reboot_charging = 1,
        .use_board_table = 1,
        .board_batt_table = batt_table,
        .control_usb_charging = charge_current_set,
};

static struct platform_device rk30_device_adc_battery = {
        .name   = "rk30-battery",
        .id     = -1,
        .dev = {
                .platform_data = &rk30_adc_battery_platdata,
        },
};
#endif
#ifdef CONFIG_RK30_PWM_REGULATOR
const static int pwm_voltage_map[] = {
	950000,975000,1000000, 1025000, 1050000, 1075000, 1100000, 1125000, 1150000, 1175000, 1200000, 1225000, 1250000, 1275000, 1300000, 1325000, 1350000, 1375000, 1400000
};

static struct regulator_consumer_supply pwm_dcdc1_consumers[] = {
	{
		.supply = "vdd_core",
	}
};

struct regulator_init_data pwm_regulator_init_dcdc[1] =
{
	{
		.constraints = {
			.name = "PWM_DCDC1",
			.min_uV = 600000,
			.max_uV = 1800000,	//0.6-1.8V
			.apply_uV = true,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
		},
		.num_consumer_supplies = ARRAY_SIZE(pwm_dcdc1_consumers),
		.consumer_supplies = pwm_dcdc1_consumers,
	},
};

static struct pwm_platform_data pwm_regulator_info[1] = {
	{
		.pwm_id = 3,
		.pwm_gpio = RK30_PIN0_PD7,
		.pwm_iomux_name = GPIO0D7_PWM3_NAME,
		.pwm_iomux_pwm = GPIO0D_PWM3,
		.pwm_iomux_gpio = GPIO0D_GPIO0D6,
		.pwm_voltage = 1100000,
		.suspend_voltage = 1050000,
		.min_uV = 950000,
		.max_uV	= 1400000,
		.coefficient = 455,	//45.5%
		.pwm_voltage_map = pwm_voltage_map,
		.init_data	= &pwm_regulator_init_dcdc[0],
	},
};

struct platform_device pwm_regulator_device[1] = {
	{
		.name = "pwm-voltage-regulator",
		.id = 0,
		.dev		= {
			.platform_data = &pwm_regulator_info[0],
		}
	},
};
#endif

#ifdef CONFIG_RK29_VMAC
#define PHY_PWR_EN_GPIO	RK30_PIN1_PD6
#include "board-rk30-sdk-vmac.c"
#endif

#ifdef CONFIG_RFKILL_RK
// bluetooth rfkill device, its driver in net/rfkill/rfkill-rk.c
static struct rfkill_rk_platform_data rfkill_rk_platdata = {
    .type               = RFKILL_TYPE_BLUETOOTH,

    .poweron_gpio       = { // BT_REG_ON
        .io             = RK30_PIN3_PC7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = GPIO3C7_SDMMC1WRITEPRT_NAME,
            .fgpio      = GPIO3C_GPIO3C7,
        },
    },

    .reset_gpio         = { // BT_RST
        .io             = RK30_PIN3_PD1, // set io to INVALID_GPIO for disable it
        .enable         = GPIO_LOW,
        .iomux          = {
            .name       = GPIO3D1_SDMMC1BACKENDPWR_NAME,
            .fgpio      = GPIO3D_GPIO3D1,
        },
    },

    .wake_gpio          = { // BT_WAKE, use to control bt's sleep and wakeup
        .io             = RK30_PIN3_PC6, // set io to INVALID_GPIO for disable it
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = GPIO3C6_SDMMC1DETECTN_NAME,
            .fgpio      = GPIO3C_GPIO3C6,
        },
    },

    .wake_host_irq      = { // BT_HOST_WAKE, for bt wakeup host when it is in deep sleep
        .gpio           = {
            .io         = RK30_PIN6_PA7, // set io to INVALID_GPIO for disable it
            .enable     = GPIO_LOW,      // set GPIO_LOW for falling, set 0 for rising
            .iomux      = {
                .name   = NULL,
            },
        },
    },

    .rts_gpio           = { // UART_RTS, enable or disable BT's data coming
        .io             = RK30_PIN1_PA3, // set io to INVALID_GPIO for disable it
        .enable         = GPIO_LOW,
        .iomux          = {
            .name       = GPIO1A3_UART0RTSN_NAME,
            .fgpio      = GPIO1A_GPIO1A3,
            .fmux       = GPIO1A_UART0_RTS_N,
        },
    },
};

static struct platform_device device_rfkill_rk = {
    .name   = "rfkill_rk",
    .id     = -1,
    .dev    = {
        .platform_data = &rfkill_rk_platdata,
    },
};
#endif

#ifdef CONFIG_RK_BOARD_ID
static int board_id_init_platform_hw(void)
{
	rk30_mux_api_set(GPIO0D5_I2S22CHSDO_SMCADDR1_NAME, GPIO0D_GPIO0D5);
	rk30_mux_api_set(GPIO0D4_I2S22CHSDI_SMCADDR0_NAME, GPIO0D_GPIO0D4);
	rk30_mux_api_set(GPIO0D3_I2S22CHLRCKTX_SMCADVN_NAME, GPIO0D_GPIO0D3);
	rk30_mux_api_set(GPIO0D2_I2S22CHLRCKRX_SMCOEN_NAME, GPIO0D_GPIO0D2);

	return 0;
}

static int board_id_exit_platform_hw(void)
{

	return 0;
}

//setting someting according to board id
static int init_paramter_according_id(int id)
{
	if(id < 0)
		return -1;
	
#if defined(CONFIG_MACH_RK30_PHONE_PAD_C8003)

	signed char orientation[4][9] = {
		{0, -1, 0, 0, 0, 1, 1, 0, 0},
		{1, 0, 0, 0, 0, -1, 0, -1, 0},
    {0, 1, 0, 0, 0, -1, 1, 0, 0},
		{0, -1, 0, 0, 0, -1, -1, 0, 0},
	};

	u32 bl_ref[4] = {1, 0, 0, 0};
	int bp_id[4]={BP_ID_MT6229,BP_ID_MU509,BP_ID_MT6229,BP_ID_MT6229};
	
#if defined (CONFIG_GS_MMA7660)	
	memcpy(mma7660_info.orientation, orientation[id], 9);	
#endif
#ifdef  LCD_DISP_ON_PIN
	rk29_bl_info.bl_ref = bl_ref[id];
#endif
#if defined(CONFIG_BP_AUTO)
	bp_auto_info.bp_id=bp_id[id];
#endif
	if(id == BOARD_ID_C8003)
	{
		//enable vccio_wl 
		gpio_request(RK30_PIN0_PA6, "codec_en");
		rk30_mux_api_set(GPIO0A6_HOSTDRVVBUS_NAME, GPIO0A_GPIO0A6);
		gpio_direction_output(RK30_PIN0_PA6, GPIO_HIGH);
	}
#else


#endif
	return 0;
}


static struct board_id_platform_data rk30_board_id = {
	.gpio_pin = {RK30_PIN0_PD2,RK30_PIN0_PD3,RK30_PIN0_PD4,RK30_PIN0_PD5},
	.num_gpio = 4,
	.init_platform_hw = board_id_init_platform_hw,
	.exit_platform_hw = board_id_exit_platform_hw,
	.init_parameter	= init_paramter_according_id,
};


static struct platform_device device_board_id = {
    .name   = "rk-board-id",
    .id     = -1,
    .dev    = {
        .platform_data = &rk30_board_id,
    },
};


#endif

static struct platform_device *devices[] __initdata = {
#ifdef CONFIG_FB_ROCKCHIP
	&device_fb,
#endif
#if defined(CONFIG_LCDC0_RK30)
	&device_lcdc0,
#endif
#if defined(CONFIG_LCDC1_RK30)
	&device_lcdc1,
#endif

#ifdef CONFIG_BACKLIGHT_RK29_BL
	&rk29_device_backlight,
#endif

#ifdef CONFIG_ION
	&device_ion,
#endif
#ifdef CONFIG_ANDROID_TIMED_GPIO
	&rk29_device_vibrator,
#endif
#ifdef CONFIG_LEDS_GPIO_PLATFORM
	&rk29_device_gpio_leds,
#endif
#ifdef CONFIG_RK_IRDA
	&irda_device,
#endif
#if defined(CONFIG_WIFI_CONTROL_FUNC)||defined(CONFIG_WIFI_COMBO_MODULE_CONTROL_FUNC)
	&rk29sdk_wifi_device,
#endif

#if defined(CONFIG_MT6620)
    &mt3326_device_gps,
#endif   

#ifdef CONFIG_RK29_SUPPORT_MODEM
	&rk30_device_modem,
#endif
#if defined(CONFIG_MU509)
	&rk29_device_mu509,
#endif
#if defined(CONFIG_MW100)
	&rk29_device_mw100,
#endif
#if defined(CONFIG_MT6229)
	&rk29_device_mt6229,
#endif
#if defined(CONFIG_SEW868)
	&rk30_device_sew868,
#endif
#if defined (CONFIG_RK_HEADSET_DET) ||  defined (CONFIG_RK_HEADSET_IRQ_HOOK_ADC_DET)
	&rk_device_headset,
#endif
#if defined(CONFIG_MI700)
        &rk29_device_mi700,
#endif
#if defined(CONFIG_BP_AUTO)
	&device_bp_auto,
#endif

#if (defined(CONFIG_BATTERY_RK30_ADC)||defined(CONFIG_BATTERY_RK30_ADC_FAC))
 	&rk30_device_adc_battery,
#endif
#ifdef CONFIG_RFKILL_RK
	&device_rfkill_rk,
#endif
#ifdef CONFIG_RK_BOARD_ID
	&device_board_id,
#endif

};

// i2c
#ifdef CONFIG_I2C0_RK30
static struct i2c_board_info __initdata i2c0_info[] = {
#if defined (CONFIG_GS_MMA8452)
	{
		.type	        = "gs_mma8452",
		.addr	        = 0x1d,
		.flags	        = 0,
		.irq	        = MMA8452_INT_PIN,
		.platform_data = &mma8452_info,
	},
#endif
#if defined (CONFIG_GS_LIS3DH)
	{
		.type	        = "gs_lis3dh",
		.addr	        = 0x19,   //0x19(SA0-->VCC), 0x18(SA0-->GND)
		.flags	        = 0,
		.irq	        = LIS3DH_INT_PIN,
		.platform_data = &lis3dh_info,
	},
#endif

#if defined (CONFIG_GS_KXTIK)
	{
		.type	        = "gs_kxtik",
		.addr	        = 0x0F,
		.flags	        = 0,
		.irq	        = KXTIK_INT_PIN,
		.platform_data = &kxtik_info,
	},
#endif

#if defined (CONFIG_GS_MMA7660)
	{
		.type		= "gs_mma7660",
		.addr		= 0x4c,
		.flags		= 0,
		.irq		= MMA7660_INT_PIN,
		.platform_data = &mma7660_info,
	},
#endif

#if defined (CONFIG_COMPASS_AK8975)
	{
		.type          = "ak8975",
		.addr          = 0x0d,
		.flags         = 0,
		.irq           = RK30_PIN4_PC1,
		.platform_data = &akm8975_info,
	},
#endif
#if defined (CONFIG_GYRO_L3G4200D)
	{
		.type          = "l3g4200d_gryo",
		.addr          = 0x69,
		.flags         = 0,
		.irq           = L3G4200D_INT_PIN,
		.platform_data = &l3g4200d_info,
	},
#endif
#if defined (CONFIG_INPUT_LPSENSOR_AL3006)
	{
		.type           = "al3006",
		.addr           = 0x1c,             //sel = 0; if sel =1, then addr = 0x1D
		.flags          = 0,
		.irq            = RK30_PIN6_PA2,
	},
#endif

#if defined (CONFIG_LS_AL3006)
	{
		.type           = "light_al3006",
		.addr           = 0x1c,             //sel = 0; if sel =1, then addr = 0x1D
		.flags          = 0,
		.irq            = RK30_PIN6_PA2,	
		.platform_data = &light_al3006_info,
	},
#endif
#if defined (CONFIG_LS_STK3171)
	{
		.type           = "ls_stk3171",
		.addr           = 0x48,            
		.flags          = 0,
		.irq            = RK30_PIN6_PA2,	
		.platform_data = &light_stk3171_info,
	},
#endif


#if defined (CONFIG_PS_AL3006)
	{
		.type           = "proximity_al3006",
		.addr           = 0x1c,             //sel = 0; if sel =1, then addr = 0x1D
		.flags          = 0,
		.irq            = RK30_PIN6_PA2,	
		.platform_data = &proximity_al3006_info,
	},
#endif

#if defined (CONFIG_PS_STK3171)
	{
		.type           = "ps_stk3171",
		.addr           = 0x48,            
		.flags          = 0,
		.irq            = RK30_PIN6_PA2,	
		.platform_data = &proximity_stk3171_info,
	},
#endif


#if defined (CONFIG_SND_SOC_RK1000)
	{
		.type          = "rk1000_i2c_codec",
		.addr          = 0x60,
		.flags         = 0,
	},
	{
		.type          = "rk1000_control",
		.addr          = 0x40,
		.flags         = 0,
	},
#endif

#if defined (CONFIG_SND_SOC_RT5623)
        {
                .type                   = "rt5623",
                .addr                   = 0x1a,
                .flags                  = 0,
        },
#endif
#if defined (CONFIG_SND_SOC_RT5631)
        {
                .type                   = "rt5631",
                .addr                   = 0x1a,
                .flags                  = 0,
        },
#endif
#if defined (CONFIG_SND_SOC_RT3224) || defined (CONFIG_SND_SOC_RT3261)
        {
                .type                   = "rt3261",
                .addr                   = 0x1c,
                .flags                  = 0,
				.platform_data          = &rt3261_info,
        },
#endif

#ifdef CONFIG_MFD_RK610
		{
			.type			= "rk610_ctl",
			.addr			= 0x40,
			.flags			= 0,
		},
#ifdef CONFIG_RK610_TVOUT
		{
			.type			= "rk610_tvout",
			.addr			= 0x42,
			.flags			= 0,
		},
#endif
#ifdef CONFIG_HDMI_RK610
		{
			.type			= "rk610_hdmi",
			.addr			= 0x46,
			.flags			= 0,
			.irq			= RK29_PIN5_PA2,
		},
#endif
#ifdef CONFIG_SND_SOC_RK610
		{//RK610_CODEC addr  from 0x60 to 0x80 (0x60~0x80)
			.type			= "rk610_i2c_codec",
			.addr			= 0x60,
			.flags			= 0,
		},
#endif
#endif

};
#endif

int __sramdata g_pmic_type =  0;
#ifdef CONFIG_I2C1_RK30
#ifdef CONFIG_MFD_WM831X_I2C
#include "board-rk30-sdk-wm8326.c"
#endif
#ifdef CONFIG_MFD_TPS65910
#define TPS65910_HOST_IRQ        RK30_PIN6_PA4
#include "board-rk30-sdk-tps65910.c"
#endif

static struct i2c_board_info __initdata i2c1_info[] = {
#if defined (CONFIG_MFD_WM831X_I2C)
	{
		.type          = "wm8326",
		.addr          = 0x34,
		.flags         = 0,
		.irq           = RK30_PIN6_PA4,
		.platform_data = &wm831x_platdata,
	},
#endif
#if defined (CONFIG_MFD_TPS65910)
	{
        .type           = "tps65910",
        .addr           = TPS65910_I2C_ID0,
        .flags          = 0,
        .irq            = TPS65910_HOST_IRQ,
    	.platform_data = &tps65910_data,
	},
#endif
};
#endif


void __sramfunc board_pmu_suspend(void)
{      
	#if defined (CONFIG_MFD_WM831X_I2C)
       if(pmic_is_wm8326())
       board_pmu_wm8326_suspend();
	#endif
	#if defined (CONFIG_MFD_TPS65910)
       if(pmic_is_tps65910())
       board_pmu_tps65910_suspend(); 
    #endif   
}

void __sramfunc board_pmu_resume(void)
{      
	#if defined (CONFIG_MFD_WM831X_I2C)
       if(pmic_is_wm8326())
       board_pmu_wm8326_resume();
	#endif
	#if defined (CONFIG_MFD_TPS65910)
       if(pmic_is_tps65910())
       board_pmu_tps65910_resume(); 
	#endif
}

 int __sramdata gpio0d7_iomux,gpio0d7_do,gpio0d7_dir,gpio0d7_en;

void __sramfunc rk30_pwm_logic_suspend_voltage(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR

//	int gpio0d7_iomux,gpio0d7_do,gpio0d7_dir,gpio0d7_en;
	sram_udelay(10000);
	gpio0d7_iomux = readl_relaxed(GRF_GPIO0D_IOMUX);
	gpio0d7_do = grf_readl(GRF_GPIO0H_DO);
	gpio0d7_dir = grf_readl(GRF_GPIO0H_DIR);
	gpio0d7_en = grf_readl(GRF_GPIO0H_EN);

	writel_relaxed((1<<30), GRF_GPIO0D_IOMUX);
	grf_writel((1<<31)|(1<<15), GRF_GPIO0H_DIR);
	grf_writel((1<<31)|(1<<15), GRF_GPIO0H_DO);
	grf_writel((1<<31)|(1<<15), GRF_GPIO0H_EN);
#endif 
}
void __sramfunc rk30_pwm_logic_resume_voltage(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR
	writel_relaxed((1<<30)|gpio0d7_iomux, GRF_GPIO0D_IOMUX);
	grf_writel((1<<31)|gpio0d7_en, GRF_GPIO0H_EN);
	grf_writel((1<<31)|gpio0d7_dir, GRF_GPIO0H_DIR);
	grf_writel((1<<31)|gpio0d7_do, GRF_GPIO0H_DO);
	sram_udelay(10000);

#endif

}
extern void pwm_suspend_voltage(void);
extern void pwm_resume_voltage(void);
void  rk30_pwm_suspend_voltage_set(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR
	pwm_suspend_voltage();
#endif
}
void  rk30_pwm_resume_voltage_set(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR
	pwm_resume_voltage();
#endif
}

u32 gpio1a_iomux,gpio2c_iomux, gpio1b_pull,gpio2d_pull, gpio1b_dir,gpio2d_dir,gpio1b_en, gpio2d_en;
void board_gpio_suspend(void) {
	
	gpio1a_iomux = readl_relaxed(GRF_GPIO1A_IOMUX);
	gpio2c_iomux = readl_relaxed(GRF_GPIO2C_IOMUX);
	writel_relaxed((0xf<< 26), GRF_GPIO1A_IOMUX);
	writel_relaxed((0x3 <<18), GRF_GPIO2C_IOMUX);

	gpio1b_pull =  grf_readl(GRF_GPIO1L_PULL);
	gpio2d_pull =  grf_readl(GRF_GPIO2H_PULL);
	grf_writel(gpio1b_pull |(0x3<<21)|(0x3<<5),GRF_GPIO1L_PULL);	
	grf_writel( gpio2d_pull | (0x1<<17) |(0x1<<1),GRF_GPIO2H_PULL);
	
	gpio1b_dir =  grf_readl(GRF_GPIO1L_DIR);
	gpio2d_dir =  grf_readl(GRF_GPIO2H_DIR);
	grf_writel(gpio1b_dir |(0x1<<21),GRF_GPIO1L_DIR);	
	grf_writel(gpio2d_dir | (0x1<<17) ,GRF_GPIO2H_DIR);

	gpio1b_en =  grf_readl(GRF_GPIO1L_EN);
	gpio2d_en =  grf_readl(GRF_GPIO2H_EN);
	grf_writel( gpio1b_en |(0x3<<21)|(0x3<<5),GRF_GPIO1L_EN);	
	grf_writel( gpio2d_en | (0x1<<17) |(0x1<<1),GRF_GPIO2H_EN);
	
}
 void board_gpio_resume(void) {

	writel_relaxed(0xffff0000|gpio1a_iomux, GRF_GPIO1A_IOMUX);
	writel_relaxed(0xffff0000|gpio2c_iomux, GRF_GPIO2C_IOMUX);
	
	grf_writel( 0xffff0000|gpio1b_pull,GRF_GPIO1L_PULL);
	grf_writel( 0xffff0000|gpio2d_pull,GRF_GPIO2H_PULL);

	grf_writel( 0xffff0000|gpio1b_dir,GRF_GPIO1L_DIR);
	grf_writel( 0xffff0000|gpio2d_dir,GRF_GPIO2H_DIR);

	grf_writel( 0xffff0000|gpio1b_en,GRF_GPIO1L_EN);
	grf_writel( 0xffff0000|gpio2d_en,GRF_GPIO2H_EN);
	
}

#ifdef CONFIG_I2C2_RK30
static struct i2c_board_info __initdata i2c2_info[] = {
#if defined (CONFIG_TOUCHSCREEN_GT8XX)
	{
		.type          = "Goodix-TS",
		.addr          = 0x55,
		.flags         = 0,
		.irq           = RK30_PIN4_PC2,
		.platform_data = &goodix_info,
	},
#endif
#if defined(CONFIG_TOUCHSCREEN_GT82X_IIC)
	{
		.type          = "Goodix-TS-82X",
		.addr          = 0x5D,
		.flags         = 0,
		.irq           = RK30_PIN4_PC2,
		.platform_data = &ts82x_pdata,
	},
#endif
#if defined (CONFIG_LS_CM3217)
	{
		.type          = "light_cm3217",
		.addr          = 0x10,
		.flags         = 0,
		.platform_data = &cm3217_info,
	},
#endif


#if defined (CONFIG_TOUCHSCREEN_SYNAPTICS_S3202)
{
	.type           = "rmi_i2c",
	.addr           = 0x20,
	.flags          = 0,
	.irq            = RK30_PIN4_PC2,
	.platform_data = &s3202_platformdata,
},
#endif

#if defined (CONFIG_TOUCHSCREEN_FT5306)
{
	.type           = "ft5x0x_ts",
	.addr           = 0x3e,
	.flags          = 0,
	.irq            = RK30_PIN4_PC2,
	.platform_data = &ft5306_info,
},
#endif

#if defined (CONFIG_TOUCHSCREEN_CT360_IIC)
	{
	.type		="ct360_ts",
	.addr		=0x01,
	.flags		=0,
	.irq		= RK30_PIN4_PC2,
	.platform_data = &ct360_info,
	},
#endif

#if  defined(CONFIG_TOUCHSCREEN_GT8110)
	{
		.type          = "Goodix-TS",
		.addr          = 0x5c,
		.flags         = 0,
		.irq           = TOUCH_INT_PIN,
		.platform_data = &goodix_info,
	},
#endif

#if defined (CONFIG_TS_AUTO_I2C)
	{
		.type          = "auto_ts_i2c",
		.addr          = 0x01,
		.flags         = 0,
		.irq           = TOUCH_INT_PIN,
		.platform_data = &auto_ts_info,
	},
#endif


#if defined (CONFIG_LS_ISL29023)
	{
		.type           = "ls_isl29023",
		.addr           = 0x44,            
		.flags          = 0,
		.irq            = RK30_PIN4_PC6,	
		.platform_data = &light_isl29023_info,
	},
#endif

};
#endif

#ifdef CONFIG_I2C3_RK30
static struct i2c_board_info __initdata i2c3_info[] = {
};
#endif

#ifdef CONFIG_I2C4_RK30
static struct i2c_board_info __initdata i2c4_info[] = {
};
#endif

#ifdef CONFIG_I2C_GPIO_RK30
#define I2C_SDA_PIN     INVALID_GPIO// RK30_PIN2_PD6   //set sda_pin here
#define I2C_SCL_PIN     INVALID_GPIO//RK30_PIN2_PD7   //set scl_pin here
static int rk30_i2c_io_init(void)
{
        //set iomux (gpio) here
        //rk30_mux_api_set(GPIO2D7_I2C1SCL_NAME, GPIO2D_GPIO2D7);
        //rk30_mux_api_set(GPIO2D6_I2C1SDA_NAME, GPIO2D_GPIO2D6);

        return 0;
}
struct i2c_gpio_platform_data default_i2c_gpio_data = {
       .sda_pin = I2C_SDA_PIN,
       .scl_pin = I2C_SCL_PIN,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(100),
       .bus_num    = 5,
       .io_init = rk30_i2c_io_init,
};
static struct i2c_board_info __initdata i2c_gpio_info[] = {
};
#endif

static void __init rk30_i2c_register_board_info(void)
{
#ifdef CONFIG_I2C0_RK30
	i2c_register_board_info(0, i2c0_info, ARRAY_SIZE(i2c0_info));
#endif
#ifdef CONFIG_I2C1_RK30
	i2c_register_board_info(1, i2c1_info, ARRAY_SIZE(i2c1_info));
#endif
#ifdef CONFIG_I2C2_RK30
	i2c_register_board_info(2, i2c2_info, ARRAY_SIZE(i2c2_info));
#endif
#ifdef CONFIG_I2C3_RK30
	i2c_register_board_info(3, i2c3_info, ARRAY_SIZE(i2c3_info));
#endif
#ifdef CONFIG_I2C4_RK30
	i2c_register_board_info(4, i2c4_info, ARRAY_SIZE(i2c4_info));
#endif
#ifdef CONFIG_I2C_GPIO_RK30
	i2c_register_board_info(5, i2c_gpio_info, ARRAY_SIZE(i2c_gpio_info));
#endif
}
//end of i2c

static void dcr_en_low(void)
{
  int ret;
  ret=gpio_request(RK30_PIN4_PB7,"dcr_en");
  if(ret<0){
     printk("dcr_en_low request io error");
     gpio_free(RK30_PIN4_PB7);
     return;
  }
  gpio_direction_output(RK30_PIN4_PB7, GPIO_LOW);
}

#define POWER_ON_PIN RK30_PIN6_PB0   //power_hold
void rk30_pm_power_off(void)
{
  int pwr_cnt;
  
	printk(KERN_ERR "rk30_pm_power_off start...\n");
	
#if 0

	if(gpio_get_value (rk30_adc_battery_platdata.dc_det_pin) == rk30_adc_battery_platdata.dc_det_level){
		pwr_cnt=0;
		while(1){
			 if(gpio_get_value (rk30_adc_battery_platdata.dc_det_pin) != rk30_adc_battery_platdata.dc_det_level){
			 	  break;
			 }
			 if((gpio_get_value(RK30_PIN6_PA2)==GPIO_LOW)){
			 	 if(pwr_cnt++>40)
			 	   break;
			 }else{
			 	 //printk("0\n");
			 	 pwr_cnt=0;
			 }		
			 mdelay(50);
		}	
              arm_pm_restart(0, NULL);
	}
#endif
	#if defined(CONFIG_MFD_WM831X)	
	if(pmic_is_wm8326())
	{
		wm831x_set_bits(Wm831x,WM831X_GPIO_LEVEL,0x0001,0x0000);  //set sys_pwr 0
		wm831x_device_shutdown(Wm831x);//wm8326 shutdown
	}
	#endif
	#if defined(CONFIG_MFD_TPS65910)
	if(pmic_is_tps65910())
	{
		tps65910_device_shutdown();//tps65910 shutdown
	}
	#endif
  gpio_direction_output(POWER_ON_PIN, GPIO_LOW);
	while(1);
}
EXPORT_SYMBOL_GPL(rk30_pm_power_off);
static void __init machine_rk30_board_init(void)
{
	avs_init();
	gpio_request(POWER_ON_PIN, "poweronpin");
	gpio_direction_output(POWER_ON_PIN, GPIO_HIGH);

	//add for codec_en 
	gpio_request(RK30_PIN4_PD7, "codec_en");
	rk30_mux_api_set(GPIO4D7_SMCDATA15_TRACEDATA15_NAME, GPIO4D_GPIO4D7);
	gpio_direction_output(RK30_PIN4_PD7, GPIO_HIGH);
	
	pm_power_off = rk30_pm_power_off;
	
	rk30_i2c_register_board_info();
	spi_register_board_info(board_spi_devices, ARRAY_SIZE(board_spi_devices));
	platform_add_devices(devices, ARRAY_SIZE(devices));
	board_usb_detect_init(RK30_PIN6_PA3);

#ifdef CONFIG_WIFI_CONTROL_FUNC
	rk29sdk_wifi_bt_gpio_control_init();
#endif
  dcr_en_low();

#if defined(CONFIG_MT6620)
    clk_set_rate(clk_get_sys("rk_serial.0", "uart"), 48*1000000);
#endif
}

static void __init rk30_reserve(void)
{
#ifdef CONFIG_ION
	rk30_ion_pdata.heaps[0].base = board_mem_reserve_add("ion", ION_RESERVE_SIZE);
#endif
#ifdef CONFIG_FB_ROCKCHIP
	resource_fb[0].start = board_mem_reserve_add("fb0", RK30_FB0_MEM_SIZE);
	resource_fb[0].end = resource_fb[0].start + RK30_FB0_MEM_SIZE - 1;
	#if 0
	resource_fb[1].start = board_mem_reserve_add("ipp buf", RK30_FB0_MEM_SIZE);
	resource_fb[1].end = resource_fb[1].start + RK30_FB0_MEM_SIZE - 1;
	resource_fb[2].start = board_mem_reserve_add("fb2", RK30_FB0_MEM_SIZE);
	resource_fb[2].end = resource_fb[2].start + RK30_FB0_MEM_SIZE - 1;
	#endif
#endif
#ifdef CONFIG_VIDEO_RK29
	rk30_camera_request_reserve_mem();
#endif
	board_mem_reserved();
}

/**
 * dvfs_cpu_logic_table: table for arm and logic dvfs 
 * @frequency	: arm frequency
 * @cpu_volt	: arm voltage depend on frequency
 * @logic_volt	: logic voltage arm requests depend on frequency
 * comments	: min arm/logic voltage
 */
static struct dvfs_arm_table dvfs_cpu_logic_table[] = {
	{.frequency = 252 * 1000,	.cpu_volt = 1075 * 1000,	.logic_volt = 1125 * 1000},//0.975V/1.000V
	{.frequency = 504 * 1000,	.cpu_volt = 1100 * 1000,	.logic_volt = 1125 * 1000},//0.975V/1.000V
	{.frequency = 816 * 1000,	.cpu_volt = 1125 * 1000,	.logic_volt = 1150 * 1000},//1.000V/1.025V
	{.frequency = 1008 * 1000,	.cpu_volt = 1125 * 1000,	.logic_volt = 1150 * 1000},//1.025V/1.050V
	{.frequency = 1200 * 1000,	.cpu_volt = 1175 * 1000,	.logic_volt = 1200 * 1000},//1.100V/1.050V
	{.frequency = 1272 * 1000,	.cpu_volt = 1225 * 1000,	.logic_volt = 1200 * 1000},//1.150V/1.100V
	{.frequency = 1416 * 1000,	.cpu_volt = 1300 * 1000,	.logic_volt = 1200 * 1000},//1.225V/1.100V
	{.frequency = 1512 * 1000,	.cpu_volt = 1350 * 1000,	.logic_volt = 1250 * 1000},//1.300V/1.150V
	{.frequency = 1608 * 1000,	.cpu_volt = 1425 * 1000,	.logic_volt = 1300 * 1000},//1.325V/1.175V
	{.frequency = CPUFREQ_TABLE_END},
};

static struct cpufreq_frequency_table dvfs_gpu_table[] = {
	{.frequency = 266 * 1000,	.index = 1050 * 1000},
	{.frequency = 400 * 1000,	.index = 1275 * 1000},
	{.frequency = CPUFREQ_TABLE_END},
};

static struct cpufreq_frequency_table dvfs_ddr_table[] = {
	{.frequency = 300 * 1000,	.index = 1050 * 1000},
	{.frequency = 400 * 1000,	.index = 1125 * 1000},
	{.frequency = CPUFREQ_TABLE_END},
};

#define DVFS_CPU_TABLE_SIZE	(ARRAY_SIZE(dvfs_cpu_logic_table))
static struct cpufreq_frequency_table cpu_dvfs_table[DVFS_CPU_TABLE_SIZE];
static struct cpufreq_frequency_table dep_cpu2core_table[DVFS_CPU_TABLE_SIZE];

void __init board_clock_init(void)
{
	//support uart 1-3Mbits/s 
	rk30_clock_data_init(periph_pll_default, codec_pll_768mhz, RK30_CLOCKS_DEFAULT_FLAGS | CLK_FLG_UART_1_3M);
	dvfs_set_arm_logic_volt(dvfs_cpu_logic_table, cpu_dvfs_table, dep_cpu2core_table);
	dvfs_set_freq_volt_table(clk_get(NULL, "gpu"), dvfs_gpu_table);
	dvfs_set_freq_volt_table(clk_get(NULL, "ddr"), dvfs_ddr_table);
}

MACHINE_START(RK30, "RK30board")
	.boot_params	= PLAT_PHYS_OFFSET + 0x800,
	.fixup		= rk30_fixup,
	.reserve	= &rk30_reserve,
	.map_io		= rk30_map_io,
	.init_irq	= rk30_init_irq,
	.timer		= &rk30_timer,
	.init_machine	= machine_rk30_board_init,
MACHINE_END
