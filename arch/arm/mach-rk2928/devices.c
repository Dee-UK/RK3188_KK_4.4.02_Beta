/* arch/arm/mach-rk2928/devices.c
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

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#ifdef CONFIG_USB_ANDROID
#include <linux/usb/android_composite.h>
#endif
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <asm/pmu.h>
#include <mach/irqs.h>
#include <mach/board.h>
#include <mach/dma-pl330.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#include <mach/dvfs.h>
#include <mach/debug_uart.h>
#include <plat/rk_fiq_debugger.h>
#include <linux/regulator/consumer.h>

#ifdef CONFIG_ADC_RK30
static int rk30_get_base_volt(void)
{
        int volt;
        struct regulator *logic = dvfs_get_regulator("vdd_core");

        if(unlikely(IS_ERR_OR_NULL(logic))){
                printk("%s: fail to get logic voltage\n", __func__);
                return -EINVAL;
        }
        volt =  regulator_get_voltage(logic)/1000;

        return volt;
}

static struct adc_platform_data rk30_adc_pdata = {
        .ref_volt = 3300, //3300mV
#if  defined(CONFIG_ARCH_RK3026)  
	.base_chn = 6,
#else
	.base_chn = 3,
#endif
        .get_base_volt = &rk30_get_base_volt,
};

static struct resource rk30_adc_resource[] = {
	{
		.start	= IRQ_SARADC,
		.end	= IRQ_SARADC,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RK2928_SARADC_PHYS,
		.end	= RK2928_SARADC_PHYS + RK2928_SARADC_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

struct platform_device device_adc = {
	.name		= "rk30-adc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(rk30_adc_resource),
	.resource	= rk30_adc_resource,
        .dev            = {
		.platform_data = &rk30_adc_pdata,
        },
};
#endif

static u64 dma_dmamask = DMA_BIT_MASK(32);

static struct resource resource_dmac[] = {
	[0] = {
		.start  = RK2928_DMAC_PHYS,
		.end    = RK2928_DMAC_PHYS + RK2928_DMAC_SIZE -1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_DMAC_0,
		.end	= IRQ_DMAC_1,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct rk29_pl330_platdata dmac_pdata = {
	.peri = {
		[0] = DMACH_I2S0_8CH_TX,
		[1] = DMACH_I2S0_8CH_RX,
		[2] = DMACH_UART0_TX,
		[3] = DMACH_UART0_RX,
		[4] = DMACH_UART1_TX,
		[5] = DMACH_UART1_RX,
		[6] = DMACH_UART2_TX,
		[7] = DMACH_UART2_RX,
		[8] = DMACH_SPI0_TX,
		[9] = DMACH_SPI0_RX,
		[10] = DMACH_SDMMC,
		[11] = DMACH_SDIO,
		[12] = DMACH_EMMC,
		[13] = DMACH_DMAC1_MEMTOMEM,
		[14] = DMACH_MAX,
		[15] = DMACH_MAX,
		[16] = DMACH_MAX,
		[17] = DMACH_MAX,
		[18] = DMACH_MAX,
		[19] = DMACH_MAX,
		[20] = DMACH_MAX,
		[21] = DMACH_MAX,
		[22] = DMACH_MAX,
		[23] = DMACH_MAX,
		[24] = DMACH_MAX,
		[25] = DMACH_MAX,
		[26] = DMACH_MAX,
		[27] = DMACH_MAX,
		[28] = DMACH_MAX,
		[29] = DMACH_MAX,
		[30] = DMACH_MAX,
		[31] = DMACH_MAX,
	},
};

static struct platform_device device_dmac = {
	.name		= "rk29-pl330",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resource_dmac),
	.resource	= resource_dmac,
	.dev		= {
		.dma_mask = &dma_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &dmac_pdata,
	},
};

static struct platform_device *rk2928_dmacs[] __initdata = {
	&device_dmac,
};

static void __init rk2928_init_dma(void)
{
	platform_add_devices(rk2928_dmacs, ARRAY_SIZE(rk2928_dmacs));
}

#ifdef CONFIG_UART0_RK29
static struct resource resources_uart0[] = {
	{
		.start	= IRQ_UART0,
		.end	= IRQ_UART0,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RK2928_UART0_PHYS,
		.end	= RK2928_UART0_PHYS + RK2928_UART0_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device device_uart0 = {
	.name	= "rk_serial",
	.id	= 0,
	.num_resources	= ARRAY_SIZE(resources_uart0),
	.resource	= resources_uart0,
};
#endif

#ifdef CONFIG_UART1_RK29
static struct resource resources_uart1[] = {
	{
		.start	= IRQ_UART1,
		.end	= IRQ_UART1,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RK2928_UART1_PHYS,
		.end	= RK2928_UART1_PHYS + RK2928_UART1_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device device_uart1 = {
	.name	= "rk_serial",
	.id	= 1,
	.num_resources	= ARRAY_SIZE(resources_uart1),
	.resource	= resources_uart1,
};
#endif

#ifdef CONFIG_UART2_RK29
static struct resource resources_uart2[] = {
	{
		.start	= IRQ_UART2,
		.end	= IRQ_UART2,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RK2928_UART2_PHYS,
		.end	= RK2928_UART2_PHYS + RK2928_UART2_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device device_uart2 = {
	.name	= "rk_serial",
	.id	= 2,
	.num_resources	= ARRAY_SIZE(resources_uart2),
	.resource	= resources_uart2,
};
#endif

static void __init rk2928_init_uart(void)
{
#ifdef CONFIG_UART0_RK29
	platform_device_register(&device_uart0);
#endif
#ifdef CONFIG_UART1_RK29
	platform_device_register(&device_uart1);
#endif
#ifdef CONFIG_UART2_RK29
	platform_device_register(&device_uart2);
#endif
}

// i2c
#ifdef CONFIG_I2C_RK30
#ifdef CONFIG_I2C0_CONTROLLER_RK29
#define I2C0_ADAP_TYPE  I2C_RK29_ADAP
#define I2C0_START      RK2928_I2C0_PHYS
#define I2C0_END        RK2928_I2C0_PHYS + RK2928_I2C0_SIZE - 1
#endif
#ifdef CONFIG_I2C0_CONTROLLER_RK30
#define I2C0_ADAP_TYPE   I2C_RK30_ADAP
#define I2C0_START      RK2928_RKI2C0_PHYS
#define I2C0_END        RK2928_RKI2C0_PHYS + RK2928_RKI2C0_SIZE - 1
#endif

#ifdef CONFIG_I2C1_CONTROLLER_RK29
#define I2C1_ADAP_TYPE  I2C_RK29_ADAP
#define I2C1_START      RK2928_I2C1_PHYS
#define I2C1_END        RK2928_I2C1_PHYS + RK2928_I2C1_SIZE - 1
#endif
#ifdef CONFIG_I2C1_CONTROLLER_RK30
#define I2C1_ADAP_TYPE   I2C_RK30_ADAP
#define I2C1_START      RK2928_RKI2C1_PHYS 
#define I2C1_END        RK2928_RKI2C1_PHYS + RK2928_RKI2C1_SIZE - 1
#endif

#ifdef CONFIG_I2C2_CONTROLLER_RK29
#define I2C2_ADAP_TYPE  I2C_RK29_ADAP
#define I2C2_START      RK2928_I2C2_PHYS
#define I2C2_END        RK2928_I2C2_PHYS + RK2928_I2C2_SIZE - 1
#endif
#ifdef CONFIG_I2C2_CONTROLLER_RK30
#define I2C2_ADAP_TYPE   I2C_RK30_ADAP
#define I2C2_START      RK2928_RKI2C2_PHYS
#define I2C2_END        RK2928_RKI2C2_PHYS + RK2928_RKI2C2_SIZE - 1
#endif

#ifdef CONFIG_I2C3_CONTROLLER_RK29
#define I2C3_ADAP_TYPE  I2C_RK29_ADAP
#define I2C3_START      RK2928_I2C3_PHYS
#define I2C3_END        RK2928_I2C3_PHYS + RK2928_I2C3_SIZE - 1
#endif
#ifdef CONFIG_I2C3_CONTROLLER_RK30
#define I2C3_ADAP_TYPE   I2C_RK30_ADAP
#define I2C3_START      RK2928_RKI2C3_PHYS
#define I2C3_END        RK2928_RKI2C3_PHYS + RK2928_RKI2C3_SIZE - 1
#endif

#ifdef CONFIG_I2C0_RK30
static struct rk30_i2c_platform_data default_i2c0_data = {
	.bus_num = 0,
	.is_div_from_arm = 1,
	.adap_type = I2C0_ADAP_TYPE,
	.sda_mode = I2C0_SDA,
	.scl_mode = I2C0_SCL,

};

static struct resource resources_i2c0[] = {
	{
		.start	= IRQ_I2C0,
		.end	= IRQ_I2C0,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= I2C0_START,
        .end    = I2C0_END,    
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device device_i2c0 = {
	.name	= "rk30_i2c",
	.id	= 0,
	.num_resources	= ARRAY_SIZE(resources_i2c0),
	.resource	= resources_i2c0,
	.dev 		= {
		.platform_data = &default_i2c0_data,
	},
};
#endif

#ifdef CONFIG_I2C1_RK30
static struct rk30_i2c_platform_data default_i2c1_data = {
	.bus_num = 1,
	.is_div_from_arm = 1,
	.adap_type = I2C1_ADAP_TYPE,
	.sda_mode = I2C1_SDA,
	.scl_mode = I2C1_SCL,
};

static struct resource resources_i2c1[] = {
	{
		.start	= IRQ_I2C1,
		.end	= IRQ_I2C1,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= I2C1_START,
        .end    = I2C1_END,    
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device device_i2c1 = {
	.name	= "rk30_i2c",
	.id	= 1,
	.num_resources	= ARRAY_SIZE(resources_i2c1),
	.resource	= resources_i2c1,
	.dev 		= {
		.platform_data = &default_i2c1_data,
	},
};
#endif

#ifdef CONFIG_I2C2_RK30
static struct rk30_i2c_platform_data default_i2c2_data = {
	.bus_num = 2,
	.is_div_from_arm = 0,
	.adap_type = I2C2_ADAP_TYPE,
	.sda_mode = I2C2_SDA,
	.scl_mode = I2C2_SCL,
};

static struct resource resources_i2c2[] = {
	{
		.start	= IRQ_I2C2,
		.end	= IRQ_I2C2,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= I2C2_START,
        .end    = I2C2_END,    
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device device_i2c2 = {
	.name	= "rk30_i2c",
	.id	= 2,
	.num_resources	= ARRAY_SIZE(resources_i2c2),
	.resource	= resources_i2c2,
	.dev 		= {
		.platform_data = &default_i2c2_data,
	},
};
#endif

#ifdef CONFIG_I2C3_RK30
static struct rk30_i2c_platform_data default_i2c3_data = {
	.bus_num = 3,
	.is_div_from_arm = 0,
	.adap_type = I2C3_ADAP_TYPE,
	.sda_mode = I2C3_SDA,
	.scl_mode = I2C3_SCL,
};

static struct resource resources_i2c3[] = {
	{
		.start	= IRQ_I2C3,
		.end	= IRQ_I2C3,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= I2C3_START,
        .end    = I2C3_END,    
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device device_i2c3 = {
	.name	= "rk30_i2c",
	.id	= 3,
	.num_resources	= ARRAY_SIZE(resources_i2c3),
	.resource	= resources_i2c3,
	.dev 		= {
		.platform_data = &default_i2c3_data,
	},
};
#endif

#ifdef CONFIG_I2C_GPIO_RK30
static struct platform_device device_i2c_gpio = {
        .name   = "i2c-gpio",
        .id = 4,
        .dev            = {
                .platform_data = &default_i2c_gpio_data,
        },
};
#endif

static void __init rk2928_init_i2c(void)
{
#ifdef CONFIG_I2C0_RK30
	platform_device_register(&device_i2c0);
#endif
#ifdef CONFIG_I2C1_RK30
	platform_device_register(&device_i2c1);
#endif
#ifdef CONFIG_I2C2_RK30
	platform_device_register(&device_i2c2);
#endif
#ifdef CONFIG_I2C3_RK30
	platform_device_register(&device_i2c3);
#endif
#ifdef CONFIG_I2C_GPIO_RK30
	platform_device_register(&device_i2c_gpio);
#endif
}
#endif//end of i2c

#if defined(CONFIG_SPIM0_RK29) || defined(CONFIG_SPIM1_RK29)
/*****************************************************************************************
 * spi devices
 * author: cmc@rock-chips.com
 *****************************************************************************************/
