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

#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_S3202
#include <linux/interrupt.h>
#include <linux/rmi.h>
#endif
#include <linux/sensor-dev.h>
#include <linux/regulator/machine.h>
#include <linux/rfkill-rk.h>

#if defined(CONFIG_HDMI_RK30)
	#include "../../../drivers/video/rockchip/hdmi/rk_hdmi.h"
#endif
#include "../../../drivers/headset_observe/rk_headset.h"
#include <linux/mfd/tlv320aic3262-core.h>
#include <linux/mfd/tlv320aic3262-registers.h>

#if defined(CONFIG_SPIM_RK29)
#include "../../../drivers/spi/rk29_spim.h"
#endif
#if defined(CONFIG_ANDROID_TIMED_GPIO)
#include "../../../drivers/staging/android/timed_gpio.h"
#endif
#include <linux/mtk23d.h>
#include "../../../drivers/tty/serial/sc8800.h"
#define BP_VOL_PIN			RK30_PIN6_PB2
#define RK30_FB0_MEM_SIZE 8*SZ_1M

#ifdef CONFIG_VIDEO_RK29
/*---------------- Camera Sensor Macro Define Begin  ------------------------*/
/*---------------- Camera Sensor Configuration Macro Begin ------------------------*/

#define CONFIG_SENSOR_0 RK29_CAM_SENSOR_OV5640						/* back camera sensor */
#define CONFIG_SENSOR_IIC_ADDR_0		  0x78
#define CONFIG_SENSOR_IIC_ADAPTER_ID_0	  3
#define CONFIG_SENSOR_CIF_INDEX_0         0
#define CONFIG_SENSOR_ORIENTATION_0 	  90
#define CONFIG_SENSOR_POWER_PIN_0		  RK30_PIN4_PC6
#define CONFIG_SENSOR_RESET_PIN_0		  INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_0 	  RK30_PIN1_PB6
#define CONFIG_SENSOR_FALSH_PIN_0		  RK30_PIN4_PD6
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_0 RK29_CAM_POWERACTIVE_H
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_0 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_0 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_0 RK29_CAM_FLASHACTIVE_H

#define CONFIG_SENSOR_QCIF_FPS_FIXED_0		15000
#define CONFIG_SENSOR_240X160_FPS_FIXED_0   15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_0		15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_0		15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_0		15000
#define CONFIG_SENSOR_480P_FPS_FIXED_0		15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_0		15000
#define CONFIG_SENSOR_720P_FPS_FIXED_0		30000

#define CONFIG_SENSOR_01  RK29_CAM_SENSOR_OV5642                   /* back camera sensor 1 */
#define CONFIG_SENSOR_IIC_ADDR_01 	    0x00
#define CONFIG_SENSOR_CIF_INDEX_01                    1
#define CONFIG_SENSOR_IIC_ADAPTER_ID_01    4
#define CONFIG_SENSOR_ORIENTATION_01       90
#define CONFIG_SENSOR_POWER_PIN_01         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_01         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_01       RK30_PIN1_PD6
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
#define CONFIG_SENSOR_IIC_ADDR_02 	    0x00
#define CONFIG_SENSOR_CIF_INDEX_02                    1
#define CONFIG_SENSOR_IIC_ADAPTER_ID_02    4
#define CONFIG_SENSOR_ORIENTATION_02       90
#define CONFIG_SENSOR_POWER_PIN_02         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_02         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_02       RK30_PIN1_PD6
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

#define CONFIG_SENSOR_1 RK29_CAM_SENSOR_GT2005                      /* front camera sensor 0 */
#define CONFIG_SENSOR_IIC_ADDR_1 	      0x78
#define CONFIG_SENSOR_IIC_ADAPTER_ID_1	  3
#define CONFIG_SENSOR_CIF_INDEX_1		  0
#define CONFIG_SENSOR_ORIENTATION_1       270
#define CONFIG_SENSOR_POWER_PIN_1         RK30_PIN4_PC6
#define CONFIG_SENSOR_RESET_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_1 	  RK30_PIN1_PB7
#define CONFIG_SENSOR_FALSH_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_1 RK29_CAM_POWERACTIVE_H
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_1 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_1 RK29_CAM_POWERDNACTIVE_L
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_1 RK29_CAM_FLASHACTIVE_L

#define CONFIG_SENSOR_QCIF_FPS_FIXED_1		15000
#define CONFIG_SENSOR_240X160_FPS_FIXED_1   15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_1		15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_1		15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_1		15000
#define CONFIG_SENSOR_480P_FPS_FIXED_1		15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_1		15000
#define CONFIG_SENSOR_720P_FPS_FIXED_1		30000

