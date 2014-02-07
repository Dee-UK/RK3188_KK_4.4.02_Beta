/* arch/arm/mach-rk2928/board-rk2928-fpga.c
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

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/hardware/gic.h>
#include <mach/dvfs.h>

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
#include <linux/regulator/act8931.h>
#include <linux/regulator/rk29-pwm-regulator.h>
#if defined(CONFIG_HDMI_RK30)
	#include "../../../drivers/video/rockchip/hdmi/rk_hdmi.h"
#endif

#if defined(CONFIG_SPIM_RK29)
#include "../../../drivers/spi/rk29_spim.h"
#endif

#include "board-rk2928-a720-camera.c" 
#include "board-rk2928-a720-key.c"

#ifdef  CONFIG_THREE_FB_BUFFER
#define RK30_FB0_MEM_SIZE 12*SZ_1M
#else
#define RK30_FB0_MEM_SIZE 8*SZ_1M
#endif

int __sramdata g_pmic_type =  0;

static struct spi_board_info board_spi_devices[] = {
};

/***********************************************************
*	rk30  backlight
************************************************************/
#ifdef CONFIG_BACKLIGHT_RK29_BL
#define PWM_ID            0
#define PWM_MUX_NAME      GPIO0D2_PWM_0_NAME
#define PWM_MUX_MODE      GPIO0D_PWM_0
#define PWM_MUX_MODE_GPIO GPIO0D_GPIO0D2
#define PWM_GPIO 	  RK2928_PIN0_PD2
#define PWM_EFFECT_VALUE  0

#if defined(CONFIG_MACH_RK2926_M713)
//#define LCD_DISP_ON_PIN
#else
#define LCD_DISP_ON_PIN
#endif

#ifdef  LCD_DISP_ON_PIN

#define BL_EN_PIN         RK2928_PIN1_PB0
#define BL_EN_VALUE       GPIO_HIGH
#endif
static int rk29_backlight_io_init(void)
{
	int ret = 0;
	rk30_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE);
#ifdef  LCD_DISP_ON_PIN
	// rk30_mux_api_set(BL_EN_MUX_NAME, BL_EN_MUX_MODE);

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
	gpio_set_value(BL_EN_PIN, !BL_EN_VALUE);
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
	#if defined(CONFIG_MFD_TPS65910)	
	if(pmic_is_tps65910())
	{
		gpio_direction_output(PWM_GPIO, GPIO_LOW);
	}
	#endif
	#if defined(CONFIG_REGULATOR_ACT8931)
	if(pmic_is_act8931())
	{
		gpio_direction_output(PWM_GPIO, GPIO_HIGH);
	}
	#endif
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
        .min_brightness = 80,
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

#ifdef CONFIG_FB_ROCKCHIP

#define LCD_MUX_NAME  GPIO0C2_UART0_RTSN_NAME
#define LCD_GPIO_MODE GPIO0C_GPIO0C2

#ifdef CONFIG_MACH_RK2926_M713
#define LCD_EN        RK2928_PIN1_PB3
#else
#define LCD_EN        RK2928_PIN0_PC2
#endif
#define LCD_EN_VALUE  GPIO_LOW

static int rk_fb_io_init(struct rk29_fb_setting_info *fb_setting)
{
	int ret = 0;

        rk30_mux_api_set(LCD_MUX_NAME, LCD_GPIO_MODE);

	ret = gpio_request(LCD_EN, NULL);
	if (ret != 0)
	{
		gpio_free(LCD_EN);
		printk(KERN_ERR "request lcd en pin fail!\n");
		return -1;
	}
	else
	{
		gpio_direction_output(LCD_EN, LCD_EN_VALUE); //disable
	}
	return 0;
}
static int rk_fb_io_disable(void)
{

	#if 0//defined(CONFIG_REGULATOR_ACT8931)
	if(pmic_is_act8931())
	{
		struct regulator *ldo;
		ldo = regulator_get(NULL, "act_ldo4");	 //vcc_lcd
		regulator_disable(ldo);
		regulator_put(ldo);
		udelay(100);
	}
	#endif
        gpio_set_value(LCD_EN, !LCD_EN_VALUE);
	return 0;
}
static int rk_fb_io_enable(void)
{
	#if 0//defined(CONFIG_REGULATOR_ACT8931)
	if(pmic_is_act8931())
	{
		struct regulator *ldo;
		ldo = regulator_get(NULL, "act_ldo4");	 //vcc_lcd
		regulator_enable(ldo);
		regulator_put(ldo);
		udelay(100);
		msleep(300);	// wait for powering on LED circuit
	}
	#endif

        gpio_set_value(LCD_EN, LCD_EN_VALUE);
	return 0;
}