#define SPI_CHIPSELECT_NUM 2

static int spi_io_init(struct spi_cs_gpio *cs_gpios, int cs_num)
{
	int i;
	if (cs_gpios) {
		for (i = 0; i < cs_num; i++) {
			rk30_mux_api_set(cs_gpios[i].cs_iomux_name, cs_gpios[i].cs_iomux_mode);
		}
	}
	return 0;
}

static int spi_io_deinit(struct spi_cs_gpio *cs_gpios, int cs_num)
{
	return 0;
}

static int spi_io_fix_leakage_bug(void)
{
#if 0
	gpio_direction_output(RK29_PIN2_PC1, GPIO_LOW);
#endif
	return 0;
}

static int spi_io_resume_leakage_bug(void)
{
#if 0
	gpio_direction_output(RK29_PIN2_PC1, GPIO_HIGH);
#endif
	return 0;
}
#endif

/*
 * rk29xx spi master device
 */
#ifdef CONFIG_SPIM0_RK29
static struct spi_cs_gpio rk29xx_spi0_cs_gpios[SPI_CHIPSELECT_NUM] = {
#if 0
	{
		.name = "spi0 cs0",
		.cs_gpio = RK2928_PIN1_PB3,
		.cs_iomux_name = GPIO1B3_SPI_CSN0_UART1_RTSN_NAME,
		.cs_iomux_mode = GPIO1B_SPI_CSN0,
	},
	{
		.name = "spi0 cs1",
		.cs_gpio = RK2928_PIN1_PB4,
		.cs_iomux_name = GPIO1B4_SPI_CSN1_UART1_CTSN_NAME,//if no iomux,set it NULL
		.cs_iomux_mode = GPIO1B_SPI_CSN1,
	},
#endif
};

