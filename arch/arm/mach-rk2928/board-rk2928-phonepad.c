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
#include <linux/regulator/rk29-pwm-regulator.h>
#if defined(CONFIG_MODEM_SOUND)
#include "../../../drivers/misc/modem_sound.h"
#endif
#if defined(CONFIG_HDMI_RK30)
	#include "../../../drivers/video/rockchip/hdmi/rk_hdmi.h"
#endif
#include "../../../drivers/headset_observe/rk_headset.h"

#if defined(CONFIG_SPIM_RK29)
#include "../../../drivers/spi/rk29_spim.h"
#endif

#if defined(CONFIG_SC6610)
#include <linux/sc6610.h>
#endif

#include "board-rk2928-phonepad-camera.c" 
#include "board-rk2928-phonepad-key.c"

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
#define PWM_EFFECT_VALUE  1

//#define LCD_DISP_ON_PIN

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
	gpio_direction_output(PWM_GPIO, GPIO_LOW);
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

#define LCD_MUX_NAME  GPIO0C3_UART0_CTSN_NAME
#define LCD_GPIO_MODE GPIO0C_GPIO0C3

#define LCD_EN        RK2928_PIN0_PC3
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
    gpio_set_value(LCD_EN, !LCD_EN_VALUE);
	return 0;
}
static int rk_fb_io_enable(void)
{
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


#if defined (CONFIG_TOUCHSCREEN_I30)
#define TOUCH_RESET_PIN RK2928_PIN3_PD5
#define TOUCH_INT_PIN   RK2928_PIN3_PC7
int ft5306_init_platform_hw(void)
{
	struct regulator *ldo;
//	printk("ft5306_init_platform_hw\n");

	ldo = regulator_get(NULL, "vaux33");
	regulator_disable(ldo);
	
	if(gpio_request(TOUCH_RESET_PIN,NULL) != 0)
	{
		gpio_free(TOUCH_RESET_PIN);
		printk("ft5306_init_platform_hw TOUCH_RESET_PIN error\n");
		return -EIO;
	}

	if(gpio_request(TOUCH_INT_PIN,NULL) != 0)
	{
		gpio_free(TOUCH_INT_PIN);
		printk("ift5306_init_platform_hw TOUCH_INT_PIN error\n");
		return -EIO;
	}
	gpio_direction_input(TOUCH_INT_PIN);
	gpio_direction_output(TOUCH_RESET_PIN, 1);
	gpio_set_value(TOUCH_RESET_PIN,GPIO_HIGH);
	msleep(50);
	regulator_enable(ldo);
	regulator_put(ldo);
	msleep(300);
	return 0;
	
}

void ft5306_exit_platform_hw(void)
{
//	printk("ft5306_exit_platform_hw\n");
	gpio_free(TOUCH_RESET_PIN);
	gpio_free(TOUCH_INT_PIN);
}

int ft5306_platform_sleep(void)
{
#if 0
	struct regulator *ldo;
	ldo = regulator_get(NULL, "ldo9");
	regulator_disable(ldo);
	regulator_put(ldo);

	printk("ft5306_platform_sleep\n");
#endif
	gpio_set_value(TOUCH_RESET_PIN,GPIO_LOW);
	return 0;
}

int ft5306_platform_wakeup(void)
{
#if 0
	struct regulator *ldo;
	ldo = regulator_get(NULL, "ldo9");
        regulator_set_voltage(ldo, 3300000, 3300000);
    	regulator_enable(ldo);
    	regulator_put(ldo);

	printk("ft5306_platform_wakeup\n");
#endif
	gpio_set_value(TOUCH_RESET_PIN,GPIO_HIGH);
	msleep(300);
	return 0;
}

struct ft5306_platform_data ft5306_info = {
  .irq_pin = TOUCH_INT_PIN,
  .rest_pin = TOUCH_RESET_PIN,
  .init_platform_hw= ft5306_init_platform_hw,
  .exit_platform_hw= ft5306_exit_platform_hw,
  .platform_sleep  = ft5306_platform_sleep,
  .platform_wakeup = ft5306_platform_wakeup,

};
#endif

#if defined(CONFIG_TOUCHSCREEN_BYD693X)

#define TOUCH_RESET_PIN RK2928_PIN3_PD5
#define TOUCH_INT_PIN   RK2928_PIN3_PC7
struct byd_platform_data byd693x_info = {
	.int_pin = TOUCH_INT_PIN,
	.rst_pin = TOUCH_RESET_PIN,
	.screen_max_x = 800,
	.screen_max_y = 480,
	.xpol = -1,
};
#endif

/*MMA7660 gsensor*/
#if defined (CONFIG_GS_MMA7660)
#define MMA7660_INT_PIN   RK2928_PIN1_PB1

static int mma7660_init_platform_hw(void)
{
	rk30_mux_api_set(GPIO1B1_SPI_TXD_UART1_SOUT_NAME, GPIO1B_GPIO1B1);

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


#if defined (CONFIG_GS_KXTIK)
#define KXTIK_INT_PIN         RK2928_PIN3_PD1

static struct sensor_platform_data kxtik_pdata = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
	.orientation = {-1, 0, 0, 0, 0, -1, 0, 1, 0},
};

#endif /* CONFIG_GS_KXTIK*/

#ifdef CONFIG_LS_AP321XX
#define LS_AP321XX_INT_PIN         RK2928_PIN0_PC6

static struct sensor_platform_data ls_ap321xx_info = {
	.type = SENSOR_TYPE_LIGHT,
	.irq_enable = 1,
	.poll_delay_ms = 500,
};
#endif
#ifdef CONFIG_PS_AP321XX
#define PS_AP321XX_INT_PIN         RK2928_PIN0_PC6

static struct sensor_platform_data ps_ap321xx_info = {
	.type = SENSOR_TYPE_PROXIMITY,
	.irq_enable = 1,
	.poll_delay_ms = 500,
};
#endif

#if defined(CONFIG_BATTERY_RK30_ADC)||defined(CONFIG_BATTERY_RK30_ADC_FAC)
#define   CHARGE_OK_PIN  RK2928_PIN1_PA0
#define   DC_DET_PIN     RK2928_PIN1_PA5
int rk30_battery_adc_io_init(void){
	int ret = 0;
		
	//dc charge detect pin
	ret = gpio_request(DC_DET_PIN, NULL);
	if (ret) {
		printk("failed to request dc_det gpio\n");
		return ret ;
	}

	gpio_pull_updown(DC_DET_PIN, 0);//important
	ret = gpio_direction_input(DC_DET_PIN);
	if (ret) {
		printk("failed to set gpio dc_det input\n");
		return ret ;
	}
	
	//charge ok pin
	ret = gpio_request(CHARGE_OK_PIN, NULL);
	if (ret) {
		printk("failed to request charge_ok gpio\n");
		return ret ;
	}

	gpio_pull_updown(CHARGE_OK_PIN, 1);//important
	ret = gpio_direction_input(CHARGE_OK_PIN);
	if (ret) {
		printk("failed to set gpio charge_ok input\n");
		return ret ;
	}
	
	return 0;

}
static struct rk30_adc_battery_platform_data rk30_adc_battery_platdata = {
        .dc_det_pin      = RK2928_PIN1_PA5,
        .batt_low_pin    = INVALID_GPIO,
        .charge_set_pin  = INVALID_GPIO,
        .charge_ok_pin   = RK2928_PIN1_PA0,
        .dc_det_level    = GPIO_LOW,  //
        .charge_ok_level = GPIO_HIGH,

        .io_init = rk30_battery_adc_io_init,
};

static struct platform_device rk30_device_adc_battery = {
        .name   = "rk30-battery",
        .id     = -1,
        .dev = {
                .platform_data = &rk30_adc_battery_platdata,
        },
};
#endif


#if CONFIG_RK30_PWM_REGULATOR
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
		.pwm_id = 2,
		.pwm_gpio = RK2928_PIN0_PD4,
		.pwm_iomux_name = GPIO0D4_PWM_2_NAME,
		.pwm_iomux_pwm = GPIO0D_PWM_2, 
		.pwm_iomux_gpio = GPIO0D_GPIO0D4,
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

#if defined(CONFIG_MODEM_SOUND)

struct modem_sound_data modem_sound_info = {
	.spkctl_io = RK2928_PIN3_PD4,
	.spkctl_active = GPIO_HIGH,
};

struct platform_device modem_sound_device = {
	.name = "modem_sound",
	.id = -1,
	.dev		= {
	.platform_data = &modem_sound_info,
		}
	};
#endif
/**************************************************************************************************
 * SDMMC devices,  include the module of SD,MMC,and sdio.noted by xbw at 2012-03-05
**************************************************************************************************/
#ifdef CONFIG_SDMMC_RK29
#include "board-rk2928-phonepad-sdmmc.c"

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
#define SDMMC0_WRITE_PROTECT_PIN	RK2928_PIN1_PA7	//According to your own project to set the value of write-protect-pin.
#endif

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
#define SDMMC1_WRITE_PROTECT_PIN	RK2928_PIN0_PD5	//According to your own project to set the value of write-protect-pin.
#endif

#define RK29SDK_WIFI_SDIO_CARD_DETECT_N    RK2928_PIN0_PB2

#endif //endif ---#ifdef CONFIG_SDMMC_RK29

#ifdef CONFIG_SDMMC0_RK29
static int rk29_sdmmc0_cfg_gpio(void)
{
	rk29_sdmmc_set_iomux(0, 0xFFFF);

	rk30_mux_api_set(GPIO1C1_MMC0_DETN_NAME, GPIO1C_MMC0_DETN);

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
	.detect_irq =  INVALID_GPIO,
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
	rk30_mux_api_set(GPIO0B0_MMC1_CMD_NAME, GPIO0B_MMC1_CMD);
	rk30_mux_api_set(GPIO0B1_MMC1_CLKOUT_NAME, GPIO0B_MMC1_CLKOUT);
	rk30_mux_api_set(GPIO0B3_MMC1_D0_NAME, GPIO0B_MMC1_D0);
	rk30_mux_api_set(GPIO0B4_MMC1_D1_NAME, GPIO0B_MMC1_D1);
	rk30_mux_api_set(GPIO0B5_MMC1_D2_NAME, GPIO0B_MMC1_D2);
	rk30_mux_api_set(GPIO0B6_MMC1_D3_NAME, GPIO0B_MMC1_D3);
	//rk30_mux_api_set(GPIO0B2_MMC1_DETN_NAME, GPIO0B_MMC1_DETN);

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
#if 1
	.detect_irq = INVALID_GPIO,//RK29SDK_WIFI_SDIO_CARD_DETECT_N,
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

#if defined(CONFIG_SC6610)
static int sc6610_io_init(void)
{
        
        return 0;
}

static int sc6610_io_deinit(void)
{
        

        return 0;
}

struct rk29_sc6610_data rk29_sc6610_info = {
        .io_init = sc6610_io_init,
        .io_deinit = sc6610_io_deinit,
        .bp_power = RK2928_PIN3_PC2,//RK29_PIN0_PB4,
        .bp_reset = INVALID_GPIO,//RK29_PIN0_PB3,
        .bp_wakeup_ap = RK2928_PIN3_PC3,//RK29_PIN0_PC2,
        .ap_wakeup_bp = RK2928_PIN3_PC4,//RK29_PIN0_PB0, 
        .modem_assert = RK2928_PIN3_PC5,
};
struct platform_device rk29_device_sc6610 = {
        .name = "SC6610",
        .id = -1,
        .dev            = {
                .platform_data = &rk29_sc6610_info,
        }
    };
#endif
#if defined (CONFIG_RK_HEADSET_DET) || defined (CONFIG_RK_HEADSET_IRQ_HOOK_ADC_DET)
static int rk_headset_io_init(int gpio)
{
	int ret;
	ret = gpio_request(gpio, "headset_io");
	if(ret) 
		return ret;

	rk30_mux_api_set(GPIO1B4_SPI_CSN1_NAME, GPIO1B_GPIO1B4);
	gpio_pull_updown(gpio, PullDisable);
	gpio_direction_input(gpio);
	mdelay(50);
	return 0;
};

static int rk_hook_io_init(int gpio)
{
	int ret;
	ret = gpio_request(gpio, "hook_io");
	if(ret) 
		return ret;

	rk30_mux_api_set(GPIO0D1_UART2_CTSN_NAME, GPIO0D_GPIO0D1);
	gpio_pull_updown(gpio, PullDisable);
	gpio_direction_input(gpio);
	mdelay(50);
	return 0;
};

struct rk_headset_pdata rk_headset_info = {
		.Headset_gpio		= RK2928_PIN1_PB4,
		.Hook_gpio  = RK2928_PIN0_PD1,
		.Hook_down_type = HOOK_DOWN_HIGH,
		.headset_in_type = HEADSET_IN_HIGH,
		.hook_key_code = KEY_MEDIA,
		.headset_io_init = rk_headset_io_init,
		.hook_io_init = rk_hook_io_init,
};
struct platform_device rk_device_headset = {
		.name	= "rk_headsetdet",
		.id 	= 0,
		.dev    = {
			    .platform_data = &rk_headset_info,
		}
};
#endif
#ifdef CONFIG_SND_SOC_RK2928
static struct resource resources_acodec[] = {
	{
		.start 	= RK2928_ACODEC_PHYS,
		.end 	= RK2928_ACODEC_PHYS + RK2928_ACODEC_SIZE - 1,
		.flags 	= IORESOURCE_MEM,
	},
	{
		.start	= RK2928_PIN3_PD4,
		.end	= RK2928_PIN3_PD4,
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

#if defined(CONFIG_BATTERY_RK30_ADC)||defined(CONFIG_BATTERY_RK30_ADC_FAC)
 	&rk30_device_adc_battery,
#endif
#if defined(CONFIG_SC6610)
        &rk29_device_sc6610,

#endif
#if defined (CONFIG_RK_HEADSET_DET) ||  defined (CONFIG_RK_HEADSET_IRQ_HOOK_ADC_DET)
	&rk_device_headset,
#endif
#if defined (CONFIG_MODEM_SOUND)
 &modem_sound_device,
#endif
};
//i2c
#ifdef CONFIG_I2C0_RK30
#ifdef CONFIG_MFD_TPS65910
#define TPS65910_HOST_IRQ        RK2928_PIN3_PC6
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
		.platform_data=&act8931_data,
	},
#endif
};
#endif
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


#if defined (CONFIG_GS_KXTIK)
		{
				.type		= "gs_kxtik",
				.addr		= 0x0F,
				.flags		= 0,
				.platform_data = &kxtik_pdata,
				.irq = KXTIK_INT_PIN, // Replace with appropriate GPIO setup
		},
#endif

#ifdef CONFIG_LS_AP321XX
        {
                .type                   = "ls_ap321xx",
                .addr                   = 0x1E,
                .flags                  = 0,
                .irq                     = LS_AP321XX_INT_PIN,
                .platform_data = &ls_ap321xx_info
        },
#endif

#ifdef CONFIG_PS_AP321XX
        {
                .type                   = "ps_ap321xx",
                .addr                   = 0x1E,
                .flags                  = 0,
                .irq                     = PS_AP321XX_INT_PIN,
                .platform_data = &ps_ap321xx_info
        },
#endif

#ifdef CONFIG_RDA5990
#define RDA_WIFI_CORE_ADDR (0x13)
#define RDA_WIFI_RF_ADDR (0x14) //correct add is 0x14
#define RDA_BT_CORE_ADDR (0x15)
#define RDA_BT_RF_ADDR (0x16)

#define RDA_WIFI_RF_I2C_DEVNAME "rda_wifi_rf_i2c"
#define RDA_WIFI_CORE_I2C_DEVNAME "rda_wifi_core_i2c"
#define RDA_BT_RF_I2C_DEVNAME "rda_bt_rf_i2c"
#define RDA_BT_CORE_I2C_DEVNAME "rda_bt_core_i2c"
		{
			.type          = RDA_WIFI_CORE_I2C_DEVNAME,
			.addr          = RDA_WIFI_CORE_ADDR,
                	.flags         = 0,

		},

		{
			.type          = RDA_WIFI_RF_I2C_DEVNAME,
			.addr          = RDA_WIFI_RF_ADDR,
                	.flags         = 0,

		},
		{
			.type          = RDA_BT_CORE_I2C_DEVNAME,
			.addr          = RDA_BT_CORE_ADDR,
                	.flags         = 0,

		},
		{
			.type          = RDA_BT_RF_I2C_DEVNAME,
			.addr          = RDA_BT_RF_ADDR,
                	.flags         = 0,

		},
#endif

};
#endif
#ifdef CONFIG_I2C2_RK30
static struct i2c_board_info __initdata i2c2_info[] = {
#if defined (CONFIG_TOUCHSCREEN_I30)
	{
		.type          = "ft5x0x_ts",
		.addr          = 0x38,
		.flags         = 0,
		.irq           = TOUCH_INT_PIN,
		.platform_data = &ft5306_info,
	},
#endif
#if defined(CONFIG_TOUCHSCREEN_BYD693X)
	{
		.type          = "byd693x-ts",
		.addr          = 0x52,
		.flags         = 0,
		.irq           = TOUCH_INT_PIN,
		.platform_data = &byd693x_info,
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
static void rk2928_pm_power_off(void)
{
	printk(KERN_ERR "rk2928_pm_power_off start...\n");
	
	#if defined(CONFIG_MFD_TPS65910)
		tps65910_device_shutdown();//tps65910 shutdown
	#endif
	gpio_direction_output(POWER_ON_PIN, GPIO_LOW);
	
};

static void __init rk2928_board_init(void)
{
	gpio_request(POWER_ON_PIN, "poweronpin");
	gpio_direction_output(POWER_ON_PIN, GPIO_HIGH);
        gpio_free(POWER_ON_PIN);
	
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
	{.frequency = 216 * 1000,	.cpu_volt = 1200 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 312 * 1000,	.cpu_volt = 1200 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 408 * 1000,	.cpu_volt = 1200 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 504 * 1000,	.cpu_volt = 1200 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 600 * 1000,	.cpu_volt = 1200 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 696 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 816 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
	//{.frequency = 912 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
	//{.frequency = 1008 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
#if 0
	{.frequency = 1104 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 1200 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 1104 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
	{.frequency = 1248 * 1000,	.cpu_volt = 1400 * 1000,	.logic_volt = 1200 * 1000},
#endif
	//{.frequency = 1000 * 1000,	.cpu_volt = 1225 * 1000,	.logic_volt = 1200 * 1000},
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