#define CONFIG_SENSOR_11 RK29_CAM_SENSOR_OV2659                      /* front camera sensor 1 */
#define CONFIG_SENSOR_IIC_ADDR_11 	    0x00
#define CONFIG_SENSOR_IIC_ADAPTER_ID_11    3
#define CONFIG_SENSOR_CIF_INDEX_11				  0
#define CONFIG_SENSOR_ORIENTATION_11       270
#define CONFIG_SENSOR_POWER_PIN_11         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_11         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_11       INVALID_GPIO//RK30_PIN1_PB7
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
#define CONFIG_SENSOR_IIC_ADDR_12 	   0x00
#define CONFIG_SENSOR_IIC_ADAPTER_ID_12    3
#define CONFIG_SENSOR_CIF_INDEX_12				  0
#define CONFIG_SENSOR_ORIENTATION_12       270
#define CONFIG_SENSOR_POWER_PIN_12         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_12         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_12       INVALID_GPIO//RK30_PIN1_PB7
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
#define CONFIG_SENSOR_POWER_IOCTL_USR	   0
#define CONFIG_SENSOR_RESET_IOCTL_USR	   0
#define CONFIG_SENSOR_POWERDOWN_IOCTL_USR	   0
#define CONFIG_SENSOR_FLASH_IOCTL_USR	   1

#if CONFIG_SENSOR_POWER_IOCTL_USR
static int sensor_power_usr_cb (struct rk29camera_gpio_res *res,int on)
{
	#error "CONFIG_SENSOR_POWER_IOCTL_USR is 1, sensor_power_usr_cb function must be writed!!";
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
//hhb@rock-chips.com
#if CONFIG_SENSOR_FLASH_IOCTL_USR
//sgm3140 LED driver
#define CONFIG_SENSOR_FALSH_EN_PIN_0		  RK30_PIN4_PD6    //high:enable
#define CONFIG_SENSOR_FALSH_MODE_PIN_0		  RK30_PIN0_PD6    //high:FLASH, low:torch
static int sensor_flash_usr_cb (struct rk29camera_gpio_res *res, int on)
{
	static int init_flag = 0;
	if(init_flag == 0) {
		gpio_request(CONFIG_SENSOR_FALSH_MODE_PIN_0, "camera_falsh_mode");
		rk30_mux_api_set(GPIO0D6_PWM2_NAME, GPIO0D_GPIO0D6);
		gpio_request(CONFIG_SENSOR_FALSH_EN_PIN_0, "camera_falsh_en");
		rk30_mux_api_set(GPIO4D6_SMCDATA14_TRACEDATA14_NAME, GPIO4D_GPIO4D6);
		init_flag = 1;
	}
	switch (on) {
		case Flash_Off: {
			gpio_direction_output(CONFIG_SENSOR_FALSH_EN_PIN_0, 0);
			gpio_direction_output(CONFIG_SENSOR_FALSH_MODE_PIN_0, 1);
			break;
		}

		case Flash_On: {
			gpio_direction_output(CONFIG_SENSOR_FALSH_EN_PIN_0, 1);
			gpio_direction_output(CONFIG_SENSOR_FALSH_MODE_PIN_0, 1);
			break;
		}

		case Flash_Torch: {
			gpio_direction_output(CONFIG_SENSOR_FALSH_EN_PIN_0, 1);
			gpio_direction_output(CONFIG_SENSOR_FALSH_MODE_PIN_0, 0);
			break;
		}

		default: {
			printk("%s..Flash command(%d) is invalidate \n",__FUNCTION__, on);
			gpio_direction_output(CONFIG_SENSOR_FALSH_EN_PIN_0, 0);
			break;
		}
	}
	return 0;
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

//hhb@rock-chips.com 2012-04-27

#if defined(CONFIG_TOUCHSCREEN_FT5306)

#define TOUCH_RESET_PIN	 RK30_PIN6_PB1
#define TOUCH_INT_PIN 	 RK30_PIN4_PD7
int ft5306_init_platform_hw(void)
{

	printk("ft5406_init_platform_hw\n");
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
    rk30_mux_api_set(GPIO4D7_SMCDATA15_TRACEDATA15_NAME, 0);

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

  .init_platform_hw= ft5306_init_platform_hw,
  .exit_platform_hw= ft5306_exit_platform_hw,
  .ft5x0x_platform_sleep  = ft5306_platform_sleep,
  .ft5x0x_platform_wakeup = ft5306_platform_wakeup,
};


#endif

#if defined (CONFIG_TOUCHSCREEN_SYNAPTICS_S3202)

#define TOUCH_RESET_PIN -1
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
		.nbuttons = 4,
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
		.flip_x = 1,
		.flip_y = 1,
		.clip_X_low = 0,
		.clip_Y_low = 0,
		.clip_X_high = 0,
		.clip_Y_high = 0,
	},
	.f1a_button_map = &s3202_buttons,
};

#endif