#if defined(CONFIG_LCDC_RK2928)
struct rk29fb_info lcdc_screen_info = {
	.prop	   = PRMRY,		//primary display device
	.io_init   = rk_fb_io_init,
	.io_disable = rk_fb_io_disable,
	.io_enable = rk_fb_io_enable,
	.set_screen_info = set_lcd_info,
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

//LCDC
#ifdef CONFIG_LCDC_RK2928
static struct resource resource_lcdc[] = {
	[0] = {
		.name  = "lcdc reg",
		.start = RK2928_LCDC_PHYS,
		.end   = RK2928_LCDC_PHYS + RK2928_LCDC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	
	[1] = {
		.name  = "lcdc irq",
		.start = IRQ_LCDC,
		.end   = IRQ_LCDC,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device device_lcdc = {
	.name		  = "rk2928-lcdc",
	.id		  = 0,
	.num_resources	  = ARRAY_SIZE(resource_lcdc),
	.resource	  = resource_lcdc,
	.dev 		= {
		.platform_data = &lcdc_screen_info,
	},
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


#if defined(CONFIG_TOUCHSCREEN_SITRONIX_A720)

#define TOUCH_RESET_PIN	 RK2928_PIN1_PA3
#ifdef CONFIG_MACH_RK2926_M713
#define TOUCH_INT_PIN 	 RK2928_PIN1_PB0
#else
#define TOUCH_INT_PIN 	 RK2928_PIN1_PB3
#endif
int ft5306_init_platform_hw(void)
{

        //printk("ft5306_init_platform_hw\n");
        if(gpio_request(TOUCH_RESET_PIN,NULL) != 0){
                gpio_free(TOUCH_RESET_PIN);
                printk("ft5306_init_platform_hw gpio_request error\n");
                return -EIO;
        }

        if(gpio_request(TOUCH_INT_PIN,NULL) != 0){
                gpio_free(TOUCH_INT_PIN);
                printk("ift5306_init_platform_hw gpio_request error\n");
                return -EIO;
        }
        gpio_direction_output(TOUCH_RESET_PIN, GPIO_HIGH);
        mdelay(10);
        gpio_set_value(TOUCH_RESET_PIN,GPIO_LOW);
        mdelay(10);
        gpio_set_value(TOUCH_RESET_PIN,GPIO_HIGH);
        msleep(300);
        return 0;

}

struct ft5x0x_platform_data sitronix_info = {
        .model = 5007,
        .init_platform_hw= ft5306_init_platform_hw,
};

#endif


/*MMA7660 gsensor*/
#if defined (CONFIG_GS_MMA7660)
#ifdef CONFIG_MACH_RK2926_M713
#define MMA7660_INT_PIN   RK2928_PIN1_PB2
#else
#define MMA7660_INT_PIN   RK2928_PIN1_PB1
#endif

static int mma7660_init_platform_hw(void)
{
#ifdef CONFIG_MACH_RK2926_M713
	rk30_mux_api_set(GPIO1B2_SPI_RXD_UART1_SIN_NAME, GPIO1B_GPIO1B2);
#else
	rk30_mux_api_set(GPIO1B1_SPI_TXD_UART1_SOUT_NAME, GPIO1B_GPIO1B1);
#endif

	return 0;
}

static struct sensor_platform_data mma7660_info = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
        .init_platform_hw = mma7660_init_platform_hw,
        .orientation = {-1, 0, 0, 0, 0, -1, 0, 1, 0},
};
#endif


#if CONFIG_RK30_PWM_REGULATOR
static int pwm_voltage_map[] = {
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
#ifdef CONFIG_MACH_RK2926_M713
		.pwm_id = 1,
		.pwm_gpio = RK2928_PIN0_PD3,
		.pwm_iomux_name = GPIO0D3_PWM_1_NAME,
		.pwm_iomux_pwm = GPIO0D_PWM_1, 
		.pwm_iomux_gpio = GPIO0D_GPIO0D3,
#else
		.pwm_id = 2,
		.pwm_gpio = RK2928_PIN0_PD4,
		.pwm_iomux_name = GPIO0D4_PWM_2_NAME,
		.pwm_iomux_pwm = GPIO0D_PWM_2, 
		.pwm_iomux_gpio = GPIO0D_GPIO0D4,
#endif
		.pwm_voltage = 1200000,
		.suspend_voltage = 1050000,
		.min_uV = 950000,
		.max_uV	= 1400000,
		.coefficient = 504,	//50.4%
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

/***********************************************************
*	usb wifi
************************************************************/
#if defined(CONFIG_RTL8192CU) || defined(CONFIG_RTL8188EU) 

static void rkusb_wifi_power(int on) {
	struct regulator *ldo = NULL;
	
#if defined(CONFIG_MFD_TPS65910)	
	if(pmic_is_tps65910()) {
		ldo = regulator_get(NULL, "vmmc");  //vccio_wl
	}
#endif
#if defined(CONFIG_REGULATOR_ACT8931)
	if(pmic_is_act8931()) {
		ldo = regulator_get(NULL, "act_ldo4");  //vccio_wl
	}
#endif	
	
	if(on) {
		regulator_enable(ldo);
		printk("%s: vccio_wl enable\n", __func__);
	} else {
		printk("%s: vccio_wl disable\n", __func__);
		regulator_disable(ldo);
	}
	
	regulator_put(ldo);
	udelay(100);
}

#endif

/**************************************************************************************************
 * SDMMC devices,  include the module of SD,MMC,and sdio.noted by xbw at 2012-03-05
**************************************************************************************************/
#ifdef CONFIG_SDMMC_RK29
#include "board-rk2928-sdk-sdmmc.c"

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
#define SDMMC0_WRITE_PROTECT_PIN	RK2928_PIN1_PA7	//According to your own project to set the value of write-protect-pin.
#endif

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
#define SDMMC1_WRITE_PROTECT_PIN	RK2928_PIN0_PD5	//According to your own project to set the value of write-protect-pin.
#endif

#define RK29SDK_WIFI_SDIO_CARD_DETECT_N    RK2928_PIN0_PB2

#define RK29SDK_SD_CARD_DETECT_N        RK2928_PIN2_PA7  //According to your own project to set the value of card-detect-pin.
#define RK29SDK_SD_CARD_INSERT_LEVEL    GPIO_LOW         // set the voltage of insert-card. Please pay attention to the default setting.


#endif //endif ---#ifdef CONFIG_SDMMC_RK29

#ifdef CONFIG_SDMMC0_RK29
static int rk29_sdmmc0_cfg_gpio(void)
{
	rk29_sdmmc_set_iomux(0, 0xFFFF);

#if defined(CONFIG_SDMMC0_RK29_SDCARD_DET_FROM_GPIO)
    rk30_mux_api_set(GPIO1C1_MMC0_DETN_NAME, GPIO1C_GPIO1C1);
   // gpio_request(RK29SDK_SD_CARD_DETECT_N, "sd-detect");
   // gpio_direction_output(RK29SDK_SD_CARD_DETECT_N,GPIO_HIGH);//set mmc0-data1 to high.
#else
	rk30_mux_api_set(GPIO1C1_MMC0_DETN_NAME, GPIO1C_MMC0_DETN);
#endif	

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
	gpio_request(SDMMC0_WRITE_PROTECT_PIN, "sdmmc-wp");
	gpio_direction_input(SDMMC0_WRITE_PROTECT_PIN);
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

#if defined(CONFIG_SDMMC0_RK29_SDCARD_DET_FROM_GPIO)
    .detect_irq = RK29SDK_SD_CARD_DETECT_N,
    .insert_card_level = RK29SDK_SD_CARD_INSERT_LEVEL,
#else
	.detect_irq = INVALID_GPIO,
#endif

	.enable_sd_wakeup = 0,

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
	.write_prt = SDMMC0_WRITE_PROTECT_PIN,
#else
	.write_prt = INVALID_GPIO,
#endif
};
#endif // CONFIG_SDMMC0_RK29

#ifdef CONFIG_SND_SOC_RK2928
static struct resource resources_acodec[] = {
	{
		.start 	= RK2928_ACODEC_PHYS,
		.end 	= RK2928_ACODEC_PHYS + RK2928_ACODEC_SIZE - 1,
		.flags 	= IORESOURCE_MEM,
	},
	{
		.start	= RK2928_PIN1_PA0,
		.end	= RK2928_PIN1_PA0,
		.flags	= IORESOURCE_IO,
	},
};

static struct platform_device device_acodec = {
	.name	= "rk2928-codec",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resources_acodec),
	.resource	= resources_acodec,
};
#endif

#ifdef CONFIG_BATTERY_RK30_ADC_FAC
static struct rk30_adc_battery_platform_data rk30_adc_battery_platdata = {
        .dc_det_pin      = INVALID_GPIO,
        .batt_low_pin    = INVALID_GPIO,
        .charge_set_pin  = INVALID_GPIO,
        .charge_ok_pin   = INVALID_GPIO,
};

static struct platform_device rk30_device_adc_battery = {
        .name   = "rk30-battery",
        .id     = -1,
        .dev = {
                .platform_data = &rk30_adc_battery_platdata,
        },
};
#endif

static struct platform_device *devices[] __initdata = {
#ifdef CONFIG_FB_ROCKCHIP
	&device_fb,
#endif
#ifdef CONFIG_LCDC_RK2928
	&device_lcdc,
#endif
#ifdef CONFIG_BACKLIGHT_RK29_BL
	&rk29_device_backlight,
#endif
#ifdef CONFIG_ION
	&device_ion,
#endif
#ifdef CONFIG_SND_SOC_RK2928
	&device_acodec,
#endif
#ifdef CONFIG_BATTERY_RK30_ADC_FAC
	&rk30_device_adc_battery,
#endif
};
//i2c
#ifdef CONFIG_I2C0_RK30
#ifdef CONFIG_MFD_TPS65910
#define TPS65910_HOST_IRQ        RK2928_PIN1_PB2
#define PMU_POWER_SLEEP RK2928_PIN1_PA1

static struct pmu_info  tps65910_dcdc_info[] = {
	{
		.name          = "vdd_cpu",   //arm
		.min_uv          = 1200000,
		.max_uv         = 1200000,
	},
	{
		.name          = "vdd2",    //ddr
		.min_uv          = 1200000,
		.max_uv         = 1200000,
	},
	{
		.name          = "vio",   //vcc_io
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
	
};
static  struct pmu_info  tps65910_ldo_info[] = {
	#if defined(CONFIG_MACH_RK2928_TB) || defined(CONFIG_MACH_RK2926_TB)
	{
		.name          = "vpll",   //vcc25
		.min_uv          = 2500000,
		.max_uv         = 2500000,
	},
	{
		.name          = "vdig1",    //vcc18_cif
		.min_uv          = 1800000,
		.max_uv         = 1800000,
	},
	{
		.name          = "vdac",   //vccio_wl
		.min_uv          = 1800000,
		.max_uv         = 1800000,
	},
	#else
	{
		.name          = "vdig1",    //vcc18_cif
		.min_uv          = 1500000,
		.max_uv         = 1500000,
	},

	{
		.name          = "vdig2",   //vdd11
		.min_uv          = 1200000,
		.max_uv         = 1200000,
	},
	{
		.name          = "vaux1",   //vcc28_cif
		.min_uv          = 2800000,
		.max_uv         = 2800000,
	},
	{
		.name          = "vaux2",   //vcca33
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
	{
		.name          = "vaux33",   //vcc_tp
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
	{
		.name          = "vmmc",   //
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
	#endif
 };

#include "board-rk2928-sdk-tps65910.c"
#endif

#ifdef CONFIG_REGULATOR_ACT8931
#ifdef CONFIG_MACH_RK2926_M713
#define ACT8931_HOST_IRQ		RK2928_PIN1_PB1
#else
#define ACT8931_HOST_IRQ		RK2928_PIN1_PB2
#endif
#if defined(CONFIG_MACH_RK2928_SDK)
#define ACT8931_CHGSEL_PIN RK2928_PIN0_PD0
#else
#define ACT8931_CHGSEL_PIN RK2928_PIN1_PA1
#endif

static struct pmu_info  act8931_dcdc_info[] = {
	{
		.name          = "act_dcdc1",   //vcc_io
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
	{
		.name          = "act_dcdc2",    //ddr
		.min_uv          = 1500000,
		.max_uv         = 1500000,
	},
	{
		.name          = "vdd_cpu",   //vdd_arm
		.min_uv          = 1200000,
		.max_uv         = 1200000,
	},
	
};
static  struct pmu_info  act8931_ldo_info[] = {
	{
		.name          = "act_ldo1",   //vcc28_cif
		.min_uv          = 2800000,
		.max_uv         = 2800000,
	},
	{
		.name          = "act_ldo2",    //vcc18_cif
		.min_uv          = 1800000,
		.max_uv         = 1800000,
	},
	{
		.name          = "act_ldo3",    //vcca30
		.min_uv          = 3000000,
		.max_uv         = 3000000,
	},
	{
		.name          = "act_ldo4",    //vcc_wl
		.min_uv          = 3300000,
		.max_uv         = 3300000,
	},
};
#include "board-rk2928-sdk-act8931.c"
#endif

static struct i2c_board_info __initdata i2c0_info[] = {
#if defined (CONFIG_MFD_TPS65910)
	{
        .type           = "tps65910",
        .addr           = TPS65910_I2C_ID0,
        .flags          = 0,
        .irq            = TPS65910_HOST_IRQ,
    	.platform_data = &tps65910_data,
	},
#endif
#if defined (CONFIG_REGULATOR_ACT8931)
	{
		.type    		= "act8931",
		.addr           = 0x5b, 
		.flags			= 0,
		.irq            = ACT8931_HOST_IRQ,
		.platform_data=&act8931_data,
	},
#endif
#ifdef CONFIG_MACH_RK2926_M713
#if defined (CONFIG_RTC_HYM8563)
	{
		.type    		= "rtc_hym8563",
		.addr           = 0x51,
		.flags			= 0,
		.irq            = RK2928_PIN1_PA5,
	},
#endif
#endif
};
#endif

#ifdef CONFIG_MACH_RK2926_M713
int __sramdata gpio0d3_iomux,gpio0d3_do,gpio0d3_dir;
#else
int __sramdata gpio0d4_iomux,gpio0d4_do,gpio0d4_dir;
#endif

#define gpio0_readl(offset)	readl_relaxed(RK2928_GPIO0_BASE + offset)
#define gpio0_writel(v, offset)	do { writel_relaxed(v, RK2928_GPIO0_BASE + offset); dsb(); } while (0)

void __sramfunc rk30_pwm_logic_suspend_voltage(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR

#ifdef CONFIG_MACH_RK2926_M713
	sram_udelay(10000);
	gpio0d3_iomux = readl_relaxed(GRF_GPIO0D_IOMUX);
	gpio0d3_do = gpio0_readl(GPIO_SWPORTA_DR);
	gpio0d3_dir = gpio0_readl(GPIO_SWPORTA_DDR);

	writel_relaxed((gpio0d3_iomux |(1<<22)) & (~(1<<6)), GRF_GPIO0D_IOMUX);
	gpio0_writel(gpio0d3_dir |(1<<27), GPIO_SWPORTA_DDR);
	gpio0_writel(gpio0d3_do |(1<<27), GPIO_SWPORTA_DR);
#else
//	int gpio0d7_iomux,gpio0d7_do,gpio0d7_dir,gpio0d7_en;
	sram_udelay(10000);
	gpio0d4_iomux = readl_relaxed(GRF_GPIO0D_IOMUX);
	gpio0d4_do = gpio0_readl(GPIO_SWPORTA_DR);
	gpio0d4_dir = gpio0_readl(GPIO_SWPORTA_DDR);

	writel_relaxed((gpio0d4_iomux |(1<<24)) & (~(1<<8)), GRF_GPIO0D_IOMUX);
	gpio0_writel(gpio0d4_dir |(1<<28), GPIO_SWPORTA_DDR);
	gpio0_writel(gpio0d4_do |(1<<28), GPIO_SWPORTA_DR);
#endif
	
#endif 
}
void __sramfunc rk30_pwm_logic_resume_voltage(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR
#ifdef CONFIG_MACH_RK2926_M713
	writel_relaxed((1<<22)|gpio0d3_iomux, GRF_GPIO0D_IOMUX);
	gpio0_writel(gpio0d3_dir, GPIO_SWPORTA_DDR);
	gpio0_writel(gpio0d3_do, GPIO_SWPORTA_DR);
	sram_udelay(10000);
#else
	writel_relaxed((1<<24)|gpio0d4_iomux, GRF_GPIO0D_IOMUX);
	gpio0_writel(gpio0d4_dir, GPIO_SWPORTA_DDR);
	gpio0_writel(gpio0d4_do, GPIO_SWPORTA_DR);
	sram_udelay(10000);
#endif

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

void __sramfunc board_pmu_suspend(void)
{      
	#if defined (CONFIG_MFD_TPS65910)
       if(pmic_is_tps65910())
       board_pmu_tps65910_suspend(); 
   	#endif   
}
void __sramfunc board_pmu_resume(void)
{      
	#if defined (CONFIG_MFD_TPS65910)
       if(pmic_is_tps65910())
       board_pmu_tps65910_resume(); 
	#endif
}

#ifdef CONFIG_I2C1_RK30
static struct i2c_board_info __initdata i2c1_info[] = {
#if defined (CONFIG_GS_MMA7660)
		{
			.type		= "gs_mma7660",
			.addr		= 0x4c,
			.flags		= 0,
			.irq		= MMA7660_INT_PIN,
			.platform_data = &mma7660_info,
		},
#endif

};
#endif
#ifdef CONFIG_I2C2_RK30
static struct i2c_board_info __initdata i2c2_info[] = {
#if defined (CONFIG_TOUCHSCREEN_SITRONIX_A720)
{
        .type	        ="sitronix",
#ifdef CONFIG_MACH_RK2926_M713
	.addr           = 0x60,
#else
	.addr           = 0x38,
#endif
	.flags          = 0,
	.irq            = TOUCH_INT_PIN,
	.platform_data = &sitronix_info,
},
#endif
};
#endif
#ifdef CONFIG_I2C3_RK30
static struct i2c_board_info __initdata i2c3_info[] = {
};
#endif
#ifdef CONFIG_I2C_GPIO_RK30
#define I2C_SDA_PIN     INVALID_GPIO   //set sda_pin here
#define I2C_SCL_PIN     INVALID_GPIO   //set scl_pin here
static int rk30_i2c_io_init(void)
{
        //set iomux (gpio) here

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
#ifdef CONFIG_I2C_GPIO_RK30
	i2c_register_board_info(4, i2c_gpio_info, ARRAY_SIZE(i2c_gpio_info));
#endif
}
//end of i2c

#define POWER_ON_PIN RK2928_PIN1_PA2   //power_hold
#if defined(CONFIG_REGULATOR_ACT8931)
extern  int act8931_charge_det ;
#endif
static void rk2928_pm_power_off(void)
{
	printk(KERN_ERR "rk2928_pm_power_off start...\n");
        
        #if defined(CONFIG_REGULATOR_ACT8931)
        if(pmic_is_act8931())
        {
              if(act8931_charge_det)
                   arm_pm_restart(0, NULL);
		 act8931_device_shutdown();
        }
        #endif
	
	#if defined(CONFIG_MFD_TPS65910)	
	if(pmic_is_tps65910())
	{
		tps65910_device_shutdown();//tps65910 shutdown
	}
	#endif
	gpio_direction_output(POWER_ON_PIN, GPIO_LOW);
	
};

static void __init rk2928_board_init(void)
{
	gpio_request(POWER_ON_PIN, "poweronpin");
	gpio_direction_output(POWER_ON_PIN, GPIO_HIGH);
 
	pm_power_off = rk2928_pm_power_off;
	
	rk30_i2c_register_board_info();
	spi_register_board_info(board_spi_devices, ARRAY_SIZE(board_spi_devices));
	platform_add_devices(devices, ARRAY_SIZE(devices));

}

static void __init rk2928_reserve(void)
{
#ifdef CONFIG_ION
	rk30_ion_pdata.heaps[0].base = board_mem_reserve_add("ion", ION_RESERVE_SIZE);
#endif
#ifdef CONFIG_FB_ROCKCHIP
	resource_fb[0].start = board_mem_reserve_add("fb0", RK30_FB0_MEM_SIZE);
	resource_fb[0].end = resource_fb[0].start + RK30_FB0_MEM_SIZE - 1;
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
#if defined(CONFIG_MACH_RK2926_M713)
	{.frequency = 216 * 1000,	.cpu_volt =  850 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 312 * 1000,	.cpu_volt =  900 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 408 * 1000,	.cpu_volt =  950 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 504 * 1000,	.cpu_volt = 1000 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 600 * 1000,	.cpu_volt = 1100 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 696 * 1000,	.cpu_volt = 1175 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 816 * 1000,	.cpu_volt = 1250 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 912 * 1000,	.cpu_volt = 1350 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 1008 * 1000,	.cpu_volt = 1450 * 1000,	.logic_volt = 1200 * 1000},
#else
	{.frequency = 216 * 1000,	.cpu_volt =  850 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 312 * 1000,	.cpu_volt =  900 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 408 * 1000,	.cpu_volt =  975 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 504 * 1000,	.cpu_volt = 1050 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 600 * 1000,	.cpu_volt = 1150 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 696 * 1000,	.cpu_volt = 1225 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 816 * 1000,	.cpu_volt = 1325 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 912 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 1008 * 1000,	.cpu_volt = 1475 * 1000,	.logic_volt = 1200 * 1000},
#endif
#if 0
	{.frequency = 1104 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 1200 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 1104 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 1248 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
#endif
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
	rk2928_clock_data_init(periph_pll_default, codec_pll_default, RK30_CLOCKS_DEFAULT_FLAGS);
	dvfs_set_arm_logic_volt(dvfs_cpu_logic_table, cpu_dvfs_table, dep_cpu2core_table);
	dvfs_set_freq_volt_table(clk_get(NULL, "gpu"), dvfs_gpu_table);
	//dvfs_set_freq_volt_table(clk_get(NULL, "ddr"), dvfs_ddr_table);
	printk("%s end\n", __func__);
}


MACHINE_START(RK2928, "RK2928board")
	.boot_params	= PLAT_PHYS_OFFSET + 0x800,
	.fixup		= rk2928_fixup,
	.reserve	= &rk2928_reserve,
	.map_io		= rk2928_map_io,
	.init_irq	= rk2928_init_irq,
	.timer		= &rk2928_timer,
	.init_machine	= rk2928_board_init,
MACHINE_END