static struct rk29xx_spi_platform_data rk29xx_spi0_platdata = {
	.num_chipselect = SPI_CHIPSELECT_NUM,
	.chipselect_gpios = rk29xx_spi0_cs_gpios,
	.io_init = spi_io_init,
	.io_deinit = spi_io_deinit,
	.io_fix_leakage_bug = spi_io_fix_leakage_bug,
	.io_resume_leakage_bug = spi_io_resume_leakage_bug,
};

static struct resource rk29_spi0_resources[] = {
	{
		.start	= IRQ_SPI,
		.end	= IRQ_SPI,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RK2928_SPI_PHYS,
		.end	= RK2928_SPI_PHYS + RK2928_SPI_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start  = DMACH_SPI0_TX,
		.end    = DMACH_SPI0_TX,
		.flags  = IORESOURCE_DMA,
	},
	{
		.start  = DMACH_SPI0_RX,
		.end    = DMACH_SPI0_RX,
		.flags  = IORESOURCE_DMA,
	},
};

struct platform_device rk29xx_device_spi0m = {
	.name	= "rk29xx_spim",
	.id	= 0,
	.num_resources	= ARRAY_SIZE(rk29_spi0_resources),
	.resource	= rk29_spi0_resources,
	.dev			= {
		.dma_mask = &dma_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data	= &rk29xx_spi0_platdata,
	},
};
#endif