/***********************************************************
*	rk30  backlight
************************************************************/
#ifdef CONFIG_BACKLIGHT_RK29_BL
#define PWM_ID            0
#define PWM_MUX_NAME      GPIO0A3_PWM0_NAME
#define PWM_MUX_MODE      GPIO0A_PWM0
#define PWM_MUX_MODE_GPIO GPIO0A_GPIO0A3
#define PWM_GPIO 	  RK30_PIN0_PA3
#define PWM_EFFECT_VALUE  0

#define LCD_DISP_ON_PIN

#ifdef  LCD_DISP_ON_PIN
#define BL_EN_MUX_NAME    GPIO4D2_SMCDATA10_TRACEDATA10_NAME
#define BL_EN_MUX_MODE    GPIO4D_GPIO4D2
#define BL_EN_PIN         RK30_PIN4_PD2
#define BL_EN_VALUE       GPIO_HIGH
#endif
static int rk29_backlight_io_init(void)
{
	int ret = 0;
	rk30_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE);
#ifdef  LCD_DISP_ON_PIN
	rk30_mux_api_set(BL_EN_MUX_NAME, BL_EN_MUX_MODE);
	ret = gpio_request(BL_EN_PIN, NULL);
	if (ret != 0) {
		gpio_free(BL_EN_PIN);
	}

	gpio_direction_output(BL_EN_PIN, 0);
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
	gpio_direction_output(PWM_GPIO, GPIO_HIGH);
#ifdef  LCD_DISP_ON_PIN
	gpio_direction_output(BL_EN_PIN, 0);
	gpio_set_value(BL_EN_PIN, !BL_EN_VALUE);
#endif
	return ret;
}

static int rk29_backlight_pwm_resume(void)
{
	gpio_direction_output(PWM_GPIO, GPIO_LOW);
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
};

static struct platform_device rk29_device_backlight = {
	.name	= "rk29_backlight",
	.id 	= -1,
	.dev	= {
		.platform_data  = &rk29_bl_info,
	}
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
#if defined(CONFIG_RK29_SC8800)
// tdsc8800
static int tdsc8800_io_init(void)
{

	return 0;
}

static int tdsc8800_io_deinit(void)
{

	return 0;
}

struct rk2818_23d_data rk29_tdsc8800_info = {
	.io_init = tdsc8800_io_init,
       .io_deinit = tdsc8800_io_deinit,
	.bp_power = BP_VOL_PIN,
	.bp_power_active_low = 1,
};
struct platform_device rk29_device_tdsc8800 = {
        .name = "tdsc8800",
	.id = -1,
	.dev		= {
		.platform_data = &rk29_tdsc8800_info,
	}
    };
//sc8800
static int sc8800_io_init(void)
{
	rk30_mux_api_set(GPIO2B5_LCDC1DATA13_SMCADDR17_HSADCDATA8_NAME, GPIO2B_GPIO2B5);
	rk30_mux_api_set(GPIO2A2_LCDCDATA2_SMCADDR6_NAME, GPIO2A_GPIO2A2);

	rk30_mux_api_set(GPIO2C2_LCDC1DATA18_SMCBLSN1_HSADCDATA5_NAME, GPIO2C_GPIO2C2);//AP_ASK_BP_TEST1
	gpio_request(RK30_PIN2_PC2, NULL);
	gpio_direction_output(RK30_PIN2_PC2, 0);
	gpio_request(RK30_PIN6_PB0, NULL);//AP_ON/OFF_BP
	gpio_direction_output(RK30_PIN6_PB0, 0);


	rk30_mux_api_set(GPIO2C3_LCDC1DATA19_SPI1CLK_HSADCDATA0_NAME, GPIO2C_SPI1_CLK);//spi 1
	rk30_mux_api_set(GPIO2C4_LCDC1DATA20_SPI1CSN0_HSADCDATA1_NAME, GPIO2C_SPI1_CSN0);
	rk30_mux_api_set(GPIO2C5_LCDC1DATA21_SPI1TXD_HSADCDATA2_NAME, GPIO2C_SPI1_TXD);
	rk30_mux_api_set(GPIO2C6_LCDC1DATA22_SPI1RXD_HSADCDATA3_NAME, GPIO2C_SPI1_RXD);
	return 0;
}

static int sc8800_io_deinit(void)
{

	return 0;
}

static struct plat_sc8800 sc8800_plat_data = {
	.slav_rts_pin = RK30_PIN6_PA0,
	.slav_rdy_pin = RK30_PIN6_PA1,
	.master_rts_pin = RK30_PIN2_PB5,
	.master_rdy_pin = RK30_PIN2_PA2,
	//.poll_time = 100,
	.io_init = sc8800_io_init,
    .io_deinit = sc8800_io_deinit,
};
static struct rk29xx_spi_chip sc8800_spi_chip = {
	//.poll_mode = 1,
	.enable_dma = 1,
};

#endif

static struct spi_board_info board_spi_devices[] = {

#if defined(CONFIG_RK29_SC8800)
		{
			.modalias  = "sc8800",
			.bus_num = 1,
			.platform_data = &sc8800_plat_data,
			.max_speed_hz  = 13*1000*1000,
			.chip_select   = 0,		
			.controller_data = &sc8800_spi_chip,
		},
#endif

};

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

static struct gsensor_platform_data lis3dh_info = {
	.model = 8452,
	.swap_xy = 0,
	.swap_xyz = 1,
	.init_platform_hw = lis3dh_init_platform_hw,
	.orientation = {-1, 0, 0, 0, 0, 1, 0, -1, 0},
};
#endif


#if defined (CONFIG_RK_HEADSET_DET) || defined (CONFIG_RK_HEADSET_IRQ_HOOK_ADC_DET)

static int rk_headset_io_init(int gpio)
{
	int ret;
	ret = gpio_request(gpio, "headset_input");
	if(ret) 
		return ret;

	rk30_mux_api_set(GPIO0D3_I2S22CHLRCKTX_SMCADVN_NAME, GPIO0D_GPIO0D3);
	gpio_pull_updown(gpio, PullDisable);
	gpio_direction_input(gpio);
	return 0;
};

struct rk_headset_pdata rk_headset_info = {
	.Headset_gpio		= RK30_PIN0_PD3,
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

#if defined (CONFIG_COMPASS_AK8975)
static struct sensor_platform_data akm8975_info =
{
	.type = SENSOR_TYPE_COMPASS,
	.irq_enable = 1,
	.poll_delay_ms = 30,
	.m_layout = 
	{
		{
			{0, 0, 1},
			{1, 0, 0},
			{0, 1, 0},
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
	.orientation = {1, 0, 0, 0, 1, 0, 0, 0, 1},
	.init_platform_hw = l3g4200d_init_platform_hw,
	.x_min = 40,//x_min,y_min,z_min = (0-100) according to hardware
	.y_min = 40,
	.z_min = 20,
};

#endif

#ifdef CONFIG_LS_CM3217

#define CM3217_POWER_PIN 	INVALID_GPIO
#define CM3217_IRQ_PIN		INVALID_GPIO
static int cm3217_init_hw(void)
{
#if 0
	if (gpio_request(CM3217_POWER_PIN, NULL) != 0) {
		gpio_free(CM3217_POWER_PIN);
		printk("%s: request cm3217 power pin error\n", __func__);
		return -EIO;
	}
	gpio_pull_updown(CM3217_POWER_PIN, PullDisable);

	if (gpio_request(CM3217_IRQ_PIN, NULL) != 0) {
		gpio_free(CM3217_IRQ_PIN);
		printk("%s: request cm3217 int pin error\n", __func__);
		return -EIO;
	}
	gpio_pull_updown(CM3217_IRQ_PIN, PullDisable);
#endif
	return 0;
}

static void cm3217_exit_hw(void)
{
#if 0
	gpio_free(CM3217_POWER_PIN);
	gpio_free(CM3217_IRQ_PIN);
#endif
	return;
}

static struct cm3217_platform_data cm3217_info = {
	.irq_pin = CM3217_IRQ_PIN,
	.power_pin = CM3217_POWER_PIN,
	.init_platform_hw = cm3217_init_hw,
	.exit_platform_hw = cm3217_exit_hw,
};
#endif

#if defined(CONFIG_PS_AL3006)
static struct sensor_platform_data proximity_info = {
	.type = SENSOR_TYPE_PROXIMITY,
	.irq_enable = 1,
	.poll_delay_ms = 200,
};
#endif

#if defined(CONFIG_LS_AL3006)
static struct sensor_platform_data light_info = {
	.type = SENSOR_TYPE_LIGHT,
	.irq_enable = 1,
	.poll_delay_ms = 200,
};
#endif


#ifdef CONFIG_FB_ROCKCHIP

/*****************************************************************************************
 * lcd  devices
 * author: hhb@rock-chips.com
 *****************************************************************************************/

#define LCD_TXD_PIN		RK30_PIN1_PA7
#define LCD_CLK_PIN		RK30_PIN1_PA5
#define LCD_CS_PIN		RK30_PIN1_PA4
#define LCD_RST_PIN		RK30_PIN4_PC7

/*****************************************************************************************
* frame buffer  devices
* author: hhb@rock-chips.com
*****************************************************************************************/
#define FB_ID                       0
#define FB_DISPLAY_ON_PIN           INVALID_GPIO//RK29_PIN6_PD0
#define FB_LCD_STANDBY_PIN          INVALID_GPIO//RK29_PIN6_PD1
#define FB_LCD_CABC_EN_PIN          INVALID_GPIO//RK29_PIN6_PD2
#define FB_MCU_FMK_PIN              INVALID_GPIO

#define FB_DISPLAY_ON_VALUE         GPIO_HIGH
#define FB_LCD_STANDBY_VALUE        GPIO_HIGH


//#endif
static int rk29_lcd_io_init(void)
{
	int ret = 0;

	//printk("rk29_lcd_io_init\n");

	ret = gpio_request(LCD_TXD_PIN, NULL);
	if (ret != 0) {
		gpio_free(LCD_TXD_PIN);
		printk("%s: request LCD_TXD_PIN error\n", __func__);
		return -EIO;
	}

	ret = gpio_request(LCD_CLK_PIN, NULL);
	if (ret != 0) {
		gpio_free(LCD_CLK_PIN);
		printk("%s: request LCD_CLK_PIN error\n", __func__);
		return -EIO;
	}

	ret = gpio_request(LCD_CS_PIN, NULL);
	if (ret != 0) {
		gpio_free(LCD_CS_PIN);
		printk("%s: request LCD_CS_PIN error\n", __func__);
		return -EIO;
	}

	rk30_mux_api_set(GPIO1A7_UART1RTSN_SPI0TXD_NAME, GPIO1A_GPIO1A7);
	rk30_mux_api_set(GPIO1A5_UART1SOUT_SPI0CLK_NAME, GPIO1A_GPIO1A5);
	rk30_mux_api_set(GPIO1A4_UART1SIN_SPI0CSN0_NAME, GPIO1A_GPIO1A4);
	//rk30_mux_api_set(GPIO4C7_SMCDATA7_TRACEDATA7_NAME, GPIO4C_GPIO4C7);
	gpio_direction_output(LCD_CS_PIN, 1);
	gpio_direction_output(LCD_CLK_PIN, 1);
	gpio_direction_output(LCD_TXD_PIN, 1);
	return ret;
}

#if defined (CONFIG_RK29_WORKING_POWER_MANAGEMENT)
static int rk29_lcd_io_deinit(void)
{
	int ret = 0;

	gpio_direction_input(LCD_TXD_PIN);
	gpio_direction_input(LCD_CLK_PIN);
	gpio_direction_input(LCD_CS_PIN);

	gpio_free(LCD_CS_PIN);
	gpio_free(LCD_CLK_PIN);
	gpio_free(LCD_TXD_PIN);
	return ret;
}
#else
static int rk29_lcd_io_deinit(void)
{
	int ret = 0;
	//printk("rk29_lcd_io_deinit\n");

	gpio_direction_input(LCD_TXD_PIN);
	gpio_direction_input(LCD_CLK_PIN);
	gpio_direction_input(LCD_CS_PIN);

	gpio_free(LCD_CS_PIN);
	gpio_free(LCD_CLK_PIN);
	gpio_free(LCD_TXD_PIN);
	//gpio_free(LCD_RXD_PIN);

	//rk30_mux_api_set(GPIO0B7_I2S8CHSDO3_NAME, GPIO0B_GPIO0B7);
	//rk30_mux_api_set(GPIO0B6_I2S8CHSDO2_NAME, GPIO0B_GPIO0B6);
	//rk30_mux_api_set(GPIO0B5_I2S8CHSDO1_NAME, GPIO0B_GPIO0B5);

	return ret;
}
#endif


static struct rk29lcd_info rk29_lcd_info = {
    .txd_pin  = LCD_TXD_PIN,
    .clk_pin = LCD_CLK_PIN,
    .cs_pin = LCD_CS_PIN,
    .reset_pin = LCD_RST_PIN,
    .io_init   = rk29_lcd_io_init,
    .io_deinit = rk29_lcd_io_deinit,
};


#define LCD_EN_MUX_NAME    GPIO0D4_I2S22CHSDI_SMCADDR0_NAME
#define LCD_EN_PIN         RK30_PIN0_PD4
#define LCD_EN_VALUE       GPIO_HIGH

static int rk_fb_io_init(struct rk29_fb_setting_info *fb_setting)
{
	int ret = 0;
	rk30_mux_api_set(LCD_EN_MUX_NAME, GPIO0D_GPIO0D4);
	ret = gpio_request(LCD_EN_PIN, NULL);
	if (ret != 0)
	{
		gpio_free(LCD_EN_PIN);
		printk(KERN_ERR "request lcd en pin fail!\n");
		return -1;
	}
	else
	{
		gpio_direction_output(LCD_EN_PIN, 1);
		gpio_set_value(LCD_EN_PIN, LCD_EN_VALUE);
	}
	return 0;
}
static int rk_fb_io_disable(void)
{
	gpio_set_value(LCD_EN_PIN, LCD_EN_VALUE ? 0:1);
	return 0;
}
static int rk_fb_io_enable(void)
{
	gpio_set_value(LCD_EN_PIN, LCD_EN_VALUE);
	return 0;
}

#if defined(CONFIG_LCDC0_RK30)
struct rk29fb_info lcdc0_screen_info = {
	.fb_id   = FB_ID,
	.prop	   = PRMRY,		//primary display device
	.lcd_info = &rk29_lcd_info,
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
	.set_screen_info = hdmi_set_info,
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
#include "board-rk30-phone-sdmmc.c"

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
#define SDMMC0_WRITE_PROTECT_PIN	RK30_PIN3_PB7	//According to your own project to set the value of write-protect-pin.
#endif

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
#define SDMMC1_WRITE_PROTECT_PIN	RK30_PIN3_PC7	//According to your own project to set the value of write-protect-pin.
#endif

#define RK29SDK_WIFI_SDIO_CARD_DETECT_N    RK30_PIN6_PB2

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

	rk30_mux_api_set(GPIO3B6_SDMMC0DETECTN_NAME, GPIO3B_SDMMC0_DETECT_N);

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
	.detect_irq = RK30_PIN3_PB6,	// INVALID_GPIO
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
#ifdef CONFIG_WIFI_CONTROL_FUNC
	.status = rk29sdk_wifi_status,
	.register_status_notify = rk29sdk_wifi_status_register,
#endif
#if 0
	.detect_irq = RK29SDK_WIFI_SDIO_CARD_DETECT_N,
#endif

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
	.write_prt = SDMMC1_WRITE_PROTECT_PIN,
#else
	.write_prt = INVALID_GPIO,
#endif

#else
	.detect_irq = INVALID_GPIO,
	.enable_sd_wakeup = 0,
#endif

};
#endif //endif--#ifdef CONFIG_SDMMC1_RK29


#ifdef CONFIG_BATTERY_RK30_ADC
static struct rk30_adc_battery_platform_data rk30_adc_battery_platdata = {
        .dc_det_pin      = RK30_PIN6_PA5,
        .batt_low_pin    = RK30_PIN6_PA0,
        .charge_set_pin  = INVALID_GPIO,
        .charge_ok_pin   = RK30_PIN6_PA6,
        .dc_det_level    = GPIO_LOW,
        .charge_ok_level = GPIO_HIGH,
};

static struct platform_device rk30_device_adc_battery = {
        .name   = "rk30-battery",
        .id     = -1,
        .dev = {
                .platform_data = &rk30_adc_battery_platdata,
        },
};
#endif


#ifdef CONFIG_RFKILL_RK
// bluetooth rfkill device, its driver in net/rfkill/rfkill-rk.c
static struct rfkill_rk_platform_data rfkill_rk_platdata = {
    .type               = RFKILL_TYPE_BLUETOOTH,

    .poweron_gpio       = { // BT_REG_ON
        .io             = RK30_PIN4_PD5,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = GPIO4D5_SMCDATA13_TRACEDATA13_NAME,
            .fgpio      = GPIO4D_GPIO4D5,
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
            .io         = RK30_PIN3_PD2, // set io to INVALID_GPIO for disable it
            .enable     = GPIO_LOW,      // set GPIO_LOW for falling, set 0 for rising
            .iomux      = {
                .name   = GPIO3D2_SDMMC1INTN_NAME,
                .fgpio  = RK30_PIN3_PD2,
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

/**************************************************************************************************
 * the end of setting for SDMMC devices
**************************************************************************************************/

static struct platform_device *devices[] __initdata = {
#ifdef CONFIG_BACKLIGHT_RK29_BL
	&rk29_device_backlight,
#endif
#ifdef CONFIG_FB_ROCKCHIP
	&device_fb,
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
#ifdef CONFIG_WIFI_CONTROL_FUNC
	&rk29sdk_wifi_device,
#endif
#ifdef CONFIG_RFKILL_RK
	&device_rfkill_rk,
#endif
#ifdef CONFIG_RK29_SUPPORT_MODEM
    &rk30_device_modem,
#endif
#if defined(CONFIG_TDSC8800)
&rk29_device_tdsc8800,
#endif
#if defined (CONFIG_RK_HEADSET_DET) ||  defined (CONFIG_RK_HEADSET_IRQ_HOOK_ADC_DET)
    &rk_device_headset,
#endif
#ifdef CONFIG_BATTERY_RK30_ADC
 	&rk30_device_adc_battery,
#endif
};

static struct aic3262_gpio_setup aic3262_gpio[] = {
	{ // GPIO1
		.used 		= 0,
		.in 		= 0,
		.value 		= AIC3262_GPIO1_FUNC_INT1_OUTPUT ,
		
	},
	{// GPIO2
		.used 		= 0,
		.in 		= 0,
		.value 		= AIC3262_GPIO2_FUNC_CLOCK_OUTPUT,
	},
	{// GPI1
		.used 		= 1,
		.in   		= 1,
		.value 		= 0,
	},
	{// GPI2
		.used 		= 1,
		.in   		= 1,
		.value 		= AIC3262_GPO1_FUNC_DISABLED,
	},
	{// GPO1
		.used 		= 0,
		.in 		= 0,
		.value		= AIC3262_GPO1_FUNC_ADC_MOD_CLK_OUTPUT,
	},
};
static struct aic3262_pdata aic3262_codec_pdata = {
	.gpio 		= aic3262_gpio,
	.gpio_reset = RK30_PIN0_PB7,
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
		.type	        = "lis3dh",
		.addr	        = 0x19,
		.flags	        = 0,
		.irq	        = LIS3DH_INT_PIN,
		.platform_data = &lis3dh_info,
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
		.platform_data = &light_info,
	},
#endif

#if defined (CONFIG_PS_AL3006)
	{
		.type           = "proximity_al3006",
		.addr           = 0x1c,             //sel = 0; if sel =1, then addr = 0x1D
		.flags          = 0,
		.irq            = RK30_PIN6_PA2,	
		.platform_data = &proximity_info,
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

#if defined (CONFIG_SND_SOC_TLV320AIC326X)
	{
		.type    		= "tlv320aic3262",
		.addr           = 0x18,
		.flags			= 0,
		.platform_data = &aic3262_codec_pdata,
	},
#endif

#if defined (CONFIG_SND_SOC_RT5631)
        {
                .type                   = "rt5631",
                .addr                   = 0x1a,
                .flags                  = 0,
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

#ifdef CONFIG_I2C1_RK30
#define DC3_VCC_DDR_VOL 1800000   //for vcc_lpddr2_1v8
#define DC5_VCC_DDR_VOL 1200000	//for vcc_lpddr2_1v2
#include "board-rk30-phone-twl60xx.c"
static struct i2c_board_info __initdata i2c1_info[] = {

#if defined (CONFIG_TWL4030_CORE)
	{
		.type = "twl6032",
		.addr = 0x48,
		.flags = 0,
		.irq            = RK30_PIN6_PA4, 
		.platform_data = &tps80032_data,

	},

#endif
};
#endif

#ifdef CONFIG_I2C2_RK30
static struct i2c_board_info __initdata i2c2_info[] = {

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
	.addr           = 0x38,
	.flags          = 0,
	.irq            = RK30_PIN4_PD7,//RK30_PIN2_PC2,
	.platform_data = &ft5306_info,
},
#endif

#if defined (CONFIG_LS_CM3217)
	{
		.type          = "lightsensor",
		.addr          = 0x10,
		.flags         = 0,
		.irq           = CM3217_IRQ_PIN,
		.platform_data = &cm3217_info,
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

static void rk30_pm_power_off(void)
{
	printk(KERN_ERR "rk30_pm_power_off start...\n");
#if defined(CONFIG_TWL4030_CORE)
	twl6030_poweroff();
#endif
	while (1);
}

#if 0
/**********************************************************************************************
 *
 * The virtual keys for android "back", "home", "menu", "search", these four keys are touch key
 * on the touch screen panel. (added by hhb@rock-chips.com 2011.04.21)
 * attention please: kobj_attribute.attr.name virtualkeys.synaptics_rmi4_i2c should be the same as
 * the input device in the touch screen driver.
 ***********************************************************************************************/
static ssize_t rk_virtual_keys_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
#if (defined(CONFIG_TOUCHSCREEN_SYNAPTICS_S3202))
	 printk("rk_virtual_keys_show S3202\n");
    /* centerx;centery;width;height; */
	return sprintf(buf,
		__stringify(EV_KEY) ":" __stringify(KEY_BACK)	     ":100:980:100:40"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":200:980:100:40"
		":" __stringify(EV_KEY) ":" __stringify(KEY_HOMEPAGE)   ":300:980:100:40"
		":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":400:980:100:40"
		"\n");
#endif
	return 0;
}

static struct kobj_attribute rk_virtual_keys_attr = {
	.attr = {
#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_S3202)
		.name = "virtualkeys.synaptics_rmi4_i2c",
#else
		.name = "virtualkeys",
#endif
		.mode = S_IRUGO,
	},
	.show = rk_virtual_keys_show,
};

static struct attribute *rk_properties_attrs[] = {
	&rk_virtual_keys_attr.attr,
	NULL
};

static struct attribute_group rk_properties_attr_group = {
	.attrs = rk_properties_attrs,
};
static int rk_virtual_keys_init(void)
{
	int ret;
	struct kobject *properties_kobj;
	printk("rk_virtual_keys_init \n");
	properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (properties_kobj)
		ret = sysfs_create_group(properties_kobj,
				&rk_properties_attr_group);
	if (!properties_kobj || ret)
	{
		printk("failed to create board_properties for virtual key\n");
	}
	return ret;
}

/*************************end of virtual_keys************************/
#endif

void board_gpio_suspend(void) {
	gpio_request(RK30_PIN2_PC3, NULL);
	rk30_mux_api_set(GPIO2C3_LCDC1DATA19_SPI1CLK_HSADCDATA0_NAME,GPIO2C_GPIO2C3);
	gpio_direction_input(RK30_PIN2_PC3);
	gpio_pull_updown(RK30_PIN2_PC3, 0);
	
	gpio_request(RK30_PIN2_PC5, NULL);
	rk30_mux_api_set(GPIO2C5_LCDC1DATA21_SPI1TXD_HSADCDATA2_NAME,GPIO2C_GPIO2C5);
	gpio_direction_input(RK30_PIN2_PC5);
	gpio_pull_updown(RK30_PIN2_PC5, 0);
	
	gpio_request(RK30_PIN2_PC6, NULL);
	rk30_mux_api_set(GPIO2C6_LCDC1DATA22_SPI1RXD_HSADCDATA3_NAME,GPIO2C_GPIO2C6);
	gpio_direction_input(RK30_PIN2_PC6);
	gpio_pull_updown(RK30_PIN2_PC6, 0);
	
}
 void board_gpio_resume(void) {

	gpio_request(RK30_PIN2_PC3, NULL);
	gpio_pull_updown(RK30_PIN2_PC3, 1);
	rk30_mux_api_set(GPIO2C3_LCDC1DATA19_SPI1CLK_HSADCDATA0_NAME,GPIO2C_SPI1_CLK);
	
	gpio_request(RK30_PIN2_PC5, NULL);
	gpio_pull_updown(RK30_PIN2_PC5, 1);
	rk30_mux_api_set(GPIO2C5_LCDC1DATA21_SPI1TXD_HSADCDATA2_NAME,GPIO2C_SPI1_TXD);
	
	gpio_request(RK30_PIN2_PC6, NULL);
	gpio_pull_updown(RK30_PIN2_PC6, 1);
	rk30_mux_api_set(GPIO2C6_LCDC1DATA22_SPI1RXD_HSADCDATA3_NAME,GPIO2C_SPI1_RXD);

	gpio_free(RK30_PIN2_PC3);
	gpio_free(RK30_PIN2_PC5);
	gpio_free(RK30_PIN2_PC6);
}

static void __init machine_rk30_board_init(void)
{
	pm_power_off = rk30_pm_power_off;
	
	rk30_i2c_register_board_info();
	spi_register_board_info(board_spi_devices, ARRAY_SIZE(board_spi_devices));
	platform_add_devices(devices, ARRAY_SIZE(devices));
	board_usb_detect_init(RK30_PIN6_PA3);

#ifdef CONFIG_WIFI_CONTROL_FUNC
	rk29sdk_wifi_bt_gpio_control_init();
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
	resource_fb[1].start = board_mem_reserve_add("ipp buf", RK30_FB0_MEM_SIZE);
	resource_fb[1].end = resource_fb[1].start + RK30_FB0_MEM_SIZE - 1;
	resource_fb[2].start = board_mem_reserve_add("fb2", RK30_FB0_MEM_SIZE);
	resource_fb[2].end = resource_fb[2].start + RK30_FB0_MEM_SIZE - 1;
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
	//{.frequency = 252 * 1000,	.cpu_volt = 1050 * 1000,	.logic_volt = 1050 * 1000},//0.975V/1.000V
	//{.frequency = 504 * 1000,	.cpu_volt = 1050 * 1000,	.logic_volt = 1050 * 1000},//0.975V/1.000V
	{.frequency = 816 * 1000,	.cpu_volt = 1050 * 1000,	.logic_volt = 1050 * 1000},//1.000V/1.025V
	{.frequency = 1008 * 1000,	.cpu_volt = 1050 * 1000,	.logic_volt = 1050 * 1000},//1.025V/1.050V
	{.frequency = 1200 * 1000,	.cpu_volt = 1150 * 1000,	.logic_volt = 1100 * 1000},//1.100V/1.050V
	{.frequency = 1272 * 1000,	.cpu_volt = 1200 * 1000,	.logic_volt = 1150 * 1000},//1.150V/1.100V
	{.frequency = 1416 * 1000,	.cpu_volt = 1275 * 1000,	.logic_volt = 1150 * 1000},//1.225V/1.100V
	//{.frequency = 1512 * 1000,	.cpu_volt = 1325 * 1000,	.logic_volt = 1200 * 1000},//1.300V/1.150V
	//{.frequency = 1608 * 1000,	.cpu_volt = 1350 * 1000,	.logic_volt = 1200 * 1000},//1.325V/1.175V
	{.frequency = CPUFREQ_TABLE_END},
};

#define DVFS_CPU_TABLE_SIZE	(ARRAY_SIZE(dvfs_cpu_logic_table))
static struct cpufreq_frequency_table cpu_dvfs_table[DVFS_CPU_TABLE_SIZE];
static struct cpufreq_frequency_table dep_cpu2core_table[DVFS_CPU_TABLE_SIZE];

void __init board_clock_init(void)
{
	rk30_clock_data_init(periph_pll_default, codec_pll_default, RK30_CLOCKS_DEFAULT_FLAGS);
	dvfs_set_arm_logic_volt(dvfs_cpu_logic_table, cpu_dvfs_table, dep_cpu2core_table);
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