static void __init rk2928_init_spim(void)
{
#ifdef CONFIG_SPIM0_RK29
	platform_device_register(&rk29xx_device_spi0m);
#endif
}

#ifdef CONFIG_MTD_NAND_RK29XX
static struct resource resources_nand[] = {
	{
		.start	= RK2928_NANDC_PHYS,
		.end	= RK2928_NANDC_PHYS + RK2928_NANDC_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	}
};

static struct platform_device device_nand = {
	.name		= "rk29xxnand",
	.id		= -1,
	.resource	= resources_nand,
	.num_resources	= ARRAY_SIZE(resources_nand),
};
#endif
#if defined(CONFIG_HDMI_RK2928) || defined(CONFIG_HDMI_RK616)
static struct resource resource_hdmi[] = {
	[0] = {
		.start = RK2928_HDMI_PHYS,
		.end   = RK2928_HDMI_PHYS + RK2928_HDMI_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_HDMI,
		.end   = IRQ_HDMI,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device device_hdmi = {
#ifdef CONFIG_HDMI_RK616
        .name                           = "rk3026-hdmi",
#else
	.name				= "rk2928-hdmi",
#endif
	.id					= -1,
	.num_resources		= ARRAY_SIZE(resource_hdmi),
	.resource			= resource_hdmi,
};
#endif

#ifdef CONFIG_RGA_RK30
static struct resource resource_rga[] = {
	[0] = {
		.start = RK2928_RGA_PHYS,
		.end   = RK2928_RGA_PHYS + RK2928_RGA_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_RGA,
		.end   = IRQ_RGA,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device device_rga = {
	.name		= "rga",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resource_rga),
	.resource	= resource_rga,
};
#endif
#ifdef CONFIG_SND_RK29_SOC_I2S
#ifdef CONFIG_SND_RK29_SOC_I2S_8CH
static struct resource resource_iis0_8ch[] = {
	[0] = {
		.start	= RK2928_I2S_PHYS,
		.end	= RK2928_I2S_PHYS + RK2928_I2S_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= DMACH_I2S0_8CH_TX,
		.end	= DMACH_I2S0_8CH_TX,
		.flags	= IORESOURCE_DMA,
	},
	[2] = {
		.start	= DMACH_I2S0_8CH_RX,
		.end	= DMACH_I2S0_8CH_RX,
		.flags	= IORESOURCE_DMA,
	},
	[3] = {
		.start	= IRQ_I2S,
		.end	= IRQ_I2S,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device device_iis0_8ch = {
	.name		= "rk29_i2s",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resource_iis0_8ch),
	.resource	= resource_iis0_8ch,
};
#endif
#endif
static struct platform_device device_pcm = {
	.name = "rockchip-audio",
	.id = -1,
};

static void __init rk2928_init_i2s(void)
{
#ifdef CONFIG_SND_RK29_SOC_I2S_8CH
	platform_device_register(&device_iis0_8ch);
#endif
	platform_device_register(&device_pcm);
}
#ifdef CONFIG_KEYS_RK29
extern struct rk29_keys_platform_data rk29_keys_pdata;
static struct platform_device device_keys = {
	.name		= "rk29-keypad",
	.id		= -1,
	.dev		= {
		.platform_data	= &rk29_keys_pdata,
	},
};
#endif

#ifdef CONFIG_EMMC_RK
static struct resource resources_emmc[] = {
	{
		.start 	= IRQ_EMMC,
		.end 	= IRQ_EMMC,
		.flags 	= IORESOURCE_IRQ,
	},
	{
		.start 	= RK2928_EMMC_PHYS,
		.end 	= RK2928_EMMC_PHYS + RK2928_EMMC_SIZE - 1,
		.flags 	= IORESOURCE_MEM,
	}
};

static struct platform_device device_emmc = {
	.name		= "emmc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resources_emmc),
	.resource	= resources_emmc,
	.dev 		= {
		.platform_data = NULL,
	},
};
#endif

#ifdef CONFIG_SDMMC0_RK29
static struct resource resources_sdmmc0[] = {
	{
		.start 	= IRQ_SDMMC,
		.end 	= IRQ_SDMMC,
		.flags 	= IORESOURCE_IRQ,
	},
	{
		.start 	= RK2928_SDMMC_PHYS,
		.end 	= RK2928_SDMMC_PHYS + RK2928_SDMMC_SIZE -1,
		.flags 	= IORESOURCE_MEM,
	}
};

static struct platform_device device_sdmmc0 = {
	.name		= "rk29_sdmmc",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_sdmmc0),
	.resource	= resources_sdmmc0,
	.dev 		= {
		.platform_data = &default_sdmmc0_data,
	},
};
#endif

#ifdef CONFIG_SDMMC1_RK29
static struct resource resources_sdmmc1[] = {
	{
		.start 	= IRQ_SDIO,
		.end 	= IRQ_SDIO,
		.flags 	= IORESOURCE_IRQ,
	},
	{
		.start 	= RK2928_SDIO_PHYS,
		.end 	= RK2928_SDIO_PHYS + RK2928_SDIO_SIZE - 1,
		.flags 	= IORESOURCE_MEM,
	}
};

static struct platform_device device_sdmmc1 = {
	.name		= "rk29_sdmmc",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(resources_sdmmc1),
	.resource	= resources_sdmmc1,
	.dev 		= {
		.platform_data = &default_sdmmc1_data,
	},
};
#endif
static void __init rk2928_init_sdmmc(void)
{
#ifdef CONFIG_EMMC_RK
	platform_device_register(&device_emmc);
#endif
#ifdef CONFIG_SDMMC0_RK29
	platform_device_register(&device_sdmmc0);
#endif
#ifdef CONFIG_SDMMC1_RK29
	platform_device_register(&device_sdmmc1);
#endif
}

#ifdef CONFIG_RK29_WATCHDOG
static struct resource resources_wdt[] = {
	{
		.start	= IRQ_WDT,
		.end	= IRQ_WDT,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RK2928_WDT_PHYS,
		.end	= RK2928_WDT_PHYS + RK2928_WDT_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device device_wdt = {
	.name	= "rk29-wdt",
	.id	= 0,
	.num_resources	= ARRAY_SIZE(resources_wdt),
	.resource	= resources_wdt,
};
#endif

static struct resource resource_arm_pmu[] = {
	{
		.start	= IRQ_ARM_PMU,
		.end	= IRQ_ARM_PMU,
		.flags	= IORESOURCE_IRQ,
	},
#if defined(CONFIG_ARCH_RK3026)
	{
		.start	= IRQ_ARM_PMU + 1,
		.end	= IRQ_ARM_PMU + 1,
		.flags	= IORESOURCE_IRQ,
	},
#endif
};

static struct platform_device device_arm_pmu = {
	.name		= "arm-pmu",
	.id		= ARM_PMU_DEVICE_CPU,
	.num_resources	= ARRAY_SIZE(resource_arm_pmu),
	.resource	= resource_arm_pmu,
};


#ifdef CONFIG_RK3026_LVDS
static struct resource resource_lvds[] = {
	{
		.start 	= RK2928_GRF_PHYS + GRF_LVDS_CON0,
		.end 	= RK2928_GRF_PHYS + GRF_DMAC_CON0,
		.flags 	= IORESOURCE_MEM,
	},
};

static struct platform_device device_lvds = {
	.name		= "rk3026-lvds",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resource_lvds),
	.resource	= resource_lvds,
};
#endif

static struct resource resource_vpu[] = {
	{
		.start	= IRQ_VDPU,
		.end	= IRQ_VDPU,
		.name	= "irq_vdpu",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= IRQ_VEPU,
		.end	= IRQ_VEPU,
		.name	= "irq_vepu",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RK2928_VCODEC_PHYS,
		.end	= RK2928_VCODEC_PHYS + RK2928_VCODEC_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device device_vpu = {
	.name		= "vpu",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resource_vpu),
	.resource	= resource_vpu,
};

static int __init rk2928_init_devices(void)
{
	rk2928_init_dma();
	rk2928_init_uart();
	rk2928_init_i2c();
	rk2928_init_spim();
#ifdef CONFIG_MTD_NAND_RK29XX
	platform_device_register(&device_nand);
#endif
#ifdef CONFIG_ADC_RK30
	platform_device_register(&device_adc);
#endif
#ifdef CONFIG_KEYS_RK29
	platform_device_register(&device_keys);
#endif
#ifdef CONFIG_RGA_RK30
	platform_device_register(&device_rga);
#endif
	rk2928_init_sdmmc();
#if defined(CONFIG_FIQ_DEBUGGER) && defined(DEBUG_UART_PHYS)
	rk_serial_debug_init(DEBUG_UART_BASE, IRQ_DEBUG_UART, IRQ_UART_SIGNAL, -1);
#endif
	rk2928_init_i2s();
#if defined(CONFIG_HDMI_RK2928) || defined(CONFIG_HDMI_RK616)
	platform_device_register(&device_hdmi);
#endif
#ifdef CONFIG_RK29_WATCHDOG
	platform_device_register(&device_wdt);
#endif
	platform_device_register(&device_arm_pmu);

#ifdef CONFIG_RK3026_LVDS
	platform_device_register(&device_lvds);
#endif
	platform_device_register(&device_vpu);

	return 0;
}
arch_initcall(rk2928_init_devices);
