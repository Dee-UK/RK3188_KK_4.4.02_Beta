/* linux/arch/arm/mach-rk30/clock_data.c
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
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/hardirq.h>
#include <plat/efuse.h>
#include <mach/cru.h>
#include <mach/iomux.h>
#include <mach/clock.h>
#include <mach/pmu.h>
#include <mach/dvfs.h>
#include <mach/ddr.h>
#include <mach/cpu.h>

#define MHZ			(1000UL * 1000UL)
#define KHZ			(1000UL)
#define CLK_LOOPS_JIFFY_REF 11996091ULL
#define CLK_LOOPS_RATE_REF (1200UL) //Mhz
#define CLK_LOOPS_RECALC(new_rate)  div_u64(CLK_LOOPS_JIFFY_REF*(new_rate),CLK_LOOPS_RATE_REF*MHZ)
void rk30_clk_dump_regs(void);
static int flag_uboot_display = 0;
//flags bit
//has extern 27mhz
#define CLK_FLG_EXT_27MHZ 			(1<<0)
//max i2s rate
#define CLK_FLG_MAX_I2S_12288KHZ 	(1<<1)
#define CLK_FLG_MAX_I2S_22579_2KHZ 	(1<<2)
#define CLK_FLG_MAX_I2S_24576KHZ 	(1<<3)
#define CLK_FLG_MAX_I2S_49152KHZ 	(1<<4)
//uart 1m\3m
#define CLK_FLG_UART_1_3M			(1<<5)

#define ARCH_RK31

struct apll_clk_set {
	unsigned long	rate;
	u32	pllcon0;
	u32	pllcon1;
	u32	pllcon2; //nb=bwadj+1;0:11;nb=nf/2
	u32 	rst_dly;//us
	u32	clksel0;
	u32	clksel1;
	unsigned long	lpj;
};
struct pll_clk_set {
	unsigned long	rate;
	u32	pllcon0;
	u32	pllcon1;
	u32	pllcon2; //nb=bwadj+1;0:11;nb=nf/2
	unsigned long	rst_dly;//us
};

#define SET_PLL_DATA(_pll_id,_table) \
{\
	.id=(_pll_id),\
	.table=(_table),\
}


#define _PLL_SET_CLKS(_mhz, nr, nf, no) \
{ \
	.rate	= (_mhz) * KHZ, \
	.pllcon0 = PLL_CLKR_SET(nr)|PLL_CLKOD_SET(no), \
	.pllcon1 = PLL_CLKF_SET(nf),\
	.pllcon2 = PLL_CLK_BWADJ_SET(nf >> 1),\
	.rst_dly=((nr*500)/24+1),\
}


#define _APLL_SET_LPJ(_mhz) \
	.lpj= (CLK_LOOPS_JIFFY_REF * _mhz)/CLK_LOOPS_RATE_REF


#define _APLL_SET_CLKS(_mhz, nr, nf, no, _periph_div, _axi_core_div,\
		_axi_div,_ahb_div, _apb_div,_ahb2apb) \
{ \
	.rate	= _mhz * MHZ, \
	.pllcon0 = PLL_CLKR_SET(nr) | PLL_CLKOD_SET(no), \
	.pllcon1 = PLL_CLKF_SET(nf),\
	.pllcon2 = PLL_CLK_BWADJ_SET(nf >> 1),\
	.clksel0 = CORE_PERIPH_W_MSK | CORE_PERIPH_##_periph_div,\
	.clksel1 = CORE_ACLK_W_MSK | CORE_ACLK_##_axi_core_div,\
	_APLL_SET_LPJ(_mhz),\
	.rst_dly=((nr*500)/24+1),\
}

#define CRU_DIV_SET(mask,shift,max) \
	.div_mask=(mask),\
.div_shift=(shift),\
.div_max=(max)


#define CRU_SRC_SET(mask,shift ) \
	.src_mask=(mask),\
.src_shift=(shift)

#define CRU_PARENTS_SET(parents_array) \
	.parents=(parents_array),\
.parents_num=ARRAY_SIZE((parents_array))

#define CRU_GATE_MODE_SET(_func,_IDX) \
	.mode=_func,\
.gate_idx=(_IDX)

struct clk_src_sel {
	struct clk	*parent;
	u8	value;//crt bit
	u8	flag;
	//selgate
};

#define GATE_CLK(NAME,PARENT,ID) \
	static struct clk clk_##NAME = { \
		.name		= #NAME, \
		.parent		= &PARENT, \
		.mode		= gate_mode, \
		.gate_idx	= CLK_GATE_##ID, \
	}
#ifdef RK30_CLK_OFFBOARD_TEST
u32 TEST_GRF_REG[0x240];
u32 TEST_CRU_REG[0x240];
#define cru_readl(offset)	(TEST_CRU_REG[offset/4])

u32 cru_writel_is_pr(u32 offset)
{
	return (offset == 0x4000);
}
void cru_writel(u32 v, u32 offset)
{

	u32 mask_v = v >> 16;
	TEST_CRU_REG[offset/4] &= (~mask_v);

	v &= (mask_v);

	TEST_CRU_REG[offset/4] |= v;
	TEST_CRU_REG[offset/4] &= 0x0000ffff;

	if(cru_writel_is_pr(offset)) {
		CLKDATA_DBG("cru w offset=%d,set=%x,reg=%x\n", offset, v, TEST_CRU_REG[offset/4]);

	}

}
void cru_writel_i2s(u32 v, u32 offset)
{
	TEST_CRU_REG[offset/4] = v;
}
#define cru_writel_frac(v,offset) cru_writel_i2s((v),(offset))

#define regfile_readl(offset)	(0xffffffff)
//#define pmu_readl(offset)	   readl(RK30_GRF_BASE + offset)
void rk30_clkdev_add(struct clk_lookup *cl);
#else
#define regfile_readl(offset)	readl_relaxed(RK30_GRF_BASE + offset)
#define regfile_writel(v, offset) do { writel_relaxed(v, RK30_GRF_BASE + offset); dsb(); } while (0)
#define cru_readl(offset)	readl_relaxed(RK30_CRU_BASE + offset)
#define cru_writel(v, offset)	do { writel_relaxed(v, RK30_CRU_BASE + offset); dsb(); } while (0)

#define cru_writel_frac(v,offset) cru_writel((v),(offset))
#endif

//#define DEBUG
#ifdef DEBUG
#define CLKDATA_DBG(fmt, args...) printk(KERN_DEBUG "CLKDATA_DBG:\t"fmt, ##args)
#define CLKDATA_LOG(fmt, args...) printk(KERN_INFO "CLKDATA_LOG:\t"fmt, ##args)
//#define CLKDATA_DBG(fmt, args...) printk("CLKDATA_DBG:\t"fmt, ##args)
//#define CLKDATA_LOG(fmt, args...) printk("CLKDATA_LOG:\t"fmt, ##args)
#else
#define CLKDATA_DBG(fmt, args...) do {} while(0)
#define CLKDATA_LOG(fmt, args...) do {} while(0)
#endif
#define CLKDATA_ERR(fmt, args...) printk(KERN_ERR "CLKDATA_ERR:\t"fmt, ##args)
#define CLKDATA_WARNING(fmt, args...) printk("CLKDATA_WANING:\t"fmt, ##args)


#define get_cru_bits(con,mask,shift)\
	((cru_readl((con)) >> (shift)) & (mask))

#define set_cru_bits_w_msk(val,mask,shift,con)\
	cru_writel(((mask)<<(shift+16))|((val)<<(shift)),(con))


#define PLLS_IN_NORM(pll_id) (((cru_readl(CRU_MODE_CON)&PLL_MODE_MSK(pll_id))==(PLL_MODE_NORM(pll_id)&PLL_MODE_MSK(pll_id)))\
		&&!(cru_readl(PLL_CONS(pll_id,3))&PLL_BYPASS))


static u32 rk30_clock_flags = 0;
static struct clk codec_pll_clk;
static struct clk general_pll_clk;
static struct clk arm_pll_clk;
static unsigned long lpj_gpll;
static unsigned int __initdata armclk = 504 * MHZ;


/************************clk recalc div rate*********************************/

//for free div
static unsigned long clksel_recalc_div(struct clk *clk)
{
	u32 div = get_cru_bits(clk->clksel_con, clk->div_mask, clk->div_shift) + 1;

	unsigned long rate = clk->parent->rate / div;
	pr_debug("%s new clock rate is %lu (div %u)\n", clk->name, rate, div);
	return rate;
}

//for div 1 2 4 2^n
static unsigned long clksel_recalc_shift(struct clk *clk)
{
	u32 shift = get_cru_bits(clk->clksel_con, clk->div_mask, clk->div_shift);
	unsigned long rate = clk->parent->rate >> shift;
	pr_debug("%s new clock rate is %lu (shift %u)\n", clk->name, rate, shift);
	return rate;
}


static unsigned long clksel_recalc_shift_2(struct clk *clk)
{
	u32 shift = get_cru_bits(clk->clksel_con, clk->div_mask, clk->div_shift) + 1;
	unsigned long rate = clk->parent->rate >> shift;
	pr_debug("%s new clock rate is %lu (shift %u)\n", clk->name, rate, shift);
	return rate;
}

static unsigned long clksel_recalc_parent_rate(struct clk *clk)
{
	unsigned long rate = clk->parent->rate;
	pr_debug("%s new clock rate is %lu\n", clk->name, rate);
	return rate;
}
/********************************set div rate***********************************/

//for free div
static int clksel_set_rate_freediv(struct clk *clk, unsigned long rate)
{
	u32 div;
	for (div = 0; div < clk->div_max; div++) {
		u32 new_rate = clk->parent->rate / (div + 1);
		if (new_rate <= rate) {
			set_cru_bits_w_msk(div, clk->div_mask, clk->div_shift, clk->clksel_con);
			//clk->rate = new_rate;
			pr_debug("clksel_set_rate_freediv for clock %s to rate %ld (div %d)\n", 
					clk->name, rate, div + 1);
			return 0;
		}
	}
	return -ENOENT;
}

//for div 1 2 4 2^n
static int clksel_set_rate_shift(struct clk *clk, unsigned long rate)
{
	u32 shift;
	for (shift = 0; (1 << shift) < clk->div_max; shift++) {
		u32 new_rate = clk->parent->rate >> shift;
		if (new_rate <= rate) {
			set_cru_bits_w_msk(shift, clk->div_mask, clk->div_shift, clk->clksel_con);
			clk->rate = new_rate;
			pr_debug("clksel_set_rate_shift for clock %s to rate %ld (shift %d)\n", 
					clk->name, rate, shift);
			return 0;
		}
	}
	return -ENOENT;
}

//for div 2 4 2^n
static int clksel_set_rate_shift_2(struct clk *clk, unsigned long rate)
{
	u32 shift;

	for (shift = 1; (1 << shift) < clk->div_max; shift++) {
		u32 new_rate = clk->parent->rate >> shift;
		if (new_rate <= rate) {
			set_cru_bits_w_msk(shift - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
			clk->rate = new_rate;
			pr_debug("clksel_set_rate_shift for clock %s to rate %ld (shift %d)\n", 
					clk->name, rate, shift);
			return 0;
		}
	}
	return -ENOENT;
}

//for div 1 2 4 2*n
static int clksel_set_rate_even(struct clk *clk, unsigned long rate)
{
	u32 div = 0, new_rate = 0;
	for (div = 1; div < clk->div_max; div++) {
		if (div >= 3 && div % 2 != 0)
			continue;
		new_rate = clk->parent->rate / div;
		if (new_rate <= rate) {
			set_cru_bits_w_msk(div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
			clk->rate = new_rate;
			pr_debug("%s for clock %s to rate %ld (even div = %d)\n", 
					__func__, clk->name, rate, div);
			return 0;
		}
	}
	return -ENOENT;
}
static u32 clk_get_freediv(unsigned long rate_out, unsigned long rate , u32 div_max)
{
	u32 div;
	unsigned long new_rate;
	for (div = 0; div < div_max; div++) {
		new_rate = rate / (div + 1);
		if (new_rate <= rate_out) {
			return div + 1;
		}
	}
	return div_max ? div_max : 1;
}
static u32 clk_get_evendiv(unsigned long rate_out, unsigned long rate , u32 div_max)
{
	u32 div;
	unsigned long new_rate;
	for (div = 1; div < div_max; div += 2) {
		new_rate = rate / (div + 1);
		if (new_rate <= rate_out) {
			return div + 1;
		}
	}
	return div_max ? div_max : 1;
}
struct clk *get_freediv_parents_div(struct clk *clk, unsigned long rate, u32 *div_out) {
	u32 div[2] = {0, 0};
	unsigned long new_rate[2] = {0, 0};
	u32 i;

	if(clk->rate == rate)
		return clk->parent;
	for(i = 0; i < 2; i++) {
		div[i] = clk_get_freediv(rate, clk->parents[i]->rate, clk->div_max);
		new_rate[i] = clk->parents[i]->rate / div[i];
		if(new_rate[i] == rate) {
			*div_out = div[i];
			return clk->parents[i];
		}
	}
	if(new_rate[0] < new_rate[1])
		i = 1;
	else
		i = 0;
	*div_out = div[i];
	return clk->parents[i];
}
struct clk *get_evendiv_parents_div(struct clk *clk, unsigned long rate, u32 *div_out) {
	u32 div[2] = {0, 0};
	unsigned long new_rate[2] = {0, 0};
	u32 i;

	if(clk->rate == rate)
		return clk->parent;
	for(i = 0; i < 2; i++) {
		div[i] = clk_get_evendiv(rate, clk->parents[i]->rate, clk->div_max);
		new_rate[i] = clk->parents[i]->rate / div[i];
		if(new_rate[i] == rate) {
			*div_out = div[i];
			return clk->parents[i];
		}
	}
	if(new_rate[0] < new_rate[1])
		i = 1;
	else
		i = 0;
	*div_out = div[i];
	return clk->parents[i];
}

static int clkset_rate_freediv_autosel_parents(struct clk *clk, unsigned long rate)
{
	struct clk *p_clk;
	u32 div, old_div;
	int ret = 0;
	if(clk->rate == rate)
		return 0;
	p_clk = get_freediv_parents_div(clk, rate, &div);

	if(!p_clk)
		return -ENOENT;

	CLKDATA_DBG("%s %lu,form %s\n", clk->name, rate, p_clk->name);
	if (clk->parent != p_clk) {
		old_div = CRU_GET_REG_BITS_VAL(cru_readl(clk->clksel_con), clk->div_shift, clk->div_mask) + 1;

		if(div > old_div) {
			set_cru_bits_w_msk(div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
		}
		ret = clk_set_parent_nolock(clk, p_clk);
		if(ret) {
			CLKDATA_ERR("%s can't set %lu,reparent err\n", clk->name, rate);
			return -ENOENT;
		}
	}
	//set div
	set_cru_bits_w_msk(div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
	return 0;
}
static int clkset_rate_evendiv_autosel_parents(struct clk *clk, unsigned long rate)
{
	struct clk *p_clk;
	u32 div, old_div;
	int ret = 0;
	if(clk->rate == rate)
		return 0;
	p_clk = get_evendiv_parents_div(clk, rate, &div);

	if(!p_clk)
		return -ENOENT;

	CLKDATA_DBG("%s %lu,form %s\n", clk->name, rate, p_clk->name);
	if (clk->parent != p_clk) {
		old_div = CRU_GET_REG_BITS_VAL(cru_readl(clk->clksel_con), clk->div_shift, clk->div_mask) + 1;

		if(div > old_div) {
			set_cru_bits_w_msk(div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
		}
		ret = clk_set_parent_nolock(clk, p_clk);
		if(ret) {
			CLKDATA_ERR("%s can't set %lu,reparent err\n", clk->name, rate);
			return -ENOENT;
		}
	}
	//set div
	set_cru_bits_w_msk(div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
	return 0;
}

//rate==div rate //hdmi
static int clk_freediv_autosel_parents_set_fixed_rate(struct clk *clk, unsigned long rate)
{
	struct clk *p_clk;
	u32 div, old_div;
	int ret;
	p_clk = get_freediv_parents_div(clk, rate, &div);

	if(!p_clk)
		return -ENOENT;

	if((p_clk->rate / div) != rate || (p_clk->rate % div))
		return -ENOENT;

	if (clk->parent != p_clk) {
		old_div = CRU_GET_REG_BITS_VAL(cru_readl(clk->clksel_con),
				clk->div_shift, clk->div_mask) + 1;
		if(div > old_div) {
			set_cru_bits_w_msk(div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
		}
		ret = clk_set_parent_nolock(clk, p_clk);
		if (ret) {
			CLKDATA_DBG("%s can't get rate%lu,reparent err\n", clk->name, rate);
			return ret;
		}
	}
	//set div
	set_cru_bits_w_msk(div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
	return 0;
}

/***************************round********************************/

static long clksel_freediv_round_rate(struct clk *clk, unsigned long rate)
{
	return clk->parent->rate / clk_get_freediv(rate, clk->parent->rate, clk->div_max);
}

static long clk_freediv_round_autosel_parents_rate(struct clk *clk, unsigned long rate)
{
	u32 div;
	struct clk *p_clk;
	if(clk->rate == rate)
		return clk->rate;
	p_clk = get_freediv_parents_div(clk, rate, &div);
	if(!p_clk)
		return 0;
	return p_clk->rate / div;
}

/**************************************others seting************************************/

static struct clk *clksel_get_parent(struct clk *clk) {
	return clk->parents[(cru_readl(clk->clksel_con) >> clk->src_shift) & clk->src_mask];
}
static int clksel_set_parent(struct clk *clk, struct clk *parent)
{
	u32 i;
	if (unlikely(!clk->parents))
		return -EINVAL;
	for (i = 0; (i < clk->parents_num); i++) {
		if (clk->parents[i] != parent)
			continue;
		set_cru_bits_w_msk(i, clk->src_mask, clk->src_shift, clk->clksel_con);
		return 0;
	}
	return -EINVAL;
}

static int gate_mode(struct clk *clk, int on)
{
	int idx = clk->gate_idx;
	if (idx >= CLK_GATE_MAX)
		return -EINVAL;
	if(on) {
		cru_writel(CLK_GATE_W_MSK(idx) | CLK_UN_GATE(idx), CLK_GATE_CLKID_CONS(idx));
		//CLKDATA_DBG("un gate id=%d %s(%x),con %x\n",idx,clk->name,
		//	CLK_GATE_W_MSK(idx)|CLK_UN_GATE(idx),CLK_GATE_CLKID_CONS(idx));
	} else {
		cru_writel(CLK_GATE_W_MSK(idx) | CLK_GATE(idx), CLK_GATE_CLKID_CONS(idx));
		//	CLKDATA_DBG("gate id=%d %s(%x),con%x\n",idx,clk->name,
		//	CLK_GATE_W_MSK(idx)|CLK_GATE(idx),CLK_GATE_CLKID_CONS(idx));
	}
	return 0;
}
/*****************************frac set******************************************/

static unsigned long clksel_recalc_frac(struct clk *clk)
{
	unsigned long rate;
	u64 rate64;
	u32 r = cru_readl(clk->clksel_con), numerator, denominator;
	if (r == 0) // FPGA ?
		return clk->parent->rate;
	numerator = r >> 16;
	denominator = r & 0xFFFF;
	rate64 = (u64)clk->parent->rate * numerator;
	do_div(rate64, denominator);
	rate = rate64;
	pr_debug("%s new clock rate is %lu (frac %u/%u)\n", clk->name, rate, numerator, denominator);
	return rate;
}

static u32 clk_gcd(u32 numerator, u32 denominator)
{
	u32 a, b;

	if (!numerator || !denominator)
		return 0;
	if (numerator > denominator) {
		a = numerator;
		b = denominator;
	} else {
		a = denominator;
		b = numerator;
	}
	while (b != 0) {
		int r = b;
		b = a % b;
		a = r;
	}

	return a;
}

static int frac_div_get_seting(unsigned long rate_out, unsigned long rate,
		u32 *numerator, u32 *denominator)
{
	u32 gcd_vl;
	gcd_vl = clk_gcd(rate, rate_out);
	CLKDATA_DBG("frac_get_seting rate=%lu,parent=%lu,gcd=%d\n", rate_out, rate, gcd_vl);

	if (!gcd_vl) {
		CLKDATA_ERR("gcd=0, i2s frac div is not be supported\n");
		return -ENOENT;
	}

	*numerator = rate_out / gcd_vl;
	*denominator = rate / gcd_vl;

	CLKDATA_DBG("frac_get_seting numerator=%d,denominator=%d,times=%d\n",
			*numerator, *denominator, *denominator / *numerator);

	if (*numerator > 0xffff || *denominator > 0xffff ||
			(*denominator / (*numerator)) < 20) {
		CLKDATA_ERR("can't get a available nume and deno\n");
		return -ENOENT;
	}

	return 0;

}
/* *********************pll **************************/

#define rk30_clock_udelay(a) udelay(a);

/*********************pll lock status**********************************/
//#define GRF_SOC_CON0       0x15c
static void pll_wait_lock(int pll_idx)
{
	u32 pll_state[4] = {1, 0, 2, 3};
	u32 bit = 0x20u << pll_state[pll_idx];
	int delay = 24000000;
	while (delay > 0) {
		if (regfile_readl(GRF_SOC_STATUS0) & bit)
			break;
		delay--;
	}
	if (delay == 0) {
		CLKDATA_ERR("PLL_ID=%d\npll_con0=%08x\npll_con1=%08x\npll_con2=%08x\npll_con3=%08x\n", pll_idx,
				cru_readl(PLL_CONS(pll_idx, 0)),
				cru_readl(PLL_CONS(pll_idx, 1)),
				cru_readl(PLL_CONS(pll_idx, 2)),
				cru_readl(PLL_CONS(pll_idx, 3)));

		CLKDATA_ERR("wait pll bit 0x%x time out!\n", bit);
		while(1);
	}
}



/***************************pll function**********************************/
static unsigned long pll_clk_recalc(u32 pll_id, unsigned long parent_rate)
{
	unsigned long rate;

	if (PLLS_IN_NORM(pll_id)) {
		u32 pll_con0 = cru_readl(PLL_CONS(pll_id, 0));
		u32 pll_con1 = cru_readl(PLL_CONS(pll_id, 1));


		u64 rate64 = (u64)parent_rate * PLL_NF(pll_con1);

		/*
		   CLKDATA_DBG("selcon con0(%x) %x,con1(%x)%x, rate64 %llu\n",PLL_CONS(pll_id,0),pll_con0
		   ,PLL_CONS(pll_id,1),pll_con1, rate64);
		   */


		//CLKDATA_DBG("pll id=%d con0=%x,con1=%x,parent=%lu\n",pll_id,pll_con0,pll_con1,parent_rate);
		//CLKDATA_DBG("first pll id=%d rate is %lu (NF %d NR %d NO %d)\n",
		//pll_id, rate, PLL_NF(pll_con1), PLL_NR(pll_con0), 1 << PLL_NO(pll_con0));

		do_div(rate64, PLL_NR(pll_con0));
		do_div(rate64, PLL_NO(pll_con0));

		rate = rate64;
		/*
		   CLKDATA_DBG("pll_clk_recalc id=%d rate=%lu (NF %d NR %d NO %d) rate64=%llu\n",
		   pll_id, rate, PLL_NF(pll_con1), PLL_NR(pll_con0),PLL_NO(pll_con0), rate64);
		   */
	} else {
		rate = parent_rate;
		CLKDATA_DBG("pll_clk_recalc id=%d rate=%lu by pass mode\n", pll_id, rate);
	}
	return rate;
}
static unsigned long plls_clk_recalc(struct clk *clk)
{
	return pll_clk_recalc(clk->pll->id, clk->parent->rate);
}
static unsigned long plus_pll_clk_recalc(u32 pll_id, unsigned long parent_rate)
{
	unsigned long rate;

	if (PLLS_IN_NORM(pll_id)) {
		u32 pll_con0 = cru_readl(PLL_CONS(pll_id, 0));
		u32 pll_con1 = cru_readl(PLL_CONS(pll_id, 1));

		u64 rate64 = (u64)parent_rate * PLUS_PLL_NF(pll_con1);

		do_div(rate64, PLUS_PLL_NR(pll_con0));
		do_div(rate64, PLUS_PLL_NO(pll_con0));

		rate = rate64;
	} else {
		rate = parent_rate;
		CLKDATA_DBG("pll_clk_recalc id=%d rate=%lu by pass mode\n", pll_id, rate);
	}
	return rate;
}
static unsigned long plus_plls_clk_recalc(struct clk *clk)
{
	DVFS_DBG("%s: for rk3188 plus\n", __func__);
	return plus_pll_clk_recalc(clk->pll->id, clk->parent->rate);
}

static int pll_clk_set_rate(struct pll_clk_set *clk_set, u8 pll_id)
{
	//enter slowmode
	cru_writel(PLL_MODE_SLOW(pll_id), CRU_MODE_CON);
	cru_writel((0x1<<(16+1))|(0x1<<1), PLL_CONS(pll_id, 3));
	dsb();
	dsb();
	dsb();
	dsb();
	dsb();
	dsb();
	cru_writel(clk_set->pllcon0, PLL_CONS(pll_id, 0));
	cru_writel(clk_set->pllcon1, PLL_CONS(pll_id, 1));

	rk30_clock_udelay(1);
	cru_writel((0x1<<(16+1)), PLL_CONS(pll_id, 3));

	pll_wait_lock(pll_id);

	//return form slow
	cru_writel(PLL_MODE_NORM(pll_id), CRU_MODE_CON);

	/*
	   CLKDATA_ERR("pll reg id=%d,con0=%x,con1=%x,mode=%x\n",pll_id,
	   cru_readl(PLL_CONS(pll_id,0)),(PLL_CONS(pll_id,1)),cru_readl(CRU_MODE_CON));
	   */

	return 0;
}

static int plus_pll_clk_set_rate(struct pll_clk_set *clk_set, u8 pll_id)
{
	//enter slowmode
	cru_writel(PLL_MODE_SLOW(pll_id), CRU_MODE_CON);

	//enter rest
	cru_writel(PLL_RESET_W_MSK | PLL_RESET, PLL_CONS(pll_id, 3));
	cru_writel(clk_set->pllcon0, PLL_CONS(pll_id, 0));
	cru_writel(clk_set->pllcon1, PLL_CONS(pll_id, 1));
	cru_writel(clk_set->pllcon2, PLL_CONS(pll_id, 2));
	rk30_clock_udelay(5);
	
	//return form rest
	cru_writel(PLL_RESET_W_MSK | PLL_RESET_RESUME, PLL_CONS(pll_id, 3));
	
	//wating lock state
	rk30_clock_udelay(clk_set->rst_dly);
	pll_wait_lock(pll_id);

	//return form slow
	cru_writel(PLL_MODE_NORM(pll_id), CRU_MODE_CON);
	
	return 0;
}

static int gpll_clk_set_rate(struct clk *c, unsigned long rate)
{
	struct _pll_data *pll_data = c->pll;
	struct pll_clk_set *clk_set = (struct pll_clk_set *)pll_data->table;

	while(clk_set->rate) {
		if (clk_set->rate == rate) {
			break;
		}
		clk_set++;
	}
	if(clk_set->rate == rate) {
		pll_clk_set_rate(clk_set, pll_data->id);
		lpj_gpll = CLK_LOOPS_RECALC(rate);
	} else {
		CLKDATA_ERR("gpll is no corresponding rate=%lu\n", rate);
		return -1;
	}
	return 0;
}

static int plus_gpll_clk_set_rate(struct clk *c, unsigned long rate)
{
	struct _pll_data *pll_data = c->pll;
	struct pll_clk_set *clk_set = (struct pll_clk_set *)pll_data->table;
	DVFS_DBG("%s: for rk3188 plus\n", __func__);

	while(clk_set->rate) {
		if (clk_set->rate == rate) {
			break;
		}
		clk_set++;
	}
	if(clk_set->rate == rate) {
		plus_pll_clk_set_rate(clk_set, pll_data->id);
		lpj_gpll = CLK_LOOPS_RECALC(rate);
	} else {
		CLKDATA_ERR("gpll is no corresponding rate=%lu\n", rate);
		return -1;
	}
	return 0;
}
#define PLL_FREF_MIN (183*KHZ)
#define PLL_FREF_MAX (1500*MHZ)

#define PLL_FVCO_MIN (300*MHZ)
#define PLL_FVCO_MAX (1500*MHZ)

#define PLL_FOUT_MIN (18750*KHZ)
#define PLL_FOUT_MAX (1500*MHZ)

#define PLL_NF_MAX (65536)
#define PLL_NR_MAX (64)
#define PLL_NO_MAX (64)

static int pll_clk_get_set(unsigned long fin_hz, unsigned long fout_hz, u32 *clk_nr, u32 *clk_nf, u32 *clk_no)
{
	u32 nr, nf, no, nonr;
	u32 n;
	u32 YFfenzi;
	u32 YFfenmu;
	unsigned long fref, fvco, fout;
	u32 gcd_val = 0;

	CLKDATA_DBG("pll_clk_get_set fin=%lu,fout=%lu\n", fin_hz, fout_hz);
	if(!fin_hz || !fout_hz || fout_hz == fin_hz)
		return 0;
	gcd_val = clk_gcd(fin_hz, fout_hz);
	YFfenzi = fout_hz / gcd_val;
	YFfenmu = fin_hz / gcd_val;

	for(n = 1;; n++) {
		nf = YFfenzi * n;
		nonr = YFfenmu * n;
		if(nf > PLL_NF_MAX || nonr > (PLL_NO_MAX * PLL_NR_MAX))
			break;
		for(no = 1; no <= PLL_NO_MAX; no++) {
			if(!(no == 1 || !(no % 2)))
				continue;

			if(nonr % no)
				continue;
			nr = nonr / no;

			if(nr > PLL_NR_MAX) //PLL_NR_MAX
				continue;

			fref = fin_hz / nr;
			if(fref < PLL_FREF_MIN || fref > PLL_FREF_MAX)
				continue;

			fvco = (fin_hz / nr) * nf;
			if(fvco < PLL_FVCO_MIN || fvco > PLL_FVCO_MAX)
				continue;
			fout = fvco / no;
			if(fout < PLL_FOUT_MIN || fout > PLL_FOUT_MAX)
				continue;
			*clk_nr = nr;
			*clk_no = no;
			*clk_nf = nf;
			return 1;

		}

	}
	return 0;
}

static int pll_clk_mode(struct clk *clk, int on)
{
	u8 pll_id = clk->pll->id;
	u32 nr = PLL_NR(cru_readl(PLL_CONS(pll_id, 0)));
	u32 dly = (nr * 500) / 24 + 1;
	CLKDATA_DBG("pll_mode %s(%d)\n", clk->name, on);
	if (on) {
		cru_writel(PLL_PWR_ON | PLL_PWR_DN_W_MSK, PLL_CONS(pll_id, 3));
		rk30_clock_udelay(dly);
		pll_wait_lock(pll_id);
		cru_writel(PLL_MODE_NORM(pll_id), CRU_MODE_CON);
	} else {
		cru_writel(PLL_MODE_SLOW(pll_id), CRU_MODE_CON);
		cru_writel(PLL_PWR_DN | PLL_PWR_DN_W_MSK, PLL_CONS(pll_id, 3));
	}
	return 0;
}

static int cpll_clk_set_rate(struct clk *c, unsigned long rate)
{
	struct _pll_data *pll_data = c->pll;
	struct pll_clk_set *clk_set = (struct pll_clk_set *)pll_data->table;
	struct pll_clk_set temp_clk_set;
	u32 clk_nr, clk_nf, clk_no;

	if(rate == 24 * MHZ) {	
		cru_writel(PLL_MODE_SLOW(pll_data->id), CRU_MODE_CON);
		cru_writel((0x1 << (16+1)) | (0x1<<1), PLL_CONS(pll_data->id, 3));
		return 0;
	}
	while(clk_set->rate) {
		if (clk_set->rate == rate) {
			break;
		}
		clk_set++;
	}
	if(clk_set->rate == rate) {
		CLKDATA_DBG("cpll get a rate\n");
		pll_clk_set_rate(clk_set, pll_data->id);

	} else {
		CLKDATA_DBG("cpll get auto calc a rate\n");
		if(pll_clk_get_set(c->parent->rate, rate, &clk_nr, &clk_nf, &clk_no) == 0) {
			pr_err("cpll auto set rate error\n");
			return -ENOENT;
		}
		CLKDATA_DBG("cpll auto ger rate set nr=%d,nf=%d,no=%d\n", clk_nr, clk_nf, clk_no);
		temp_clk_set.pllcon0 = PLL_CLKR_SET(clk_nr) | PLL_CLKOD_SET(clk_no);
		temp_clk_set.pllcon1 = PLL_CLKF_SET(clk_nf);
		temp_clk_set.rst_dly = (clk_nr * 500) / 24 + 1;
		pll_clk_set_rate(&temp_clk_set, pll_data->id);

	}
	return 0;
}
static int plus_cpll_clk_set_rate(struct clk *c, unsigned long rate)
{
	struct _pll_data *pll_data = c->pll;
	struct pll_clk_set *clk_set = (struct pll_clk_set *)pll_data->table;
	struct pll_clk_set temp_clk_set;
	u32 clk_nr, clk_nf, clk_no;
	DVFS_DBG("%s: for rk3188 plus\n", __func__);

	while(clk_set->rate) {
		if (clk_set->rate == rate) {
			break;
		}
		clk_set++;
	}
	if(clk_set->rate == rate) {
		CLKDATA_DBG("cpll get a rate\n");
		plus_pll_clk_set_rate(clk_set, pll_data->id);

	} else {
		CLKDATA_DBG("cpll get auto calc a rate\n");
		if(pll_clk_get_set(c->parent->rate, rate, &clk_nr, &clk_nf, &clk_no) == 0) {
			pr_err("cpll auto set rate error\n");
			return -ENOENT;
		}
		CLKDATA_DBG("cpll auto ger rate set nr=%d,nf=%d,no=%d\n", clk_nr, clk_nf, clk_no);
		temp_clk_set.pllcon0 = PLL_CLKR_SET(clk_nr) | PLL_CLKOD_SET(clk_no);
		temp_clk_set.pllcon1 = PLL_CLKF_SET(clk_nf);
		temp_clk_set.rst_dly = (clk_nr * 500) / 24 + 1;
		plus_pll_clk_set_rate(&temp_clk_set, pll_data->id);

	}
	return 0;
}


/* ******************fixed input clk ***********************************************/
static struct clk xin24m = {
	.name		= "xin24m",
	.rate		= 24 * MHZ,
	.flags		= RATE_FIXED,
};

static struct clk clk_12m = {
	.name		= "clk_12m",
	.parent		= &xin24m,
	.rate		= 12 * MHZ,
	.flags		= RATE_FIXED,
};

/************************************pll func***************************/
static const struct apll_clk_set *arm_pll_clk_get_best_pll_set(unsigned long rate,
		struct apll_clk_set *tables) {
	const struct apll_clk_set *ps, *pt;

	/* find the arm_pll we want. */
	ps = pt = tables;
	while (pt->rate) {
		if (pt->rate == rate) {
			ps = pt;
			break;
		}
		// we are sorted, and ps->rate > pt->rate.
		if ((pt->rate > rate || (rate - pt->rate < ps->rate - rate)))
			ps = pt;
		if (pt->rate < rate)
			break;
		pt++;
	}
	//CLKDATA_DBG("arm pll best rate=%lu\n",ps->rate);
	return ps;
}
static long arm_pll_clk_round_rate(struct clk *clk, unsigned long rate)
{
	return arm_pll_clk_get_best_pll_set(rate, clk->pll->table)->rate;
}
#if 1
struct arm_clks_div_set {
	u32 rate;
	u32	clksel0;
	u32	clksel1;
};

#define _arm_clks_div_set(_mhz,_periph_div,_axi_div,_ahb_div, _apb_div,_ahb2apb) \
{ \
	.rate    =_mhz,\
	.clksel0 = CORE_PERIPH_W_MSK|CORE_PERIPH_##_periph_div,\
	.clksel1 = CORE_ACLK_W_MSK|CORE_ACLK_##_axi_div\
	|ACLK_HCLK_W_MSK|ACLK_HCLK_##_ahb_div\
	|ACLK_PCLK_W_MSK|ACLK_PCLK_##_apb_div\
	|AHB2APB_W_MSK	|AHB2APB_##_ahb2apb,\
}
struct arm_clks_div_set arm_clk_div_tlb[] = {
	_arm_clks_div_set(50 ,  2, 11, 11, 11, 11),//25,50,50,50,50
	_arm_clks_div_set(100 , 4, 11, 21, 21, 11),//25,100,50,50,50
	_arm_clks_div_set(150 , 4, 11, 21, 21, 11),//37,150,75,75,75
	_arm_clks_div_set(200 , 8, 21, 21, 21, 11),//25,100,50,50,50
	_arm_clks_div_set(300 , 8, 21, 21, 21, 11),//37,150,75,75,75
	_arm_clks_div_set(400 , 8, 21, 21, 41, 21),//50,200,100,50,50
	_arm_clks_div_set(0 ,   2, 11, 11, 11, 11),//25,50,50,50,50
};
struct arm_clks_div_set *arm_clks_get_div(u32 rate) {
	int i = 0;
	for(i = 0; arm_clk_div_tlb[i].rate != 0; i++) {
		if(arm_clk_div_tlb[i].rate >= rate)
			return &arm_clk_div_tlb[i];
	}
	return NULL;
}

#endif

static int arm_pll_clk_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags;
	const struct apll_clk_set *ps;
	u32 pll_id = clk->pll->id;
	u32 temp_div;
	u32 old_aclk_div = 0, new_aclk_div;

	ps = arm_pll_clk_get_best_pll_set(rate, (struct apll_clk_set *)clk->pll->table);

	old_aclk_div = GET_CORE_ACLK_VAL(cru_readl(CRU_CLKSELS_CON(1))&CORE_ACLK_MSK);
	new_aclk_div = GET_CORE_ACLK_VAL(ps->clksel1 & CORE_ACLK_MSK);

	CLKDATA_LOG("apll will set rate(%lu) tlb con(%x,%x,%x),sel(%x,%x)\n",
			ps->rate, ps->pllcon0, ps->pllcon1, ps->pllcon2, ps->clksel0, ps->clksel1);

	if(general_pll_clk.rate > clk->rate) {
		temp_div = clk_get_freediv(clk->rate, general_pll_clk.rate, 10);
	} else {
		temp_div = 1;
	}

	// ungating cpu gpll path
	//cru_writel(CLK_GATE_W_MSK(CLK_GATE_CPU_GPLL_PATH) | CLK_UN_GATE(CLK_GATE_CPU_GPLL_PATH), 
	//	CLK_GATE_CLKID_CONS(CLK_GATE_CPU_GPLL_PATH));

	local_irq_save(flags);
	//div arm clk for gpll

	cru_writel(CORE_CLK_DIV_W_MSK|CORE_CLK_DIV(temp_div), CRU_CLKSELS_CON(0));
	cru_writel(CORE_SEL_PLL_W_MSK|CORE_SEL_GPLL, CRU_CLKSELS_CON(0));

	loops_per_jiffy = lpj_gpll / temp_div;
	smp_wmb();

	/*if core src don't select gpll ,apll neet to enter slow mode */
	//cru_writel(PLL_MODE_SLOW(APLL_ID), CRU_MODE_CON);


	cru_writel((0x1<<(16+1))|(0x1<<1), PLL_CONS(pll_id, 3));
	dsb();
	dsb();
	dsb();
	dsb();
	dsb();
	dsb();
	cru_writel(ps->pllcon0, PLL_CONS(pll_id, 0));
	cru_writel(ps->pllcon1, PLL_CONS(pll_id, 1));

	rk30_clock_udelay(1);
	cru_writel((0x1<<(16+1)), PLL_CONS(pll_id, 3));

	pll_wait_lock(pll_id);

	//return form slow
	//cru_writel(PLL_MODE_NORM(APLL_ID), CRU_MODE_CON);
	//reparent to apll

	if(new_aclk_div>=old_aclk_div) {
		cru_writel(ps->clksel0, CRU_CLKSELS_CON(0));
		cru_writel(ps->clksel1, CRU_CLKSELS_CON(1));
	}

	cru_writel(CORE_SEL_PLL_W_MSK | CORE_SEL_APLL, CRU_CLKSELS_CON(0));
	if(old_aclk_div>new_aclk_div) {
		cru_writel(ps->clksel0, CRU_CLKSELS_CON(0));
		cru_writel(ps->clksel1, CRU_CLKSELS_CON(1));
	}

	cru_writel(CORE_CLK_DIV_W_MSK|CORE_CLK_DIV(1), CRU_CLKSELS_CON(0));

	loops_per_jiffy = ps->lpj;
	smp_wmb();

	//CLKDATA_DBG("apll set loops_per_jiffy =%lu,rate(%lu)\n",loops_per_jiffy,ps->rate);

	local_irq_restore(flags);

	//gate gpll path
	// FIXME
	//cru_writel(CLK_GATE_W_MSK(CLK_GATE_CPU_GPLL_PATH) | CLK_GATE(CLK_GATE_CPU_GPLL_PATH)
	//		, CLK_GATE_CLKID_CONS(CLK_GATE_CPU_GPLL_PATH));

	CLKDATA_LOG("apll set over con(%x,%x,%x,%x),sel(%x,%x)\n", cru_readl(PLL_CONS(pll_id, 0)),
			cru_readl(PLL_CONS(pll_id, 1)), cru_readl(PLL_CONS(pll_id, 2)),
			cru_readl(PLL_CONS(pll_id, 3)), cru_readl(CRU_CLKSELS_CON(0)),
			cru_readl(CRU_CLKSELS_CON(1)));
	return 0;
}

static int plus_arm_pll_clk_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long flags;
	const struct apll_clk_set *ps;
	u32 pll_id = clk->pll->id;
	u32 temp_div;
	u32 old_aclk_div = 0, new_aclk_div;
	DVFS_DBG("%s: for rk3188 plus\n", __func__);

	ps = arm_pll_clk_get_best_pll_set(rate, (struct apll_clk_set *)clk->pll->table);

	old_aclk_div = GET_CORE_ACLK_VAL(cru_readl(CRU_CLKSELS_CON(1))&CORE_ACLK_MSK);
	new_aclk_div = GET_CORE_ACLK_VAL(ps->clksel1 & CORE_ACLK_MSK);

	CLKDATA_LOG("apll will set rate(%lu) tlb con(%x,%x,%x),sel(%x,%x)\n",
			ps->rate, ps->pllcon0, ps->pllcon1, ps->pllcon2, ps->clksel0, ps->clksel1);

	if(general_pll_clk.rate > clk->rate) {
		temp_div = clk_get_freediv(clk->rate, general_pll_clk.rate, 10);
	} else {
		temp_div = 1;
	}

	// ungating cpu gpll path
	//cru_writel(CLK_GATE_W_MSK(CLK_GATE_CPU_GPLL_PATH) | CLK_UN_GATE(CLK_GATE_CPU_GPLL_PATH), 
	//	CLK_GATE_CLKID_CONS(CLK_GATE_CPU_GPLL_PATH));

	local_irq_save(flags);
	//div arm clk for gpll

	cru_writel(CORE_CLK_DIV_W_MSK | CORE_CLK_DIV(temp_div), CRU_CLKSELS_CON(0));
	cru_writel(CORE_SEL_PLL_W_MSK | CORE_SEL_GPLL, CRU_CLKSELS_CON(0));

	loops_per_jiffy = lpj_gpll / temp_div;
	smp_wmb();

	/*if core src don't select gpll ,apll neet to enter slow mode */
	//cru_writel(PLL_MODE_SLOW(APLL_ID), CRU_MODE_CON);
	
	//enter rest
	cru_writel(PLL_RESET_W_MSK | PLL_RESET, PLL_CONS(pll_id, 3));
	cru_writel(ps->pllcon0, PLL_CONS(pll_id, 0));
	cru_writel(ps->pllcon1, PLL_CONS(pll_id, 1));
	cru_writel(ps->pllcon2, PLL_CONS(pll_id, 2));
	rk30_clock_udelay(5);
	
	//return form rest
	cru_writel(PLL_RESET_W_MSK | PLL_RESET_RESUME, PLL_CONS(pll_id, 3));
	
	//wating lock state
	rk30_clock_udelay(ps->rst_dly);
	pll_wait_lock(pll_id);

	if(new_aclk_div>=old_aclk_div) {
		cru_writel(ps->clksel0, CRU_CLKSELS_CON(0));
		cru_writel(ps->clksel1, CRU_CLKSELS_CON(1));
	}

	cru_writel(CORE_SEL_PLL_W_MSK | CORE_SEL_APLL, CRU_CLKSELS_CON(0));
	if(old_aclk_div>new_aclk_div) {
		cru_writel(ps->clksel0, CRU_CLKSELS_CON(0));
		cru_writel(ps->clksel1, CRU_CLKSELS_CON(1));
	}

	cru_writel(CORE_CLK_DIV_W_MSK|CORE_CLK_DIV(1), CRU_CLKSELS_CON(0));

	loops_per_jiffy = ps->lpj;
	smp_wmb();

	//CLKDATA_DBG("apll set loops_per_jiffy =%lu,rate(%lu)\n",loops_per_jiffy,ps->rate);

	local_irq_restore(flags);

	//gate gpll path
	// FIXME
	//cru_writel(CLK_GATE_W_MSK(CLK_GATE_CPU_GPLL_PATH) | CLK_GATE(CLK_GATE_CPU_GPLL_PATH)
	//		, CLK_GATE_CLKID_CONS(CLK_GATE_CPU_GPLL_PATH));

	CLKDATA_LOG("apll set over con(%x,%x,%x,%x),sel(%x,%x)\n", cru_readl(PLL_CONS(pll_id, 0)),
			cru_readl(PLL_CONS(pll_id, 1)), cru_readl(PLL_CONS(pll_id, 2)),
			cru_readl(PLL_CONS(pll_id, 3)), cru_readl(CRU_CLKSELS_CON(0)),
			cru_readl(CRU_CLKSELS_CON(1)));
	return 0;
}

/************************************pll clocks***************************/

static const struct apll_clk_set apll_clks[] = {
	//_APLL_SET_CLKS(_mhz, nr, nf, no, _periph_div, 
	//		_axi_core_div, _axi_div, _ahb_div, _apb_div, _ahb2apb) 	
	_APLL_SET_CLKS(2208, 1, 92, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(2184, 1, 91, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(2160, 1, 90, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(2136, 1, 89, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(2112, 1, 88, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(2088, 1, 87, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(2064, 1, 86, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(2040, 1, 85, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(2016, 1, 84, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1992, 1, 83, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1968, 1, 82, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1944, 1, 81, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1920, 1, 80, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1896, 1, 79, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1872, 1, 78, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1848, 1, 77, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1824, 1, 76, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1800, 1, 75, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1776, 1, 74, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1752, 1, 73, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1728, 1, 72, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1704, 1, 71, 1, 8, 	81, 81, 21, 41, 21),
	_APLL_SET_CLKS(1680, 1, 70, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1656, 1, 69, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1632, 1, 68, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1608, 1, 67, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1560, 1, 65, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1512, 1, 63, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1488, 1, 62, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1464, 1, 61, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1440, 1, 60, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1416, 1, 59, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1392, 1, 58, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1368, 1, 57, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1344, 1, 56, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1320, 1, 55, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1296, 1, 54, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1272, 1, 53, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1248, 1, 52, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1224, 1, 51, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1200, 1, 50, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1176, 1, 49, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1128, 1, 47, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1104, 1, 46, 1, 8, 	41, 41, 21, 41, 21),
	_APLL_SET_CLKS(1008, 1, 84, 2, 8, 	41, 31, 21, 41, 21),
	_APLL_SET_CLKS(912,  1, 76, 2, 8, 	41, 31, 21, 41, 21),
	_APLL_SET_CLKS(888,  1, 74, 2, 8, 	41, 31, 21, 41, 21),
	_APLL_SET_CLKS(816 , 1, 68, 2, 8, 	41, 31, 21, 41, 21),
	_APLL_SET_CLKS(792 , 1, 66, 2, 8, 	41, 31, 21, 41, 21),
	_APLL_SET_CLKS(696 , 1, 58, 2, 8, 	41, 31, 21, 41, 21),
	_APLL_SET_CLKS(600 , 1, 50, 2, 4, 	41, 31, 21, 41, 21),
	_APLL_SET_CLKS(504 , 1, 84, 4, 4, 	41, 21, 21, 41, 21),
	_APLL_SET_CLKS(408 , 1, 68, 4, 4, 	21, 21, 21, 41, 21),
	_APLL_SET_CLKS(312 , 1, 52, 4, 2, 	21, 21, 21, 21, 11),
	_APLL_SET_CLKS(252 , 1, 84, 8, 2, 	21, 21, 21, 21, 11),
	_APLL_SET_CLKS(216 , 1, 72, 8, 2, 	21, 21, 21, 21, 11),
	_APLL_SET_CLKS(126 , 1, 84, 16, 2, 	11, 21, 11, 11, 11),
	_APLL_SET_CLKS(48  , 1, 64, 32, 2, 	11, 11, 11, 11, 11),
	_APLL_SET_CLKS(0   , 1, 21, 4, 2, 	11, 11, 11, 11, 11),

};
static struct _pll_data apll_data = SET_PLL_DATA(APLL_ID, (void *)apll_clks);
static struct clk arm_pll_clk = {
	.name		= "arm_pll",
	.parent		= &xin24m,
	.mode 		= pll_clk_mode,
	.recalc		= plls_clk_recalc,
	.set_rate	= arm_pll_clk_set_rate,
	.round_rate	= arm_pll_clk_round_rate,
	.pll 		= &apll_data,
};

static int ddr_pll_clk_set_rate(struct clk *clk, unsigned long rate)
{
	/* do nothing here */
	return 0;
}
static struct _pll_data dpll_data = SET_PLL_DATA(DPLL_ID, NULL);
static struct clk ddr_pll_clk = {
	.name		= "ddr_pll",
	.parent		= &xin24m,
	.recalc		= plls_clk_recalc,
	.set_rate	= ddr_pll_clk_set_rate,
	.pll 		= &dpll_data,
};

static const struct pll_clk_set cpll_clks[] = {
	_PLL_SET_CLKS(360000, 1,  60,	4),
	_PLL_SET_CLKS(408000, 1,  68,	4),
	_PLL_SET_CLKS(456000, 1,  76,	4),
	_PLL_SET_CLKS(504000, 1,  84,	4),
	_PLL_SET_CLKS(552000, 1,  46,	2),
	_PLL_SET_CLKS(594000, 2,  198,	4),
	_PLL_SET_CLKS(600000, 1,  50,	2),
	_PLL_SET_CLKS(742500, 8,  495,	2),
	_PLL_SET_CLKS(768000, 1,  64,	2),
	_PLL_SET_CLKS(798000, 2,  133,	2),
	_PLL_SET_CLKS(1188000, 2, 99,	1),
	_PLL_SET_CLKS(     0, 4,  133,	1),
};
static struct _pll_data cpll_data = SET_PLL_DATA(CPLL_ID, (void *)cpll_clks);
static struct clk codec_pll_clk = {
	.name		= "codec_pll",
	.parent		= &xin24m,
	.mode		= pll_clk_mode,
	.recalc		= plls_clk_recalc,
	.set_rate	= cpll_clk_set_rate,
	.pll 		= &cpll_data,
};

static const struct pll_clk_set gpll_clks[] = {
	_PLL_SET_CLKS(148500,	2,	99,	8),
	_PLL_SET_CLKS(297000,	2,	198,	8),
	_PLL_SET_CLKS(300000,	1,	50,	4),
	_PLL_SET_CLKS(384000,	2,	128,	4),
	_PLL_SET_CLKS(594000,	2,	198,	4),
	_PLL_SET_CLKS(768000, 	1,	64,	2),
	_PLL_SET_CLKS(891000,	8,	594,	2),
	_PLL_SET_CLKS(1188000,	2,	99,	1),
	_PLL_SET_CLKS(1200000,	1,	50,	1),
	_PLL_SET_CLKS(0,	0,	 0,	0),
};
static struct _pll_data gpll_data = SET_PLL_DATA(GPLL_ID, (void *)gpll_clks);
static struct clk general_pll_clk = {
	.name		= "general_pll",
	.parent		= &xin24m,
	.recalc		= plls_clk_recalc,
	.set_rate	= gpll_clk_set_rate,
	.pll 		= &gpll_data
};
/********************************clocks***********************************/
//GATE_CLK(ddr_gpll_path, general_pll_clk, DDR_GPLL);
static struct clk clk_ddr_gpll_path = {
	.name		= "ddr_gpll_path",
	.parent		= &general_pll_clk,
	.recalc		= clksel_recalc_parent_rate,
	.gate_idx	= CLK_GATE_DDR_GPLL,
	.mode		= gate_mode,
};

/* core and cpu setting */
static int ddr_clk_set_rate(struct clk *c, unsigned long rate)
{
	CLKDATA_DBG("%s do nothing for ddr set rate\n", __func__);
	return 0;
}

static long ddr_clk_round_rate(struct clk *clk, unsigned long rate)
{
	CLKDATA_DBG("%s do nothing for ddr round rate\n", __func__);
	return ddr_set_pll_rk3066b(rate / MHZ, 0) * MHZ;
}
static unsigned long ddr_clk_recalc_rate(struct clk *clk)
{
	u32 shift = get_cru_bits(clk->clksel_con, clk->div_mask, clk->div_shift);
	unsigned long rate = 0;
	clk->parent = clk->get_parent(clk);
	clk->parent->rate = clk->parent->recalc(clk->parent);
	rate = clk->parent->rate >> shift;

	CLKDATA_DBG("%s new clock rate is %lu (shift %u), parent=%s, rate=%lu\n", 
			clk->name, rate, shift, clk->parent->name, clk->parent->rate);
	return rate;
}
static struct clk *clk_ddr_parents[2] = {&ddr_pll_clk, &clk_ddr_gpll_path};
static struct clk clk_ddr = {
	.name		= "ddr",
	.parent		= &ddr_pll_clk,
	.get_parent	= clksel_get_parent,
	.set_parent	= clksel_set_parent,
	.recalc		= ddr_clk_recalc_rate,
	.set_rate	= ddr_clk_set_rate,
	.round_rate	= ddr_clk_round_rate,
	.gate_idx	= CLK_GATE_DDRPHY,
	.clksel_con	= CRU_CLKSELS_CON(26),
	CRU_DIV_SET(0x3, 0, 4),
	CRU_SRC_SET(1, 8),
	CRU_PARENTS_SET(clk_ddr_parents),
};
static int clk_core_set_rate(struct clk *c, unsigned long rate)
{
	int ret;

	ret = clk_set_rate_nolock(c->parent, rate);
	if (ret) {
		CLKDATA_ERR("Failed to change clk pll %s to %lu\n", c->name, rate);
		return ret;
	}
	//set arm pll div 1
	set_cru_bits_w_msk(0, c->div_mask, c->div_shift, c->clksel_con);
	return 0;
}
static unsigned long clk_core_get_rate(struct clk *c)
{
	u32 div = (get_cru_bits(c->clksel_con, c->div_mask, c->div_shift) + 1);
	//c->parent->rate=c->parent->recalc(c->parent);
	return c->parent->rate / div;
}
static long core_clk_round_rate(struct clk *clk, unsigned long rate)
{
	u32 div = (get_cru_bits(clk->clksel_con, clk->div_mask, clk->div_shift) + 1);
	return clk_round_rate_nolock(clk->parent, rate) / div;
}

static int core_clksel_set_parent(struct clk *clk, struct clk *new_prt)
{

	u32 temp_div;
	struct clk *old_prt;

	if(clk->parent == new_prt)
		return 0;
	if (unlikely(!clk->parents))
		return -EINVAL;
	CLKDATA_DBG("%s,reparent %s\n", clk->name, new_prt->name);
	//arm
	old_prt = clk->parent;

	if(clk->parents[0] == new_prt) {
		new_prt->set_rate(new_prt, 300 * MHZ);
		set_cru_bits_w_msk(0, clk->div_mask, clk->div_shift, clk->clksel_con);
	} else if(clk->parents[1] == new_prt) {

		if(new_prt->rate > old_prt->rate) {
			temp_div = clk_get_freediv(old_prt->rate, new_prt->rate, clk->div_max);
			set_cru_bits_w_msk(temp_div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
		}
		set_cru_bits_w_msk(1, clk->src_mask, clk->src_shift, clk->clksel_con);
		new_prt->set_rate(new_prt, 300 * MHZ);
	} else
		return -1;


	return 0;

}

static int core_gpll_clk_set_rate(struct clk *c, unsigned long rate)
{
	u32 temp_div;
	u32 old_aclk_div = 0, new_aclk_div;
	struct arm_clks_div_set *temp_clk_div;
	unsigned long arm_gpll_rate, arm_gpll_lpj;
	temp_div = clk_get_freediv(rate, c->parent->rate, c->div_max);
	arm_gpll_rate = c->parent->rate / temp_div;

	temp_clk_div = arm_clks_get_div(arm_gpll_rate / MHZ);
	if(!temp_clk_div)
		temp_clk_div = &arm_clk_div_tlb[4];

	old_aclk_div = GET_CORE_ACLK_VAL(cru_readl(CRU_CLKSELS_CON(1))&CORE_ACLK_MSK);
	new_aclk_div = GET_CORE_ACLK_VAL(temp_clk_div->clksel1 & CORE_ACLK_MSK);
	if(c->rate >= rate) {
		arm_gpll_lpj = lpj_gpll / temp_div;
		set_cru_bits_w_msk(temp_div - 1, c->div_mask, c->div_shift, c->clksel_con);
	}

	cru_writel((temp_clk_div->clksel1), CRU_CLKSELS_CON(1));
	cru_writel((temp_clk_div->clksel0) | CORE_CLK_DIV(temp_div) | CORE_CLK_DIV_W_MSK,
			CRU_CLKSELS_CON(0));
	if((c->rate < rate)) {
		arm_gpll_lpj = lpj_gpll / temp_div;
		set_cru_bits_w_msk(temp_div - 1, c->div_mask, c->div_shift, c->clksel_con);
	}
	return 0;
}
static unsigned long arm_core_gpll_clk_get_rate(struct clk *c)
{
	return c->parent->rate;
}
static struct clk clk_core_gpll_path = {
	.name		= "cpu_gpll_path",
	.parent		= &general_pll_clk,
	.recalc		= arm_core_gpll_clk_get_rate,
	.set_rate 	= core_gpll_clk_set_rate,
	CRU_GATE_MODE_SET(gate_mode, CLK_GATE_CPU_GPLL_PATH),
};


static struct clk *clk_core_parents[2] = {&arm_pll_clk, &clk_core_gpll_path};

static struct clk clk_core = {
	.name		= "core",
	.parent		= &arm_pll_clk,
	.set_rate 	= clk_core_set_rate,
	.recalc		= clk_core_get_rate,
	.round_rate	= core_clk_round_rate,
	.set_parent 	= core_clksel_set_parent,
	.clksel_con	= CRU_CLKSELS_CON(0),
	CRU_DIV_SET(0x1f, 9, 32),
	CRU_SRC_SET(1, 8),
	CRU_PARENTS_SET(clk_core_parents),
};
GATE_CLK(l2c, clk_core, CLK_L2C);
GATE_CLK(core_dbg, clk_core, CLK_CORE_DBG);
static unsigned long aclk_recalc(struct clk *clk)
{
	unsigned long rate;
	u32 div = get_cru_bits(clk->clksel_con, clk->div_mask, clk->div_shift) + 1;

	BUG_ON(div > 5);
	if (div >= 5)
		div = 8;
	rate = clk->parent->rate / div;
	pr_debug("%s new clock rate is %ld (div %d)\n", clk->name, rate, div);

	return rate;
};
static struct clk core_periph = {
	.name		= "core_periph",
	.parent		= &clk_core,
	.gate_idx	= CLK_GATE_CORE_PERIPH,
	.recalc		= clksel_recalc_shift_2,
	.clksel_con	= CRU_CLKSELS_CON(0),
	CRU_DIV_SET(0x3, 6, 16),
};
static struct clk aclk_core = {
	.name		= "aclk_core",
	.parent		= &clk_core,
	.gate_idx	= CLK_GATE_ACLK_CORE,
	.recalc		= aclk_recalc,
	.clksel_con	= CRU_CLKSELS_CON(1),
	CRU_DIV_SET(0x7, 3, 8),
};

static struct clk *clk_cpu_div_parents[2] = {&arm_pll_clk, &general_pll_clk};
static struct clk clk_cpu_div = {
	.name		= "cpu_div",
	.parent		= &arm_pll_clk,
	.set_rate 	= clksel_set_rate_freediv,
	.recalc		= clksel_recalc_div,
	.clksel_con	= CRU_CLKSELS_CON(0),
	CRU_DIV_SET(0x1f, 0, 32),
	CRU_SRC_SET(1, 5),
	CRU_PARENTS_SET(clk_cpu_div_parents),
};

static struct clk aclk_cpu = {
	.name		= "aclk_cpu",
	.parent		= &clk_cpu_div,
	.gate_idx	= CLK_GATE_ACLK_CPU,
};

static struct clk hclk_cpu = {
	.name		= "hclk_cpu",
	.parent		= &aclk_cpu,
	.gate_idx	= CLK_GATE_HCLK_CPU,
	.recalc		= clksel_recalc_shift,
	.set_rate	= clksel_set_rate_shift,
	.clksel_con	= CRU_CLKSELS_CON(1),
	CRU_DIV_SET(0x3, 8, 4),

};

static struct clk pclk_cpu = {
	.name		= "pclk_cpu",
	.parent		= &aclk_cpu,
	.gate_idx	= CLK_GATE_PCLK_CPU,
	.recalc		= clksel_recalc_shift,
	.set_rate	= clksel_set_rate_shift,
	.clksel_con	= CRU_CLKSELS_CON(1),
	CRU_DIV_SET(0x3, 12, 8),
};

static struct clk ahb2apb_cpu = {
	.name		= "ahb2apb",
	.parent		= &hclk_cpu,
	.recalc		= clksel_recalc_shift,
	.set_rate	= clksel_set_rate_shift,
	.clksel_con	= CRU_CLKSELS_CON(1),
	CRU_DIV_SET(0x3, 14, 4),
};


static struct clk atclk_cpu = {
	.name		= "atclk_cpu",
	.parent		= &pclk_cpu,
	.gate_idx	= CLK_GATE_ATCLK_CPU,
};

/* GPU setting */
static int clk_gpu_set_rate(struct clk *clk, unsigned long rate)
{
	unsigned long max_rate = rate / 100 * 105;	/* +5% */
	return clkset_rate_freediv_autosel_parents(clk, max_rate);
};

static struct clk *aclk_gpu_parents[2] = {&codec_pll_clk, &general_pll_clk};

static struct clk aclk_gpu = {
	.name		= "aclk_gpu",
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.round_rate	= clk_freediv_round_autosel_parents_rate,
	.set_rate	= clkset_rate_freediv_autosel_parents,
	.clksel_con 	= CRU_CLKSELS_CON(34),
	.gate_idx	= CLK_GATE_ACLK_GPU,
	CRU_DIV_SET(0x1f, 0, 32),
	CRU_SRC_SET(0x1, 7),
	CRU_PARENTS_SET(aclk_gpu_parents),
};

/* vcodec setting */
static unsigned long clksel_recalc_vpu_hclk(struct clk *clk)
{
	unsigned long rate = clk->parent->rate / 4;
	pr_debug("%s new clock rate is %lu (div %u)\n", clk->name, rate, 4);
	return rate;
}

static struct clk *aclk_vepu_parents[2] = {&codec_pll_clk, &general_pll_clk};

static struct clk aclk_vepu = {
	.name		= "aclk_vepu",
	.parent		= &codec_pll_clk,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clkset_rate_freediv_autosel_parents,
	.clksel_con	= CRU_CLKSELS_CON(32),
	.gate_idx	= CLK_GATE_ACLK_VEPU,
	CRU_DIV_SET(0x1f, 0, 32),
	CRU_SRC_SET(0x1, 7),
	CRU_PARENTS_SET(aclk_vepu_parents),
};

static struct clk *aclk_vdpu_parents[2] = {&codec_pll_clk, &general_pll_clk};

static struct clk aclk_vdpu = {
	.name		= "aclk_vdpu",
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clkset_rate_freediv_autosel_parents,
	.clksel_con	= CRU_CLKSELS_CON(32),
	.gate_idx	= CLK_GATE_ACLK_VDPU,
	CRU_DIV_SET(0x1f, 8, 32),
	CRU_SRC_SET(0x1, 15),
	CRU_PARENTS_SET(aclk_vdpu_parents),
};
static struct clk hclk_vepu = {
	.name		= "hclk_vepu",
	.parent		= &aclk_vepu,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_vpu_hclk,
	.gate_idx	= CLK_GATE_HCLK_VEPU,
};

static struct clk hclk_vdpu = {
	.name		= "hclk_vdpu",
	.parent		= &aclk_vdpu,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_vpu_hclk,
	.gate_idx	= CLK_GATE_HCLK_VDPU,
};

/* aclk lcdc setting */
static struct clk *aclk_lcdc0_parents[] = {&codec_pll_clk, &general_pll_clk};

static struct clk aclk_lcdc0_pre = {
	.name		= "aclk_lcdc0_pre",
	.parent		= &codec_pll_clk,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clkset_rate_freediv_autosel_parents,
	.gate_idx	= CLK_GATE_ACLK_LCDC0_SRC,
	.clksel_con	= CRU_CLKSELS_CON(31),
	CRU_DIV_SET(0x1f, 0, 32),
	CRU_SRC_SET(0x1, 7),
	CRU_PARENTS_SET(aclk_lcdc0_parents),
};

static struct clk *aclk_lcdc1_parents[] = {&codec_pll_clk, &general_pll_clk};

static struct clk aclk_lcdc1_pre = {
	.name		= "aclk_lcdc1_pre",
	.parent		= &codec_pll_clk,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clkset_rate_freediv_autosel_parents,
	.gate_idx	= CLK_GATE_ACLK_LCDC1_SRC,
	.clksel_con	= CRU_CLKSELS_CON(31),
	CRU_DIV_SET(0x1f, 8, 32),
	CRU_SRC_SET(0x1, 15),
	CRU_PARENTS_SET(aclk_lcdc1_parents),
};

/* aclk/hclk/pclk periph setting */
static struct clk *aclk_periph_parents[2] = {&codec_pll_clk, &general_pll_clk};

static struct clk aclk_periph = {
	.name		= "aclk_periph",
	.parent		= &general_pll_clk,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_ACLK_PERIPH,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_freediv,
	.clksel_con	= CRU_CLKSELS_CON(10),
	CRU_DIV_SET(0x1f, 0, 32),
	CRU_SRC_SET(1, 15),
	CRU_PARENTS_SET(aclk_periph_parents),
};
GATE_CLK(periph_src, aclk_periph, PERIPH_SRC);

static struct clk pclk_periph = {
	.name		= "pclk_periph",
	.parent		= &aclk_periph,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_PCLK_PERIPH,
	.recalc		= clksel_recalc_shift,
	.set_rate	= clksel_set_rate_shift,
	.clksel_con	= CRU_CLKSELS_CON(10),
	CRU_DIV_SET(0x3, 12, 8),
};

static struct clk hclk_periph = {
	.name		= "hclk_periph",
	.parent		= &aclk_periph,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_HCLK_PERIPH,
	.recalc		= clksel_recalc_shift,
	.set_rate	= clksel_set_rate_shift,
	.clksel_con 	= CRU_CLKSELS_CON(10),
	CRU_DIV_SET(0x3, 8, 4),
};
/* dclk lcdc setting */
// FIXME
static int clksel_set_rate_hdmi(struct clk *clk, unsigned long rate)
{
	u32 div, old_div;
	int i;
	unsigned long new_rate;
	int ret = 0;

	if(clk->rate == rate)
		return 0;
	for(i = 0; i < 2; i++) {
		div = clk_get_freediv(rate, clk->parents[i]->rate, clk->div_max);
		new_rate = clk->parents[i]->rate / div;
		if((rate == new_rate) && !(clk->parents[i]->rate % div)) {
			break;
		}
	}
	if(i >= 2) {
		CLKDATA_ERR("%s can't set fixed rate%lu\n", clk->name, rate);
		return -1;
	}

	//CLKDATA_DBG("%s set rate %lu(from %s)\n",clk->name,rate,clk->parents[i]->name);

	old_div = CRU_GET_REG_BITS_VAL(cru_readl(clk->clksel_con),
			clk->div_shift, clk->div_mask) + 1;
	if(div > old_div)
		set_cru_bits_w_msk(div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);

	if(clk->parents[i] != clk->parent) {
		ret = clk_set_parent_nolock(clk, clk->parents[i]);
	}

	if (ret) {
		CLKDATA_ERR("lcdc1 %s can't get rate%lu,reparent%s(now %s) err\n",
				clk->name, rate, clk->parents[i]->name, clk->parent->name);
		return ret;
	}
	set_cru_bits_w_msk(div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
	return 0;
}

static int dclk_lcdc_set_rate(struct clk *clk, unsigned long rate)
{
	if (rate == 27 * MHZ)
		return clkset_rate_freediv_autosel_parents(clk, rate);
	else
		return clkset_rate_evendiv_autosel_parents(clk, rate);

#if 0
	int ret = 0;
	struct clk *parent;
	if (rate == 27 * MHZ && (rk30_clock_flags & CLK_FLG_EXT_27MHZ)) {
		parent = clk->parents[1];
		//CLKDATA_DBG(" %s from=%s\n",clk->name,parent->name);
	} else {
		parent = clk->parents[0];
	}
	//CLKDATA_DBG(" %s set rate=%lu parent %s(old %s)\n",
	//clk->name,rate,parent->name,clk->parent->name);

	if(parent != clk->parents[1]) {
		ret = clk_set_rate_nolock(parent, rate); //div 1:1
		if (ret) {
			CLKDATA_DBG("%s set rate=%lu err\n", clk->name, rate);
			return ret;
		}
	}
	if (clk->parent != parent) {
		ret = clk_set_parent_nolock(clk, parent);
		if (ret) {
			CLKDATA_DBG("%s can't get rate%lu,reparent err\n", clk->name, rate);
			return ret;
		}
	}
	return ret;
#endif
}

static struct clk *dclk_lcdc0_parents[2] = {&codec_pll_clk, &general_pll_clk};
static struct clk dclk_lcdc0 = {
	.name		= "dclk_lcdc0",
	.mode		= gate_mode,
	.set_rate	= dclk_lcdc_set_rate,
	.recalc		= clksel_recalc_div,
	.gate_idx	= CLK_GATE_DCLK_LCDC0_SRC,
	.clksel_con	= CRU_CLKSELS_CON(27),
	CRU_SRC_SET(0x1, 0),
	CRU_DIV_SET(0xff, 8, 256),
	CRU_PARENTS_SET(dclk_lcdc0_parents),
};

static struct clk *dclk_lcdc1_parents[2] = {&codec_pll_clk, &general_pll_clk};
static struct clk dclk_lcdc1 = {
	.name		= "dclk_lcdc1",
	.mode		= gate_mode,
	.set_rate	= dclk_lcdc_set_rate,
	.recalc		= clksel_recalc_div,
	.gate_idx	= CLK_GATE_DCLK_LCDC1_SRC,
	.clksel_con	= CRU_CLKSELS_CON(28),
	CRU_SRC_SET(0x1, 0),
	CRU_DIV_SET(0xff, 8, 256),
	CRU_PARENTS_SET(dclk_lcdc1_parents),
};

/* cif setting */
// FIXME
static struct clk *cifout_sel_pll_parents[2] = {&codec_pll_clk, &general_pll_clk};
static struct clk cif_out_pll = {
	.name		= "cif_out_pll",
	.parent		= &general_pll_clk,
	.clksel_con	= CRU_CLKSELS_CON(29),
	CRU_SRC_SET(0x1, 0),
	CRU_PARENTS_SET(cifout_sel_pll_parents),
};

static struct clk cif0_out_div = {
	.name		= "cif0_out_div",
	.parent		= &cif_out_pll,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_freediv,
	.gate_idx	= CLK_GATE_CIF0_OUT,
	.clksel_con	= CRU_CLKSELS_CON(29),
	CRU_DIV_SET(0x1f, 1, 32),
};

static int cif_out_set_rate(struct clk *clk, unsigned long rate)
{
	int ret = 0;
	struct clk *parent;

	if (rate == 24 * MHZ) {
		parent = clk->parents[1];
	} else {
		parent = clk->parents[0];
		ret = clk_set_rate_nolock(parent, rate);
		if (ret)
			return ret;
	}
	if (clk->parent != parent)
		ret = clk_set_parent_nolock(clk, parent);

	return ret;
}

static struct clk *cif0_out_parents[2] = {&cif0_out_div, &xin24m};
static struct clk cif0_out = {
	.name		= "cif0_out",
	.parent		= &cif0_out_div,
	.set_rate	= cif_out_set_rate,
	.clksel_con	= CRU_CLKSELS_CON(29),
	CRU_SRC_SET(0x1, 7),
	CRU_PARENTS_SET(cif0_out_parents),
};

static struct clk pclkin_cif0 = {
	.name		= "pclkin_cif0",
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_PCLKIN_CIF0,
};

static struct clk inv_cif0 = {
	.name		= "inv_cif0",
	.parent		= &pclkin_cif0,
};

static struct clk *cif0_in_parents[2] = {&pclkin_cif0, &inv_cif0};
static struct clk cif0_in = {
	.name		= "cif0_in",
	.parent		= &pclkin_cif0,
	.clksel_con	= CRU_CLKSELS_CON(30),
	CRU_SRC_SET(0x1, 8),
	CRU_PARENTS_SET(cif0_in_parents),
};

/* i2s/spdif setting */
static struct clk *clk_i2s_div_parents[] = {&general_pll_clk, &codec_pll_clk};
static struct clk clk_i2s_pll = {
	.name		= "i2s_pll",
	.parent		= &general_pll_clk,
	.clksel_con	= CRU_CLKSELS_CON(2),
	CRU_SRC_SET(0x1, 15),
	CRU_PARENTS_SET(clk_i2s_div_parents),
};

static struct clk clk_i2s0_div = {
	.name		= "i2s0_div",
	.parent		= &clk_i2s_pll,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_freediv,
	.round_rate	= clksel_freediv_round_rate,
	.gate_idx	= CLK_GATE_I2S0_SRC,
	.clksel_con	= CRU_CLKSELS_CON(3),
	CRU_DIV_SET(0x7f, 0, 64),
};

static struct clk clk_spdif_div = {
	.name		= "spdif_div",
	.parent		= &clk_i2s_pll,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_freediv,
	.round_rate	= clksel_freediv_round_rate,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_SPDIF_SRC,
	.clksel_con	= CRU_CLKSELS_CON(5),
	CRU_DIV_SET(0x7f, 0, 64),
};
static int clk_i2s_fracdiv_set_rate(struct clk *clk, unsigned long rate)
{
	u32 numerator, denominator;
	int i = 10;
	//clk_i2s_div->clk_i2s_pll->gpll/cpll
	//clk->parent->parent
	if(frac_div_get_seting(rate, clk->parent->parent->rate,
				&numerator, &denominator) == 0) {
		clk_set_rate_nolock(clk->parent, clk->parent->parent->rate); //PLL:DIV 1:

		while (i--) {
			cru_writel_frac((numerator - 1) << 16 | denominator, clk->clksel_con);
			mdelay(1);
			cru_writel_frac(numerator << 16 | denominator, clk->clksel_con);
			mdelay(1);
		}
		CLKDATA_DBG("%s set rate=%lu,is ok\n", clk->name, rate);
	} else {
		CLKDATA_ERR("clk_frac_div can't get rate=%lu,%s\n", rate, clk->name);
		return -ENOENT;
	}
	return 0;
}

static struct clk clk_i2s0_frac_div = {
	.name		= "i2s0_frac_div",
	.parent		= &clk_i2s0_div,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_I2S0_FRAC,
	.recalc		= clksel_recalc_frac,
	.set_rate	= clk_i2s_fracdiv_set_rate,
	.clksel_con	= CRU_CLKSELS_CON(7),
};

static struct clk clk_spdif_frac_div = {
	.name		= "spdif_frac_div",
	.parent		= &clk_spdif_div,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_SPDIF_FRAC,
	.recalc		= clksel_recalc_frac,
	.set_rate	= clk_i2s_fracdiv_set_rate,
	.clksel_con	= CRU_CLKSELS_CON(9),
};

#define I2S_SRC_DIV	(0x0)
#define I2S_SRC_FRAC	(0x1)
#define I2S_SRC_12M	(0x2)

static int i2s_set_rate(struct clk *clk, unsigned long rate)
{
	int ret = -EINVAL;
	struct clk *parent;

	if (rate == clk->parents[I2S_SRC_12M]->rate) {
		parent = clk->parents[I2S_SRC_12M];
	} else if((long)clk_round_rate_nolock(clk->parents[I2S_SRC_DIV], rate) == rate) {
		parent = clk->parents[I2S_SRC_DIV];
	} else {
		parent = clk->parents[I2S_SRC_FRAC];
	}

	CLKDATA_DBG(" %s set rate=%lu parent %s(old %s)\n",
			clk->name, rate, parent->name, clk->parent->name);

	if(parent != clk->parents[I2S_SRC_12M]) {
		ret = clk_set_rate_nolock(parent, rate); //div 1:1
		if (ret) {
			CLKDATA_DBG("%s set rate%lu err\n", clk->name, rate);
			return ret;
		}
	}

	if (clk->parent != parent) {
		ret = clk_set_parent_nolock(clk, parent);
		if (ret) {
			CLKDATA_DBG("%s can't get rate%lu,reparent err\n", clk->name, rate);
			return ret;
		}
	}

	return ret;
};

static struct clk *clk_i2s0_parents[3] = {&clk_i2s0_div, &clk_i2s0_frac_div, &clk_12m};

static struct clk clk_i2s0 = {
	.name		= "i2s0",
	.set_rate	= i2s_set_rate,
	.clksel_con	= CRU_CLKSELS_CON(3),
	CRU_SRC_SET(0x3, 8),
	CRU_PARENTS_SET(clk_i2s0_parents),
};

static struct clk *clk_spdif_parents[3] = {&clk_spdif_div, &clk_spdif_frac_div, &clk_12m};

static struct clk clk_spdif = {
	.name		= "spdif",
	.parent		= &clk_spdif_frac_div,
	.set_rate	= i2s_set_rate,
	.clksel_con = CRU_CLKSELS_CON(5),
	CRU_SRC_SET(0x3, 8),
	CRU_PARENTS_SET(clk_spdif_parents),
};

/* otgphy setting */
GATE_CLK(otgphy0, xin24m, OTGPHY0);
GATE_CLK(otgphy1, xin24m, OTGPHY1);

static struct clk clk_otgphy0_480m = {
	.name		= "otgphy0_480m",
	.parent		= &clk_otgphy0,
};
static struct clk clk_otgphy1_480m = {
	.name		= "otgphy1_480m",
	.parent		= &clk_otgphy1,
};

/* hsicphy setting */
#ifdef ARCH_RK31
static struct clk *clk_hsicphy_parents[4] = {&clk_otgphy0_480m, &clk_otgphy1_480m, &general_pll_clk, &codec_pll_clk};
static struct clk clk_hsicphy_480m = {
	.name		= "hsicphy_480m",
	.parent		= &clk_otgphy0_480m,
	.clksel_con	 = CRU_CLKSELS_CON(30),
	CRU_SRC_SET(0x3, 0),
	CRU_PARENTS_SET(clk_hsicphy_parents),
};
static struct clk clk_hsicphy_12m = {
	.name		= "hsicphy_12m",
	.parent		= &clk_hsicphy_480m,
	.clksel_con 	= CRU_CLKSELS_CON(11),
	CRU_DIV_SET(0x3f, 8, 64),
};
#endif

/* mac and rmii setting */
// FIXME
static struct clk rmii_clkin = {
	.name		= "rmii_clkin",
};
static struct clk *clk_mac_ref_div_parents[2] = {&general_pll_clk, &ddr_pll_clk};
static struct clk clk_mac_pll_div = {
	.name		= "mac_pll_div",
	.parent		= &general_pll_clk,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_MAC_SRC,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_freediv,
	.clksel_con	= CRU_CLKSELS_CON(21),
	CRU_DIV_SET(0x1f, 8, 32),
	CRU_SRC_SET(0x1, 0),
	CRU_PARENTS_SET(clk_mac_ref_div_parents),
};

static int clksel_mac_ref_set_rate(struct clk *clk, unsigned long rate)
{
	if(clk->parent == clk->parents[1]) {
		CLKDATA_DBG("mac_ref clk is form mii clkin,can't set it\n" );
		return -ENOENT;
	} else if(clk->parent == clk->parents[0]) {
		return clk_set_rate_nolock(clk->parents[0], rate);
	}
	return -ENOENT;
}

static struct clk *clk_mac_ref_parents[2] = {&clk_mac_pll_div, &rmii_clkin};

static struct clk clk_mac_ref = {
	.name		= "mac_ref",
	.parent		= &clk_mac_pll_div,
	.set_rate	= clksel_mac_ref_set_rate,
	.clksel_con 	= CRU_CLKSELS_CON(21),
	CRU_SRC_SET(0x1, 4),
	CRU_PARENTS_SET(clk_mac_ref_parents),
};

static int clk_set_mii_tx_parent(struct clk *clk, struct clk *parent)
{
	return clk_set_parent_nolock(clk->parent, parent);
}

static struct clk clk_mii_tx = {
	.name		= "mii_tx",
	.parent		= &clk_mac_ref,
	//.set_parent	= clk_set_mii_tx_parent,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_MAC_LBTEST,
};

/* hsadc and saradc */
static struct clk *clk_hsadc_pll_parents[2] = {&general_pll_clk, &codec_pll_clk};
static struct clk clk_hsadc_pll_div = {
	.name		= "hsadc_pll_div",
	.parent 	= &general_pll_clk,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_HSADC_SRC,
	.recalc 	= clksel_recalc_div,
	.round_rate	= clk_freediv_round_autosel_parents_rate,
	.set_rate	= clkset_rate_freediv_autosel_parents,
	//.round_rate	= clksel_freediv_round_rate,
	//.set_rate	= clksel_set_rate_freediv,
	.clksel_con = CRU_CLKSELS_CON(22),
	CRU_DIV_SET(0xff, 8, 256),
	CRU_SRC_SET(0x1, 0),
	CRU_PARENTS_SET(clk_hsadc_pll_parents),
};
static int clk_hsadc_fracdiv_set_rate_fixed_parent(struct clk *clk, unsigned long rate)
{
	u32 numerator, denominator;
	//        clk_hsadc_pll_div->gpll/cpll
	//clk->parent->parent
	if(frac_div_get_seting(rate, clk->parent->parent->rate,
				&numerator, &denominator) == 0) {
		clk_set_rate_nolock(clk->parent, clk->parent->parent->rate); //PLL:DIV 1:

		cru_writel_frac(numerator << 16 | denominator, clk->clksel_con);

		CLKDATA_DBG("%s set rate=%lu,is ok\n", clk->name, rate);
	} else {
		CLKDATA_ERR("clk_frac_div can't get rate=%lu,%s\n", rate, clk->name);
		return -ENOENT;
	}
	return 0;
}
static int clk_hsadc_fracdiv_set_rate_auto_parents(struct clk *clk, unsigned long rate)
{
	u32 numerator, denominator;
	u32 i, ret = 0;
	//        clk_hsadc_pll_div->gpll/cpll
	//clk->parent->parent
	for(i = 0; i < 2; i++) {
		if(frac_div_get_seting(rate, clk->parent->parents[i]->rate,
					&numerator, &denominator) == 0)
			break;
	}
	if(i >= 2)
		return -ENOENT;

	if(clk->parent->parent != clk->parent->parents[i])
		ret = clk_set_parent_nolock(clk->parent, clk->parent->parents[i]);
	if(ret == 0) {
		clk_set_rate_nolock(clk->parent, clk->parent->parents[i]->rate); //PLL:DIV 1:

		cru_writel_frac(numerator << 16 | denominator, clk->clksel_con);

		CLKDATA_DBG("clk_frac_div %s, rate=%lu\n", clk->name, rate);
	} else {
		CLKDATA_ERR("clk_frac_div can't get rate=%lu,%s\n", rate, clk->name);
		return -ENOENT;
	}
	return 0;
}

static long clk_hsadc_fracdiv_round_rate(struct clk *clk, unsigned long rate)
{
	u32 numerator, denominator;

	CLKDATA_ERR("clk_hsadc_fracdiv_round_rate\n");
	if(frac_div_get_seting(rate, clk->parent->parent->rate,
				&numerator, &denominator) == 0)
		return rate;

	return 0;
}
static struct clk clk_hsadc_frac_div = {
	.name		= "hsadc_frac_div",
	.parent		= &clk_hsadc_pll_div,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_frac,
	.set_rate	= clk_hsadc_fracdiv_set_rate_auto_parents,
	.round_rate	= clk_hsadc_fracdiv_round_rate,
	.gate_idx	= CLK_GATE_HSADC_FRAC_SRC,
	.clksel_con	= CRU_CLKSELS_CON(23),
};

#define HSADC_SRC_DIV 0x0
#define HSADC_SRC_FRAC 0x1
#define HSADC_SRC_EXT 0x2
static int clk_hsadc_set_rate(struct clk *clk, unsigned long rate)
{
	int ret = -EINVAL;
	struct clk *parent;

	if(clk->parent == clk->parents[HSADC_SRC_EXT]) {
		CLKDATA_DBG("hsadc clk is form ext\n");
		return 0;
	} else if((long)clk_round_rate_nolock(clk->parents[HSADC_SRC_DIV], rate) == rate) {
		parent = clk->parents[HSADC_SRC_DIV];
	} else if((long)clk_round_rate_nolock(clk->parents[HSADC_SRC_FRAC], rate) == rate) {
		parent = clk->parents[HSADC_SRC_FRAC];
	} else
		parent = clk->parents[HSADC_SRC_DIV];

	CLKDATA_DBG(" %s set rate=%lu parent %s(old %s)\n",
			clk->name, rate, parent->name, clk->parent->name);

	ret = clk_set_rate_nolock(parent, rate);
	if (ret) {
		CLKDATA_ERR("%s set rate%lu err\n", clk->name, rate);
		return ret;
	}
	if (clk->parent != parent) {
		ret = clk_set_parent_nolock(clk, parent);
		if (ret) {
			CLKDATA_ERR("%s can't get rate%lu,reparent err\n", clk->name, rate);
			return ret;
		}
	}
	return ret;
}

static struct clk clk_hsadc_ext = {
	.name		= "hsadc_ext",
};

static struct clk *clk_hsadc_out_parents[3] = {&clk_hsadc_pll_div, &clk_hsadc_frac_div, &clk_hsadc_ext};
static struct clk clk_hsadc_out = {
	.name		= "hsadc_out",
	.parent		= &clk_hsadc_pll_div,
	.set_rate	= clk_hsadc_set_rate,
	.clksel_con	= CRU_CLKSELS_CON(22),
	CRU_SRC_SET(0x3, 4),
	CRU_PARENTS_SET(clk_hsadc_out_parents),
};
static struct clk clk_hsadc_out_inv = {
	.name		= "hsadc_out_inv",
	.parent		= &clk_hsadc_out,
};

static struct clk *clk_hsadc_parents[3] = {&clk_hsadc_out, &clk_hsadc_out_inv};
static struct clk clk_hsadc = {
	.name		= "hsadc",
	.parent		= &clk_hsadc_out,
	.clksel_con	= CRU_CLKSELS_CON(22),
	CRU_SRC_SET(0x1, 7),
	CRU_PARENTS_SET(clk_hsadc_parents),
};

static struct clk clk_saradc = {
	.name		= "saradc",
	.parent		= &xin24m,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_freediv,
	.gate_idx	= CLK_GATE_SARADC_SRC,
	.clksel_con	= CRU_CLKSELS_CON(24),
	CRU_DIV_SET(0xff, 8, 256),
};

/* smc setting */
GATE_CLK(smc, hclk_periph, SMC_SRC);//smc
static struct clk clkn_smc = {
	.name		= "smc_inv",
	.parent		= &clk_smc,
};

/* spi setting */
static struct clk clk_spi0 = {
	.name		= "spi0",
	.parent		= &pclk_periph,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_freediv,
	.gate_idx	= CLK_GATE_SPI0_SRC,
	.clksel_con	= CRU_CLKSELS_CON(25),
	CRU_DIV_SET(0x7f, 0, 128),
};

static struct clk clk_spi1 = {
	.name		= "spi1",
	.parent		= &pclk_periph,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_freediv,
	.gate_idx	= CLK_GATE_SPI1_SRC,
	.clksel_con	= CRU_CLKSELS_CON(25),
	CRU_DIV_SET(0x7f, 8, 128),
};

/* sdmmc/sdio/emmc setting */
static struct clk clk_sdmmc = {
	.name		= "sdmmc",
	.parent		= &hclk_periph,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_even,
	.gate_idx	= CLK_GATE_MMC0_SRC,
	.clksel_con 	= CRU_CLKSELS_CON(11),
	CRU_DIV_SET(0x3f, 0, 64),
};

static struct clk clk_sdio = {
	.name		= "sdio",
	.parent		= &hclk_periph,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_even,
	.gate_idx	= CLK_GATE_SDIO_SRC,
	.clksel_con 	= CRU_CLKSELS_CON(12),
	CRU_DIV_SET(0x3f, 0, 64),

};

static struct clk clk_emmc = {
	.name		= "emmc",
	.parent		= &hclk_periph,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_even,
	.gate_idx	= CLK_GATE_EMMC_SRC,
	.clksel_con 	= CRU_CLKSELS_CON(12),
	CRU_DIV_SET(0x3f, 8, 64),
};

/* uart setting */
static struct clk *clk_uart_src_parents[2] = {&general_pll_clk, &codec_pll_clk};
static struct clk clk_uart_pll = {
	.name		= "uart_pll",
	.parent		= &general_pll_clk,
	.clksel_con 	= CRU_CLKSELS_CON(12),
	CRU_SRC_SET(0x1, 15),
	CRU_PARENTS_SET(clk_uart_src_parents),
};
static struct clk clk_uart0_div = {
	.name		= "uart0_div",
	.parent		= &clk_uart_pll,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_UART0_SRC,
	.recalc		= clksel_recalc_div,
	.set_rate	= clksel_set_rate_freediv,
	.round_rate	= clksel_freediv_round_rate,
	.clksel_con	= CRU_CLKSELS_CON(13),
	CRU_DIV_SET(0x7f, 0, 64),
};
static struct clk clk_uart1_div = {
	.name		= "uart1_div",
	.parent		= &clk_uart_pll,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_UART1_SRC,
	.recalc		= clksel_recalc_div,
	.round_rate	= clksel_freediv_round_rate,
	.set_rate	= clksel_set_rate_freediv,
	.clksel_con	= CRU_CLKSELS_CON(14),
	CRU_DIV_SET(0x7f, 0, 64),
};

static struct clk clk_uart2_div = {
	.name		= "uart2_div",
	.parent		= &clk_uart_pll,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_UART2_SRC,
	.recalc		= clksel_recalc_div,
	.round_rate	= clksel_freediv_round_rate,
	.set_rate	= clksel_set_rate_freediv,
	.clksel_con	= CRU_CLKSELS_CON(15),
	CRU_DIV_SET(0x7f, 0, 64),
};

static struct clk clk_uart3_div = {
	.name		= "uart3_div",
	.parent		= &clk_uart_pll,
	.mode		= gate_mode,
	.gate_idx	= CLK_GATE_UART3_SRC,
	.recalc		= clksel_recalc_div,
	.round_rate	= clksel_freediv_round_rate,
	.set_rate	= clksel_set_rate_freediv,
	.clksel_con	= CRU_CLKSELS_CON(16),
	CRU_DIV_SET(0x7f, 0, 64),
};
static int clk_uart_fracdiv_set_rate(struct clk *clk, unsigned long rate)
{
	u32 numerator, denominator;
	//clk_uart0_div->clk_uart_pll->gpll/cpll
	//clk->parent->parent
	if(frac_div_get_seting(rate, clk->parent->parent->rate,
				&numerator, &denominator) == 0) {
		clk_set_rate_nolock(clk->parent, clk->parent->parent->rate); //PLL:DIV 1:

		cru_writel_frac(numerator << 16 | denominator, clk->clksel_con);

		CLKDATA_DBG("%s set rate=%lu,is ok\n", clk->name, rate);
	} else {
		CLKDATA_ERR("clk_frac_div can't get rate=%lu,%s\n", rate, clk->name);
		return -ENOENT;
	}
	return 0;
}

static struct clk clk_uart0_frac_div = {
	.name		= "uart0_frac_div",
	.parent		= &clk_uart0_div,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_frac,
	.set_rate	= clk_uart_fracdiv_set_rate,
	.gate_idx	= CLK_GATE_UART0_FRAC_SRC,
	.clksel_con	= CRU_CLKSELS_CON(17),
};
static struct clk clk_uart1_frac_div = {
	.name		= "uart1_frac_div",
	.parent		= &clk_uart1_div,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_frac,
	.set_rate	= clk_uart_fracdiv_set_rate,
	.gate_idx	= CLK_GATE_UART1_FRAC_SRC,
	.clksel_con	= CRU_CLKSELS_CON(18),
};
static struct clk clk_uart2_frac_div = {
	.name		= "uart2_frac_div",
	.mode		= gate_mode,
	.parent		= &clk_uart2_div,
	.recalc		= clksel_recalc_frac,
	.set_rate	= clk_uart_fracdiv_set_rate,
	.gate_idx	= CLK_GATE_UART2_FRAC_SRC,
	.clksel_con	= CRU_CLKSELS_CON(19),
};
static struct clk clk_uart3_frac_div = {
	.name		= "uart3_frac_div",
	.parent		= &clk_uart3_div,
	.mode		= gate_mode,
	.recalc		= clksel_recalc_frac,
	.set_rate	= clk_uart_fracdiv_set_rate,
	.gate_idx	= CLK_GATE_UART3_FRAC_SRC,
	.clksel_con	= CRU_CLKSELS_CON(20),
};

#define UART_SRC_DIV 0
#define UART_SRC_FRAC 1
#define UART_SRC_24M 2

static int clk_uart_set_rate(struct clk *clk, unsigned long rate)
{
	int ret = 0;
	struct clk *parent;

	if(rate == clk->parents[UART_SRC_24M]->rate) { //24m
		parent = clk->parents[UART_SRC_24M];
	} else if((long)clk_round_rate_nolock(clk->parents[UART_SRC_DIV], rate) == rate) {
		parent = clk->parents[UART_SRC_DIV];
	} else {
		parent = clk->parents[UART_SRC_FRAC];
	}

	CLKDATA_DBG(" %s set rate=%lu parent %s(old %s)\n",
			clk->name, rate, parent->name, clk->parent->name);

	if(parent != clk->parents[UART_SRC_24M]) {
		ret = clk_set_rate_nolock(parent, rate);
		if (ret) {
			CLKDATA_DBG("%s set rate%lu err\n", clk->name, rate);
			return ret;
		}
	}

	if (clk->parent != parent) {
		ret = clk_set_parent_nolock(clk, parent);
		if (ret) {
			CLKDATA_DBG("%s can't get rate%lu,reparent err\n", clk->name, rate);
			return ret;
		}
	}

	return ret;
}

static struct clk *clk_uart0_parents[3] = {&clk_uart0_div, &clk_uart0_frac_div, &xin24m};
static struct clk clk_uart0 = {
	.name		= "uart0",
	.set_rate	= clk_uart_set_rate,
	.clksel_con	= CRU_CLKSELS_CON(13),
	CRU_SRC_SET(0x3, 8),
	CRU_PARENTS_SET(clk_uart0_parents),
};

static struct clk *clk_uart1_parents[3] = {&clk_uart1_div, &clk_uart1_frac_div, &xin24m};
static struct clk clk_uart1 = {
	.name		= "uart1",
	.set_rate	= clk_uart_set_rate,
	.clksel_con	= CRU_CLKSELS_CON(14),
	CRU_SRC_SET(0x3, 8),
	CRU_PARENTS_SET(clk_uart1_parents),
};

static struct clk *clk_uart2_parents[3] = {&clk_uart2_div, &clk_uart2_frac_div, &xin24m};
static struct clk clk_uart2 = {
	.name		= "uart2",
	.set_rate	= clk_uart_set_rate,
	.clksel_con	= CRU_CLKSELS_CON(15),
	CRU_SRC_SET(0x3, 8),
	CRU_PARENTS_SET(clk_uart2_parents),
};
static struct clk *clk_uart3_parents[3] = {&clk_uart3_div, &clk_uart3_frac_div, &xin24m};
static struct clk clk_uart3 = {
	.name		= "uart3",
	.set_rate	= clk_uart_set_rate,
	.clksel_con	= CRU_CLKSELS_CON(16),
	CRU_SRC_SET(0x3, 8),
	CRU_PARENTS_SET(clk_uart3_parents),
};

/* timer setting */
GATE_CLK(timer0, xin24m, TIMER0);
GATE_CLK(timer1, xin24m, TIMER1);
GATE_CLK(timer2, xin24m, TIMER2);
GATE_CLK(timer3, xin24m, TIMER3);
GATE_CLK(timer4, xin24m, TIMER4);
GATE_CLK(timer5, xin24m, TIMER5);
GATE_CLK(timer6, xin24m, TIMER6);

/*********************power domain*******************************/
#if 1
#ifdef RK30_CLK_OFFBOARD_TEST
void pmu_set_power_domain_test(enum pmu_power_domain pd, bool on) {};
#define _pmu_set_power_domain pmu_set_power_domain_test//rk30_pmu_set_power_domain
#else
void pmu_set_power_domain(enum pmu_power_domain pd, bool on);
#define _pmu_set_power_domain pmu_set_power_domain
#endif

static int pd_video_mode(struct clk *clk, int on)
{
	u32 gate[3];
	gate[0] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VEPU));
	gate[1] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VDPU));
	//gate[2] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VCODEC));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_VEPU), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VEPU));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_VDPU), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VDPU));
	//cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_VCODEC), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VCODEC));
	pmu_set_power_domain(PD_VIDEO, on);
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_VEPU) | gate[0], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VEPU));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_VDPU) | gate[1], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VDPU));
	//cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_VCODEC) | gate[2], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VCODEC));
	return 0;
}

static struct clk pd_video = {
	.name	= "pd_video",
	.flags  = IS_PD,
	.mode	= pd_video_mode,
	.gate_idx	= PD_VIDEO,
};
static int pd_display_mode(struct clk *clk, int on)
{
	u32 gate[10];
	gate[0] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC0_SRC));
	gate[1] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC1_SRC));
	gate[2] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC0));
	gate[3] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC1));
	gate[4] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_CIF0));
	//gate[5] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_CIF1));
	gate[6] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VIO0));
	gate[7] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VIO1));
	gate[8] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_IPP));
	gate[9] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_RGA));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_LCDC0_SRC), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC0_SRC));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_LCDC1_SRC), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC1_SRC));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_LCDC0), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC0));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_LCDC1), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC1));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_CIF0), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_CIF0));
	//cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_CIF1), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_CIF1));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_VIO0), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VIO0));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_VIO1), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VIO1));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_IPP), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_IPP));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_RGA), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_RGA));
	pmu_set_power_domain(PD_VIO, on);
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_LCDC0_SRC) | gate[0], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC0_SRC));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_LCDC1_SRC) | gate[1], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC1_SRC));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_LCDC0) | gate[2], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC0));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_LCDC1) | gate[3], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_LCDC1));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_CIF0) | gate[4], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_CIF0));
	//cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_CIF1) | gate[5], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_CIF1));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_VIO0) | gate[6], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VIO0));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_VIO1) | gate[7], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_VIO1));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_IPP) | gate[8], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_IPP));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_RGA) | gate[9], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_RGA));
	return 0;
}

static struct clk pd_display = {
	.name	= "pd_vio",
	.flags  = IS_PD,
	.mode	= pd_display_mode,
	.gate_idx	= PD_VIO,
};
static struct clk pd_lcdc0 = {
	.parent	= &pd_display,
	.name	= "pd_lcdc0",
};
static struct clk pd_lcdc1 = {
	.parent	= &pd_display,
	.name	= "pd_lcdc1",
};
static struct clk pd_cif0 = {
	.parent	= &pd_display,
	.name	= "pd_cif0",
};
static struct clk pd_rga = {
	.parent	= &pd_display,
	.name	= "pd_rga",
};
static struct clk pd_ipp = {
	.parent	= &pd_display,
	.name	= "pd_ipp",
};
static struct clk pd_hdmi = {
	.parent	= &pd_display,
	.name	= "pd_hdmi",
};


static int pd_gpu_mode(struct clk *clk, int on)
{
	u32 gate[3];
	gate[0] = cru_readl(CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_GPU));
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_GPU), CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_GPU));
	pmu_set_power_domain(PD_GPU, on);
	cru_writel(CLK_GATE_W_MSK(CLK_GATE_ACLK_GPU) | gate[0], CLK_GATE_CLKID_CONS(CLK_GATE_ACLK_GPU));
	return 0;
}

static struct clk pd_gpu = {
	.name	= "pd_gpu",
	.flags  = IS_PD,
	.mode	= pd_gpu_mode,
	.gate_idx	= PD_GPU,
};

static int pm_off_mode(struct clk *clk, int on)
{
	_pmu_set_power_domain(clk->gate_idx, on); //on 1
	return 0;
}
static struct clk pd_peri = {
	.name	= "pd_peri",
	.flags	= IS_PD,
	.mode	= pm_off_mode,
	.gate_idx	= PD_PERI,
};


#define PD_CLK(name) \
{\
	.dev_id = NULL,\
	.con_id = #name,\
	.clk = &name,\
}

#endif
/************************rk30 fixed div clock****************************************/

/*************************aclk_cpu***********************/

GATE_CLK(dma1, 		aclk_cpu,	ACLK_DMAC1);
GATE_CLK(intmem, 	aclk_cpu,	ACLK_INTMEM);
GATE_CLK(aclk_strc_sys, aclk_cpu,	ACLK_STRC_SYS);

/*************************hclk_cpu***********************/

GATE_CLK(rom, 		hclk_cpu,	HCLK_ROM);
GATE_CLK(hclk_i2s0_2ch, hclk_cpu,	HCLK_I2S0_2CH);
GATE_CLK(hclk_spdif, 	hclk_cpu, 	HCLK_SPDIF);
GATE_CLK(hclk_cpubus, 	hclk_cpu, 	HCLK_CPUBUS);
GATE_CLK(hclk_ahb2apb,	hclk_cpu, 	HCLK_AHB2APB);
GATE_CLK(hclk_vio_bus,	hclk_cpu, 	HCLK_VIO_BUS);
GATE_CLK(hclk_lcdc0,	hclk_cpu, 	HCLK_LCDC0);
GATE_CLK(hclk_lcdc1,	hclk_cpu, 	HCLK_LCDC1);
GATE_CLK(hclk_cif0,	hclk_cpu, 	HCLK_CIF0);
GATE_CLK(hclk_ipp,	hclk_cpu, 	HCLK_IPP);
GATE_CLK(hclk_rga,	hclk_cpu, 	HCLK_RGA);
GATE_CLK(hclk_imem0,	hclk_cpu, 	HCLK_IMEM0);
GATE_CLK(hclk_imem1,	hclk_cpu, 	HCLK_IMEM1);

/*************************ahb2apb_cpu(pclk_cpu_h2p)***********************/
GATE_CLK(pclk_uart0,	ahb2apb_cpu, PCLK_UART0);
GATE_CLK(pclk_uart1,	ahb2apb_cpu, PCLK_UART1);
/*************************pclk_cpu***********************/
GATE_CLK(pwm01,		pclk_cpu, 	PCLK_PWM01);//pwm 0��1
GATE_CLK(pclk_timer0,	pclk_cpu, 	PCLK_TIMER0);
GATE_CLK(pclk_timer2,	pclk_cpu, 	PCLK_TIMER2);
GATE_CLK(i2c0,	pclk_cpu, PCLK_I2C0);
GATE_CLK(i2c1,	pclk_cpu, PCLK_I2C1);
GATE_CLK(gpio0,	pclk_cpu, PCLK_GPIO0);
GATE_CLK(gpio1,	pclk_cpu, PCLK_GPIO1);
GATE_CLK(gpio2,	pclk_cpu, PCLK_GPIO2);
GATE_CLK(efuse,	pclk_cpu, PCLK_EFUSE);
GATE_CLK(tzpc,	pclk_cpu, PCLK_TZPC);
GATE_CLK(pclk_ddrupctl,	pclk_cpu, PCLK_DDRUPCTL);
GATE_CLK(pclk_ddrpubl,	pclk_cpu, PCLK_PUBL);
GATE_CLK(dbg,	pclk_cpu, PCLK_DBG);
GATE_CLK(grf,	pclk_cpu, PCLK_GRF);
GATE_CLK(pmu,	pclk_cpu, PCLK_PMU);

/*************************aclk_periph***********************/

GATE_CLK(dma2, aclk_periph, ACLK_DMAC2);
GATE_CLK(aclk_smc, aclk_periph, ACLK_SMC);
GATE_CLK(aclk_gps, aclk_periph, ACLK_GPS);
GATE_CLK(aclk_peri_niu, aclk_periph, ACLK_PEI_NIU);
GATE_CLK(aclk_cpu_peri, aclk_periph, ACLK_CPU_PERI);
GATE_CLK(aclk_peri_axi_matrix, aclk_periph, ACLK_PERI_AXI_MATRIX);

/*************************hclk_periph***********************/
GATE_CLK(hclk_peri_axi_matrix, hclk_periph, HCLK_PERI_AXI_MATRIX);
GATE_CLK(hclk_peri_ahb_arbi, hclk_periph, HCLK_PERI_AHB_ARBI);
GATE_CLK(hclk_emem_peri, hclk_periph, HCLK_EMEM_PERI);
GATE_CLK(hclk_mac, hclk_periph, HCLK_EMAC);
GATE_CLK(nandc, hclk_periph, HCLK_NANDC);
GATE_CLK(hclk_usb_peri, hclk_periph, HCLK_USB_PERI);
GATE_CLK(hclk_otg0, clk_hclk_usb_peri, HCLK_OTG0);
GATE_CLK(hclk_otg1, clk_hclk_usb_peri, HCLK_OTG1);
GATE_CLK(hclk_hsic, hclk_periph, HCLK_HSIC);
GATE_CLK(hclk_hsadc, hclk_periph, HCLK_HSADC);
GATE_CLK(hclk_pidfilter, hclk_periph, HCLK_PIDF);
GATE_CLK(hclk_sdmmc, hclk_periph, HCLK_SDMMC0);
GATE_CLK(hclk_sdio, hclk_periph, HCLK_SDIO);
GATE_CLK(hclk_emmc, hclk_periph, HCLK_EMMC);
/*************************pclk_periph***********************/
GATE_CLK(pclk_peri_axi_matrix, pclk_periph, PCLK_PERI_AXI_MATRIX);
GATE_CLK(pwm23, pclk_periph, PCLK_PWM23);
GATE_CLK(wdt, pclk_periph, PCLK_WDT);
GATE_CLK(pclk_spi0, pclk_periph, PCLK_SPI0);
GATE_CLK(pclk_spi1, pclk_periph, PCLK_SPI1);
GATE_CLK(pclk_uart2, pclk_periph, PCLK_UART2);
GATE_CLK(pclk_uart3, pclk_periph, PCLK_UART3);
GATE_CLK(i2c2, pclk_periph, PCLK_I2C2);
GATE_CLK(i2c3, pclk_periph, PCLK_I2C3);
GATE_CLK(i2c4, pclk_periph, PCLK_I2C4);
GATE_CLK(gpio3, pclk_periph, PCLK_GPIO3);
GATE_CLK(pclk_saradc, pclk_periph, PCLK_SARADC);
/*************************aclk_lcdc0***********************/

GATE_CLK(aclk_vio0, aclk_lcdc0_pre, ACLK_VIO0);
GATE_CLK(aclk_lcdc0, clk_aclk_vio0, ACLK_LCDC0);
GATE_CLK(aclk_cif0, clk_aclk_vio0, ACLK_CIF0);
GATE_CLK(aclk_ipp,  clk_aclk_vio0, ACLK_IPP);

/*************************aclk_lcdc0***********************/

GATE_CLK(aclk_vio1, aclk_lcdc1_pre, ACLK_VIO1);
GATE_CLK(aclk_lcdc1, clk_aclk_vio1, ACLK_LCDC1);
GATE_CLK(aclk_rga,  clk_aclk_vio1, ACLK_RGA);


#if 1
#define CLK(dev, con, ck) \
{\
	.dev_id = dev,\
	.con_id = con,\
	.clk = ck,\
}


#define CLK1(name) \
{\
	.dev_id = NULL,\
	.con_id = #name,\
	.clk = &clk_##name,\
}

#endif

static struct clk_lookup clks[] = {
	CLK(NULL, "xin24m", &xin24m),
	//CLK(NULL, "xin27m", &xin27m),
	CLK(NULL, "xin12m", &clk_12m),
	CLK(NULL, "arm_pll", &arm_pll_clk),
	CLK(NULL, "ddr_pll", &ddr_pll_clk),
	CLK(NULL, "codec_pll", &codec_pll_clk),
	CLK(NULL, "general_pll", &general_pll_clk),

	CLK(NULL, "arm_gpll", &clk_core_gpll_path),
	CLK(NULL, "ddr_gpll", &clk_ddr_gpll_path),
	
	CLK(NULL, "ddr", &clk_ddr),
	CLK(NULL, "cpu", &clk_core),
	CLK1(l2c),
	CLK1(core_dbg),
	CLK("smp_twd", NULL, &core_periph),
	CLK(NULL, "aclk_core", &aclk_core),

	CLK(NULL, "logic", &clk_cpu_div),
	CLK(NULL, "aclk_cpu", &aclk_cpu),
	CLK(NULL, "pclk_cpu", &pclk_cpu),
	CLK(NULL, "atclk_cpu", &atclk_cpu),
	CLK(NULL, "hclk_cpu", &hclk_cpu),
	CLK(NULL, "ahb2apb_cpu", &ahb2apb_cpu),

	CLK(NULL, "gpu", 	&aclk_gpu),

	CLK(NULL, "aclk_vepu",	&aclk_vepu),
	CLK(NULL, "hclk_vepu", 	&hclk_vepu),
	CLK(NULL, "aclk_vdpu", 	&aclk_vdpu),
	CLK(NULL, "hclk_vdpu", 	&hclk_vdpu),

	CLK(NULL, "aclk_lcdc0_pre", &aclk_lcdc0_pre),
	CLK(NULL, "aclk_lcdc1_pre", &aclk_lcdc1_pre),

	CLK(NULL, "aclk_periph", &aclk_periph),
	CLK(NULL, "pclk_periph", &pclk_periph),
	CLK(NULL, "hclk_periph", &hclk_periph),

	CLK(NULL, "dclk_lcdc0", &dclk_lcdc0),
	CLK(NULL, "dclk_lcdc1", &dclk_lcdc1),
	
	CLK(NULL, "cif_out_pll", &cif_out_pll),
	CLK(NULL, "cif0_out_div", &cif0_out_div),
	CLK(NULL, "cif0_out", &cif0_out),

	CLK(NULL, "pclkin_cif0", &pclkin_cif0),
	CLK(NULL, "inv_cif0", &inv_cif0),
	CLK(NULL, "cif0_in", &cif0_in),

	CLK1(i2s_pll),
	CLK("rk29_i2s.0", "i2s_div", &clk_i2s0_div),
	CLK("rk29_i2s.0", "i2s_frac_div", &clk_i2s0_frac_div),
	CLK("rk29_i2s.0", "i2s", &clk_i2s0),
	
	// actually no i2s1
	CLK("rk29_i2s.1", "i2s_div", &clk_i2s0_div),
	CLK("rk29_i2s.1", "i2s_frac_div", &clk_i2s0_frac_div),
	CLK("rk29_i2s.1", "i2s", &clk_i2s0),


	CLK1(spdif_div),
	CLK1(spdif_frac_div),
	CLK1(spdif),

	CLK1(otgphy0),
	CLK1(otgphy1),
	CLK1(otgphy0_480m),
	CLK1(otgphy1_480m),
	CLK1(hsicphy_480m),
	CLK1(hsicphy_12m),
	
	CLK(NULL, "rmii_clkin", &rmii_clkin),
	CLK(NULL, "mac_ref_div", &clk_mac_pll_div), // compatible with rk29
	CLK1(mac_ref),
	CLK1(mii_tx),

	CLK1(hsadc_pll_div),
	CLK1(hsadc_frac_div),
	CLK1(hsadc_ext),
	CLK1(hsadc_out),
	CLK1(hsadc_out_inv),
	CLK1(hsadc),

	CLK1(saradc),
	
	CLK1(smc),
	CLK(NULL, "smc_inv",	&clkn_smc),

	CLK("rk29xx_spim.0", "spi", &clk_spi0),
	CLK("rk29xx_spim.1", "spi", &clk_spi1),

	CLK("rk29_sdmmc.0", "mmc", &clk_sdmmc),
	CLK("rk29_sdmmc.1", "mmc", &clk_sdio),
	CLK1(emmc),

	CLK1(uart_pll),
	CLK("rk_serial.0", "uart_div", &clk_uart0_div),
	CLK("rk_serial.0", "uart_frac_div", &clk_uart0_frac_div),
	CLK("rk_serial.0", "uart", &clk_uart0),
	CLK("rk_serial.1", "uart_div", &clk_uart1_div),
	CLK("rk_serial.1", "uart_frac_div", &clk_uart1_frac_div),
	CLK("rk_serial.1", "uart", &clk_uart1),
	CLK("rk_serial.2", "uart_div", &clk_uart2_div),
	CLK("rk_serial.2", "uart_frac_div", &clk_uart2_frac_div),
	CLK("rk_serial.2", "uart", &clk_uart2),
	CLK("rk_serial.3", "uart_div", &clk_uart3_div),
	CLK("rk_serial.3", "uart_frac_div", &clk_uart3_frac_div),
	CLK("rk_serial.3", "uart", &clk_uart3),

	CLK1(timer0),
	CLK1(timer1),
	CLK1(timer2),
	CLK1(timer3),
	CLK1(timer4),
	CLK1(timer5),
	CLK1(timer6),

	/*************************aclk_cpu***********************/
	CLK1(dma1),
	CLK1(intmem),
	CLK1(aclk_strc_sys),
	
	/*************************hclk_cpu***********************/
	CLK1(rom),
	CLK("rk29_i2s.0", "hclk_i2s", &clk_hclk_i2s0_2ch),
	// actually no i2s1
	CLK("rk29_i2s.1", "hclk_i2s", &clk_hclk_i2s0_2ch),
	CLK1(hclk_spdif),
	CLK1(hclk_cpubus),
	CLK1(hclk_ahb2apb),
	CLK1(hclk_vio_bus),
	CLK1(hclk_lcdc0),
	CLK1(hclk_lcdc1),
	CLK1(hclk_cif0),
	CLK1(hclk_ipp),
	CLK1(hclk_rga),
	CLK1(hclk_imem0),
	CLK1(hclk_imem1),

	/*************************pclk_cpu***********************/
	CLK1(pwm01),
	CLK1(pclk_timer0),
	CLK1(pclk_timer2),
	CLK("rk30_i2c.0", "i2c", &clk_i2c0),
	CLK("rk30_i2c.1", "i2c", &clk_i2c1),
	CLK1(gpio0),
	CLK1(gpio1),
	CLK1(gpio2),
	CLK1(efuse),
	CLK1(tzpc),
	CLK("rk_serial.0", "pclk_uart", &clk_pclk_uart0),
	CLK("rk_serial.1", "pclk_uart", &clk_pclk_uart1),
	CLK1(pclk_ddrupctl),
	CLK1(pclk_ddrpubl),
	CLK1(dbg),
	CLK1(grf),
	CLK1(pmu),

	/*************************aclk_periph***********************/
	CLK1(dma2),
	CLK1(aclk_smc),
	CLK1(aclk_gps),
	CLK1(aclk_peri_niu),
	CLK1(aclk_cpu_peri),
	CLK1(aclk_peri_axi_matrix),

	/*************************hclk_periph***********************/
	CLK1(hclk_peri_axi_matrix),
	CLK1(hclk_peri_ahb_arbi),
	CLK1(hclk_emem_peri),
	CLK1(hclk_mac),
	CLK1(nandc),
	CLK1(hclk_usb_peri),
	CLK1(hclk_otg0),
	CLK1(hclk_otg1),
	CLK1(hclk_hsic),
	CLK1(hclk_hsadc),
	CLK1(hclk_pidfilter),
	CLK("rk29_sdmmc.0", "hclk_mmc", &clk_hclk_sdmmc),
	CLK("rk29_sdmmc.1", "hclk_mmc", &clk_hclk_sdio),
	CLK1(hclk_emmc),

	/*************************pclk_periph***********************/
	CLK1(pclk_peri_axi_matrix),
	CLK1(pwm23),
	CLK1(wdt),
	CLK("rk29xx_spim.0", "pclk_spi", &clk_pclk_spi0),
	CLK("rk29xx_spim.1", "pclk_spi", &clk_pclk_spi1),
	CLK("rk_serial.2", "pclk_uart", &clk_pclk_uart2),
	CLK("rk_serial.3", "pclk_uart", &clk_pclk_uart3),
	CLK("rk30_i2c.2", "i2c", &clk_i2c2),
	CLK("rk30_i2c.3", "i2c", &clk_i2c3),
	CLK("rk30_i2c.4", "i2c", &clk_i2c4),
	CLK1(gpio3),
	CLK1(pclk_saradc),

	/*************************aclk_lcdc0***********************/
	CLK1(aclk_vio0),
	CLK(NULL, "aclk_lcdc0",	&clk_aclk_lcdc0),
	CLK1(aclk_cif0),
	CLK1(aclk_ipp),

	/*************************aclk_lcdc1***********************/
	CLK1(aclk_vio1),
	CLK(NULL, "aclk_lcdc1",	&clk_aclk_lcdc1),
	CLK1(aclk_rga),
	/************************power domain**********************/
	
	PD_CLK(pd_peri),
	PD_CLK(pd_display),
	PD_CLK(pd_lcdc0),
	PD_CLK(pd_lcdc1),
	PD_CLK(pd_cif0),
	//PD_CLK(pd_cif1),
	PD_CLK(pd_rga),
	PD_CLK(pd_ipp),
	//PD_CLK(pd_video),
	PD_CLK(pd_gpu),
	//PD_CLK(pd_dbg),
};
static void __init rk30_init_enable_clocks(void)
{
	#if 0
	//clk_enable_nolock(&xin24m);
	//clk_enable_nolock(&clk_12m);
	//clk_enable_nolock(&arm_pll_clk);
	//clk_enable_nolock(&ddr_pll_clk);
	//clk_enable_nolock(&codec_pll_clk);
	//clk_enable_nolock(&general_pll_clk);
	#endif
	clk_enable_nolock(&clk_ddr);
	//clk_enable_nolock(&clk_core);
	clk_enable_nolock(&clk_cpu_div);
	clk_enable_nolock(&clk_core_gpll_path);
	clk_enable_nolock(&clk_ddr_gpll_path);
	clk_enable_nolock(&clk_l2c);
	clk_enable_nolock(&clk_core_dbg);
	clk_enable_nolock(&core_periph);
	clk_enable_nolock(&aclk_core);
	//clk_enable_nolock(&aclk_cpu);
	//clk_enable_nolock(&pclk_cpu);
	clk_enable_nolock(&atclk_cpu);
	//clk_enable_nolock(&hclk_cpu);
	clk_enable_nolock(&ahb2apb_cpu);
	if (flag_uboot_display) {
		clk_enable_nolock(&dclk_lcdc0);
		clk_enable_nolock(&dclk_lcdc1);
		clk_enable_nolock(&clk_hclk_lcdc0);
		clk_enable_nolock(&clk_hclk_lcdc1);
		clk_enable_nolock(&clk_aclk_lcdc0);
		clk_enable_nolock(&clk_aclk_lcdc1);
		clk_enable_nolock(&aclk_lcdc0_pre);
		clk_enable_nolock(&aclk_lcdc1_pre);
		clk_enable_nolock(&pd_lcdc0);
		clk_enable_nolock(&pd_lcdc1);
	}
	#if 0
	 clk_enable_nolock(&clk_gpu);
	 clk_enable_nolock(&aclk_gpu);
	 clk_enable_nolock(&aclk_gpu_slv);
	 clk_enable_nolock(&aclk_gpu_mst);

	 clk_enable_nolock(&aclk_vepu);
	 clk_enable_nolock(&hclk_vepu);
	 clk_enable_nolock(&aclk_vdpu);
	 clk_enable_nolock(&hclk_vdpu);


	 clk_enable_nolock(&aclk_periph);
	clk_enable_nolock(&pclk_periph);
	clk_enable_nolock(&hclk_periph);
	#endif
	#if 0
	 clk_enable_nolock(&cif_out_pll);
	 clk_enable_nolock(&cif0_out_div);

	 clk_enable_nolock(&cif0_out);
	 clk_enable_nolock(&pclkin_cif0);
	 clk_enable_nolock(&inv_cif0);
	 clk_enable_nolock(&cif0_in);

	 clk_enable_nolock(&clk_i2s_pll);
	 clk_enable_nolock(&clk_i2s0_div);
	 clk_enable_nolock(&clk_i2s0_frac_div);
	 clk_enable_nolock(&clk_i2s0);
	
	  actually no i2s1
	 clk_enable_nolock(&clk_i2s0_div);
	 clk_enable_nolock(&clk_i2s0_frac_div);
	 clk_enable_nolock(&clk_i2s0);

	clk_enable_nolock(&clk_spdif_div);
	clk_enable_nolock(&clk_spdif_frac_div);
	clk_enable_nolock(&clk_spdif);
	#endif
	#if 0
	clk_enable_nolock(&clk_otgphy0);
	clk_enable_nolock(&clk_otgphy1);
	clk_enable_nolock(&clk_otgphy0_480m);
	clk_enable_nolock(&clk_otgphy1_480m);
	clk_enable_nolock(&clk_hsicphy_480m);
	clk_enable_nolock(&clk_hsicphy_12m);
	#endif	
	
	#if 0
	clk_enable_nolock(&rmii_clkin);
	clk_enable_nolock(&clk_mac_pll_div); // compatible with rk29
	clk_enable_nolock(&clk_mac_ref);
	clk_enable_nolock(&clk_mii_tx);	
	#endif

	#if 0
	clk_enable_nolock(&clk_hsadc_pll_div);
	clk_enable_nolock(&clk_hsadc_frac_div);
	clk_enable_nolock(&clk_hsadc_ext);
	clk_enable_nolock(&clk_hsadc_out);
	clk_enable_nolock(&clk_hsadc_out_inv);
	clk_enable_nolock(&clk_hsadc);

	clk_enable_nolock(&clk_saradc);
	#endif
	/*
	clk_enable_nolock(&clk_smc);
	clk_enable_nolock(&clkn_smc);
	*/
	/*
	clk_enable_nolock(&clk_spi0);
	clk_enable_nolock(&clk_spi1);
	*/
	/*
	clk_enable_nolock(&clk_sdmmc);
	clk_enable_nolock(&clk_sdio);
	clk_enable_nolock(&clk_emmc);
	*/
	#if 0
	clk_enable_nolock(&clk_uart_pll);
	clk_enable_nolock(&clk_uart0_div);
	clk_enable_nolock(&clk_uart0_frac_div);
	clk_enable_nolock(&clk_uart0);
	clk_enable_nolock(&clk_uart1_div);
	clk_enable_nolock(&clk_uart1_frac_div);
	clk_enable_nolock(&clk_uart1);
	clk_enable_nolock(&clk_uart2_div);
	clk_enable_nolock(&clk_uart2_frac_div);
	clk_enable_nolock(&clk_uart2);
	clk_enable_nolock(&clk_uart3_div);
	clk_enable_nolock(&clk_uart3_frac_div);
	clk_enable_nolock(&clk_uart3);
	#endif
	#if CONFIG_RK_DEBUG_UART == 0
		clk_enable_nolock(&clk_uart0);
		clk_enable_nolock(&clk_pclk_uart0);
	#elif CONFIG_RK_DEBUG_UART == 1
		clk_enable_nolock(&clk_uart1);
		clk_enable_nolock(&clk_pclk_uart1);

	#elif CONFIG_RK_DEBUG_UART == 2
		clk_enable_nolock(&clk_uart2);
		clk_enable_nolock(&clk_pclk_uart2);

	#elif CONFIG_RK_DEBUG_UART == 3
		clk_enable_nolock(&clk_uart3);
		clk_enable_nolock(&clk_pclk_uart3);

	#endif
	#if 0
	clk_enable_nolock(&clk_timer0);
	clk_enable_nolock(&clk_timer1);
	clk_enable_nolock(&clk_timer2);
	#endif
	
	/*************************aclk_cpu***********************/
	clk_enable_nolock(&clk_dma1);
	clk_enable_nolock(&clk_intmem);
	clk_enable_nolock(&clk_aclk_strc_sys);
	
	/*************************hclk_cpu***********************/
	clk_enable_nolock(&clk_rom);
	#if 0
	clk_enable_nolock(&clk_hclk_i2s0_2ch);
	// actually no i2s1
	clk_enable_nolock(&clk_hclk_i2s0_2ch);
	clk_enable_nolock(&clk_hclk_spdif);
	#endif
	clk_enable_nolock(&clk_hclk_cpubus);
	clk_enable_nolock(&clk_hclk_ahb2apb);
	clk_enable_nolock(&clk_hclk_vio_bus);
	#if 0
	clk_enable_nolock(&clk_hclk_cif0);
	clk_enable_nolock(&clk_hclk_ipp);
	clk_enable_nolock(&clk_hclk_rga);
	#endif
	clk_enable_nolock(&clk_hclk_imem0);
	clk_enable_nolock(&clk_hclk_imem1);

	/*************************pclk_cpu***********************/
	#if 0
	clk_enable_nolock(&clk_pwm01);
	clk_enable_nolock(&clk_pclk_timer0);
	clk_enable_nolock(&clk_pclk_timer1);
	clk_enable_nolock(&clk_pclk_timer2);
	clk_enable_nolock(&clk_i2c0);
	clk_enable_nolock(&clk_i2c1);
	clk_enable_nolock(&clk_gpio0);
	clk_enable_nolock(&clk_gpio1);
	clk_enable_nolock(&clk_gpio2);
	clk_enable_nolock(&clk_efuse);
	#endif
	clk_enable_nolock(&clk_tzpc);
	//clk_enable_nolock(&clk_pclk_uart0);
	//clk_enable_nolock(&clk_pclk_uart1);
	clk_enable_nolock(&clk_pclk_ddrupctl);
	clk_enable_nolock(&clk_pclk_ddrpubl);
	clk_enable_nolock(&clk_dbg);
	clk_enable_nolock(&clk_grf);
	clk_enable_nolock(&clk_pmu);

	/*************************aclk_periph***********************/
	clk_enable_nolock(&clk_dma2);
	clk_enable_nolock(&clk_aclk_smc);
	clk_enable_nolock(&clk_aclk_peri_niu);
	clk_enable_nolock(&clk_aclk_cpu_peri);
	clk_enable_nolock(&clk_aclk_peri_axi_matrix);

	/*************************hclk_periph***********************/
	clk_enable_nolock(&clk_hclk_peri_axi_matrix);
	clk_enable_nolock(&clk_hclk_peri_ahb_arbi);
	clk_enable_nolock(&clk_hclk_emem_peri);
	//clk_enable_nolock(&clk_hclk_mac);
	clk_enable_nolock(&clk_nandc);
	clk_enable_nolock(&clk_hclk_usb_peri);
	#if 0
	clk_enable_nolock(&clk_hclk_otg0);
	clk_enable_nolock(&clk_hclk_otg1);
	clk_enable_nolock(&clk_hclk_hsic);
	clk_enable_nolock(&clk_hclk_gps);
	clk_enable_nolock(&clk_hclk_hsadc);
	clk_enable_nolock(&clk_hclk_pidfilter);
	clk_enable_nolock(&clk_hclk_sdmmc);
	clk_enable_nolock(&clk_hclk_sdio);
	clk_enable_nolock(&clk_hclk_emmc);
	#endif

	/*************************pclk_periph***********************/
	clk_enable_nolock(&clk_pclk_peri_axi_matrix);
	#if 0
	clk_enable_nolock(&clk_pwm23);
	clk_enable_nolock(&clk_wdt);
	clk_enable_nolock(&clk_pclk_spi0);
	clk_enable_nolock(&clk_pclk_spi1);
	clk_enable_nolock(&clk_pclk_uart2);
	clk_enable_nolock(&clk_pclk_uart3);
	#endif
	#if 0
	clk_enable_nolock(&clk_i2c2);
	clk_enable_nolock(&clk_i2c3);
	clk_enable_nolock(&clk_i2c4);
	clk_enable_nolock(&clk_gpio3);
	clk_enable_nolock(&clk_pclk_saradc);
	#endif
	/*************************aclk_lcdc0***********************/
#if 1
	//clk_enable_nolock(&clk_aclk_vio0);
	//clk_enable_nolock(&clk_aclk_cif0);
	//clk_enable_nolock(&clk_aclk_ipp);
#endif
	/*************************aclk_lcdc1***********************/
#if 1
	//clk_enable_nolock(&clk_aclk_vio1);
	//clk_enable_nolock(&clk_aclk_rga);
#endif
	/************************power domain**********************/

}
static void periph_clk_set_init(void)
{
	unsigned long aclk_p, hclk_p, pclk_p;
	unsigned long ppll_rate = general_pll_clk.rate;
	//aclk 148.5

	/* general pll */
	switch (ppll_rate) {
		case 148500 * KHZ:
			aclk_p = 148500 * KHZ;
			hclk_p = aclk_p >> 1;
			pclk_p = aclk_p >> 2;
			break;
		case 297 * MHZ:
			aclk_p = ppll_rate >> 1;
			hclk_p = aclk_p >> 0;
			pclk_p = aclk_p >> 1;
			break;
		case 300 * MHZ:
			aclk_p = ppll_rate >> 1;
			hclk_p = aclk_p >> 0;
			pclk_p = aclk_p >> 1;
			break;
		case 384 * MHZ: 
			aclk_p = ppll_rate >> 1; 
			hclk_p = aclk_p >> 1; 
			pclk_p = aclk_p >> 2; 
			break; 
		case 594 * MHZ:
			aclk_p = ppll_rate >> 2;
			hclk_p = aclk_p >> 0;
			pclk_p = aclk_p >> 1;
			break;
		case 768 * MHZ: 
			aclk_p = ppll_rate >> 2; 
			hclk_p = aclk_p >> 1; 
			pclk_p = aclk_p >> 2; 
			break; 
		case 891 * MHZ:
			aclk_p = ppll_rate / 6;
			hclk_p = aclk_p >> 0;
			pclk_p = aclk_p >> 1;
			break;
		case 1188 * MHZ:
			aclk_p = ppll_rate >> 3;
			hclk_p = aclk_p >> 0;
			pclk_p = aclk_p >> 1;
			break;

		default:
			aclk_p = 150 * MHZ;
			hclk_p = 150 * MHZ;
			pclk_p = 75 * MHZ;
			break;
	}
	clk_set_parent_nolock(&aclk_periph, &general_pll_clk);
	clk_set_rate_nolock(&aclk_periph, aclk_p);
	clk_set_rate_nolock(&hclk_periph, hclk_p);
	clk_set_rate_nolock(&pclk_periph, pclk_p);
}

static void cpu_axi_init(void)
{
	unsigned long cpu_div_rate = 0, aclk_cpu_rate = 0, hclk_cpu_rate = 0,
		      pclk_cpu_rate = 0, ahb2apb_cpu_rate = 0;
	unsigned long gpll_rate = general_pll_clk.rate;

	switch (gpll_rate) {
		case 297 * MHZ:
			cpu_div_rate = gpll_rate;
			aclk_cpu_rate = cpu_div_rate >> 0;
			hclk_cpu_rate = aclk_cpu_rate >> 1;
			pclk_cpu_rate = aclk_cpu_rate >> 2;
			break;

		case 384 * MHZ:
			cpu_div_rate = gpll_rate >> 1;
			aclk_cpu_rate = cpu_div_rate >> 0;
			hclk_cpu_rate = aclk_cpu_rate >> 1;
			pclk_cpu_rate = aclk_cpu_rate >> 2;
			break;

		case 594 * MHZ:
			cpu_div_rate = gpll_rate >> 1;
			aclk_cpu_rate = cpu_div_rate >> 0;
			hclk_cpu_rate = aclk_cpu_rate >> 1;
			pclk_cpu_rate = aclk_cpu_rate >> 2;
			break;

		case 768 * MHZ:
			cpu_div_rate = gpll_rate >> 2;
			aclk_cpu_rate = cpu_div_rate >> 0;
			hclk_cpu_rate = aclk_cpu_rate >> 1;
			pclk_cpu_rate = aclk_cpu_rate >> 2;
			break;

		case 891 * MHZ:
			cpu_div_rate = gpll_rate / 3;
			aclk_cpu_rate = cpu_div_rate >> 0;
			hclk_cpu_rate = aclk_cpu_rate >> 1;
			pclk_cpu_rate = aclk_cpu_rate >> 2;
			break;

		case 1188 * MHZ:
			cpu_div_rate = gpll_rate >> 2;
			aclk_cpu_rate = cpu_div_rate >> 0;
			hclk_cpu_rate = aclk_cpu_rate >> 1;
			pclk_cpu_rate = aclk_cpu_rate >> 2;
			break;

		default:
			aclk_cpu_rate = 150 * MHZ;
			hclk_cpu_rate = 150 * MHZ;
			pclk_cpu_rate = 75 * MHZ;
			break;
	}
	ahb2apb_cpu_rate = pclk_cpu_rate;

	clk_set_parent_nolock(&clk_cpu_div, &general_pll_clk);
	clk_set_rate_nolock(&clk_cpu_div, cpu_div_rate);
	clk_set_rate_nolock(&aclk_cpu, aclk_cpu_rate);
	clk_set_rate_nolock(&hclk_cpu, hclk_cpu_rate);
	clk_set_rate_nolock(&pclk_cpu, pclk_cpu_rate);
	clk_set_rate_nolock(&ahb2apb_cpu, ahb2apb_cpu_rate);
}

void rk30_clock_common_i2s_init(void)
{
	unsigned long i2s_rate;
	//20 times
	if(rk30_clock_flags & CLK_FLG_MAX_I2S_49152KHZ) {
		i2s_rate = 49152000;
	} else if(rk30_clock_flags & CLK_FLG_MAX_I2S_24576KHZ) {
		i2s_rate = 24576000;
	} else if(rk30_clock_flags & CLK_FLG_MAX_I2S_22579_2KHZ) {
		i2s_rate = 22579000;
	} else if(rk30_clock_flags & CLK_FLG_MAX_I2S_12288KHZ) {
		i2s_rate = 12288000;
	} else {
		i2s_rate = 49152000;
	}

	/*
	 * Priority setting i2s under cpll to fix i2s frac div do not effect, let
	 * axi_cpu's pll different with i2s's
	 * */
	if(((i2s_rate * 20) <= codec_pll_clk.rate) || !(codec_pll_clk.rate % i2s_rate)) {
		clk_set_parent_nolock(&clk_i2s_pll, &codec_pll_clk);
	} else if(((i2s_rate * 20) <= general_pll_clk.rate) || !(general_pll_clk.rate % i2s_rate)) {
		clk_set_parent_nolock(&clk_i2s_pll, &general_pll_clk);
	} else {
		if(general_pll_clk.rate > codec_pll_clk.rate)
			clk_set_parent_nolock(&clk_i2s_pll, &general_pll_clk);
		else
			clk_set_parent_nolock(&clk_i2s_pll, &codec_pll_clk);
	}
}
void rk30_clock_common_uart_init(struct clk *cpll_clk,struct clk *gpll_clk)
{
	struct clk *p_clk;
	unsigned long rate;
	if(!(gpll_clk->rate%(48*MHZ)))
	{
		p_clk=gpll_clk;
		rate=48*MHZ;
	}
	else if(!(cpll_clk->rate%(48*MHZ)))
	{
		p_clk=cpll_clk;
		rate=48*MHZ;
	}
	else if(!(gpll_clk->rate%(49500*KHZ)))
	{
		p_clk=gpll_clk;
		rate=(49500*KHZ);
	}
	else if(!(cpll_clk->rate%(49500*KHZ)))
	{
		p_clk=cpll_clk;
		rate=(49500*KHZ);
	}
	else
	{
		if(cpll_clk->rate>gpll_clk->rate)
		{
			p_clk=cpll_clk;
		}
		else
		{
			p_clk=gpll_clk;
		}	
		rate=50*MHZ;
	}
	clk_set_parent_nolock(&clk_uart_pll, p_clk);
	clk_set_rate_nolock(&clk_uart0_div,rate);
	clk_set_rate_nolock(&clk_uart1_div,rate);
	clk_set_rate_nolock(&clk_uart2_div,rate);
	clk_set_rate_nolock(&clk_uart3_div,rate);
}

static void inline clock_set_div(struct clk *clk,u32 div)
{
	set_cru_bits_w_msk(div - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
}

static void inline clock_set_max_div(struct clk *clk)
{
	set_cru_bits_w_msk(clk->div_max - 1, clk->div_mask, clk->div_shift, clk->clksel_con);
}

static void div_clk_for_pll_init(void)
{
	clock_set_max_div(&clk_cpu_div);
	clock_set_max_div(&aclk_vdpu);
	clock_set_max_div(&aclk_vepu);
	clock_set_max_div(&aclk_gpu);
	if (!flag_uboot_display) {
		clock_set_max_div(&aclk_lcdc0_pre);
		clock_set_max_div(&aclk_lcdc1_pre);
		clock_set_max_div(&dclk_lcdc0);
		clock_set_max_div(&dclk_lcdc1);
	}
	clock_set_max_div(&aclk_periph);
	clock_set_max_div(&cif0_out_div);
	clock_set_max_div(&clk_i2s0_div);
	clock_set_max_div(&clk_spdif_div);
	clock_set_max_div(&clk_uart0_div);
	clock_set_max_div(&clk_uart1_div);
	clock_set_max_div(&clk_uart2_div);
	clock_set_max_div(&clk_uart3_div);
	clock_set_max_div(&clk_hsicphy_12m);
	clock_set_max_div(&clk_hsadc_pll_div);
	clock_set_max_div(&clk_saradc);
}

static u8 pll_flag = 0;

static void __init rk30_clock_common_init(unsigned long gpll_rate, unsigned long cpll_rate)
{
	//general
	if (!flag_uboot_display)
		clk_set_rate_nolock(&general_pll_clk, gpll_rate);
	lpj_gpll = CLK_LOOPS_RECALC(general_pll_clk.rate);

	//code pll
	if (!flag_uboot_display)
		clk_set_rate_nolock(&codec_pll_clk, cpll_rate);

	cpu_axi_init();
	clk_set_rate_nolock(&clk_core, 816 * MHZ);
	//periph clk
	periph_clk_set_init();

	//i2s
	rk30_clock_common_i2s_init();

	// spi
	clk_set_rate_nolock(&clk_spi0, clk_spi0.parent->rate);
	clk_set_rate_nolock(&clk_spi1, clk_spi1.parent->rate);

	// uart
	rk30_clock_common_uart_init(&codec_pll_clk,&general_pll_clk);

	//mac
	if(!(gpll_rate % (50 * MHZ))) {
		clk_set_parent_nolock(&clk_mac_pll_div, &general_pll_clk);

	} else if((!(ddr_pll_clk.rate % (50 * MHZ))) && (ddr_pll_clk.rate != 24 * MHZ) && ((pll_flag & 0x2) == 0)) {
		clk_set_parent_nolock(&clk_mac_pll_div, &ddr_pll_clk);

	} else {
		CLKDATA_DBG("mac can't get 50mhz, set to gpll\n");
		clk_set_parent_nolock(&clk_mac_pll_div, &general_pll_clk);
	}

	//hsadc
	//auto pll sel
	//clk_set_parent_nolock(&clk_hsadc_pll_div, &general_pll_clk);

	if (!flag_uboot_display) {
		//lcdc0 lcd auto sel pll
		clk_set_parent_nolock(&dclk_lcdc0, &general_pll_clk);
		clk_set_parent_nolock(&dclk_lcdc1, &general_pll_clk);

		//axi lcdc auto sel
		clk_set_parent_nolock(&aclk_lcdc0_pre, &general_pll_clk);
		clk_set_parent_nolock(&aclk_lcdc1_pre, &general_pll_clk);
		clk_set_rate_nolock(&aclk_lcdc0_pre, 300 * MHZ);
		clk_set_rate_nolock(&aclk_lcdc1_pre, 300 * MHZ);
	}
	//cif
	clk_set_parent_nolock(&cif_out_pll, &general_pll_clk);

	//axi vepu auto sel
	//clk_set_parent_nolock(&aclk_vepu, &general_pll_clk);
	//clk_set_parent_nolock(&aclk_vdpu, &general_pll_clk);

	clk_set_rate_nolock(&aclk_vepu, 300 * MHZ);
	clk_set_rate_nolock(&aclk_vdpu, 300 * MHZ);
	//gpu auto sel
	clk_set_parent_nolock(&aclk_gpu, &general_pll_clk);
	clk_set_rate_nolock(&aclk_gpu, 200 * MHZ);
	
	clk_set_rate_nolock(&clk_uart0, 48000000);
	clk_set_rate_nolock(&clk_sdmmc, 24750000);
	clk_set_rate_nolock(&clk_sdio, 24750000);
}

static struct clk def_ops_clk = {
	.get_parent = clksel_get_parent,
	.set_parent = clksel_set_parent,
};

#ifdef CONFIG_PROC_FS
struct clk_dump_ops dump_ops;
#endif
void rk_dump_clock_info(void);
void __init _rk30_clock_data_init(unsigned long gpll, unsigned long cpll, int flags)
{
	struct clk_lookup *lk;

	if (soc_is_rk3188plus()) {
		arm_pll_clk.recalc = plus_plls_clk_recalc;
		ddr_pll_clk.recalc = plus_plls_clk_recalc;
		codec_pll_clk.recalc = plus_plls_clk_recalc;
		general_pll_clk.recalc = plus_plls_clk_recalc;

		arm_pll_clk.set_rate = plus_arm_pll_clk_set_rate;
		codec_pll_clk.set_rate = plus_cpll_clk_set_rate;
		general_pll_clk.set_rate = plus_gpll_clk_set_rate;
	}

	rk_efuse_init();
	pll_flag = rk_pll_flag();
	printk("CLKDATA_MSG: pll_flag = 0x%02x\n", pll_flag);

	if (0 != pll_flag) {
		CLKDATA_DBG("CPLL=%lu, GPLL=%lu;CPLL CAN NOT LOCK, SET CPLL BY PASS, USE GPLL REPLACE CPLL\n",
				cpll, gpll);
		codec_pll_clk.mode = NULL;
		cpll = 24 * MHZ;
		gpll = 891 * MHZ;
	}

	clk_register_dump_ops(&dump_ops);
	clk_register_default_ops_clk(&def_ops_clk);
	rk30_clock_flags = flags;
	for (lk = clks; lk < clks + ARRAY_SIZE(clks); lk++) {
#ifdef RK30_CLK_OFFBOARD_TEST
		rk30_clkdev_add(lk);
#else
		clkdev_add(lk);
#endif
		clk_register(lk->clk);
	}
	
	div_clk_for_pll_init();
	clk_recalculate_root_clocks_nolock();

	loops_per_jiffy = CLK_LOOPS_RECALC(arm_pll_clk.rate);

	/*
	 * Only enable those clocks we will need, let the drivers
	 * enable other clocks as necessary
	 */

	rk30_init_enable_clocks();
#if 0
	// print loader config
	CLKDATA_DBG("%s clks register dbg start\n", __func__);
	rk_dump_clock_info();
	rk30_clk_dump_regs();

	CLKDATA_DBG("%s clks register dbg end\n", __func__);
#endif
	/*
	 * Disable any unused clocks left on by the bootloader
	 */
	//clk_disable_unused();
	rk30_clock_common_init(gpll, cpll);
	preset_lpj = loops_per_jiffy;


	//gpio6_b7
	//regfile_writel(0xc0004000,0x10c);
	//cru_writel(0x07000000,CRU_MISC_CON);

}
extern int rk3188_dvfs_init(void);

void __init rk30_clock_data_init(unsigned long gpll, unsigned long cpll, u32 flags)
{
	CLKDATA_DBG("clock: gpll %lu cpll %lu flags 0x%x con2 0x%x/0x%x\n", 
			gpll, cpll, flags, cru_readl(PLL_CONS(DPLL_ID, 2)), cru_readl(PLL_CONS(CPLL_ID, 2)));
	_rk30_clock_data_init(gpll, cpll, flags);
	rk3188_dvfs_init();
}
#define STR_UBOOT_DISPLAY	"fastboot"
static int __init bootloader_setup(char *str)
{
	if (0 == strncmp(str, STR_UBOOT_DISPLAY, strlen(STR_UBOOT_DISPLAY))) {
		printk("CLKDATA_MSG: get uboot display\n");
		flag_uboot_display = 1;
	}
	return 0;
}
early_param("androidboot.bootloader", bootloader_setup);

int support_uboot_display(void)
{
	return flag_uboot_display;
}
/*
 * You can override arm_clk rate with armclk= cmdline option.
 */
static int __init armclk_setup(char *str)
{
	get_option(&str, &armclk);

	if (!armclk)
		return 0;
	if (armclk < 10000)
		armclk *= MHZ;
	//clk_set_rate_nolock(&arm_pll_clk, armclk);
	return 0;
}
#ifndef RK30_CLK_OFFBOARD_TEST
early_param("armclk", armclk_setup);
#endif


static void rk_dump_clock(struct clk *clk, int deep, const struct list_head *root_clocks)
{
	struct clk *ck;
	int i;
	unsigned long rate = clk->rate;
	//CLKDATA_DBG("dump_clock %s\n",clk->name);
	for (i = 0; i < deep; i++)
		printk("    ");

	printk("%-11s ", clk->name);
#ifndef RK30_CLK_OFFBOARD_TEST
	if (clk->flags & IS_PD) {
		printk("%s ", pmu_power_domain_is_on(clk->gate_idx) ? "on " : "off");
	}
#endif
	if ((clk->mode == gate_mode) && (clk->gate_idx < CLK_GATE_MAX)) {
		int idx = clk->gate_idx;
		u32 v;
		v = cru_readl(CLK_GATE_CLKID_CONS(idx)) & ((0x1) << (idx % 16));
		printk("%s ", v ? "off" : "on ");
	}

	if (clk->pll) {
		u32 pll_mode;
		u32 pll_id = clk->pll->id;
		pll_mode = cru_readl(CRU_MODE_CON)&PLL_MODE_MSK(pll_id);
		if (pll_mode == (PLL_MODE_SLOW(pll_id) & PLL_MODE_MSK(pll_id)))
			printk("slow   ");
		else if (pll_mode == (PLL_MODE_NORM(pll_id) & PLL_MODE_MSK(pll_id)))
			printk("normal ");
		else if (pll_mode == (PLL_MODE_DEEP(pll_id) & PLL_MODE_MSK(pll_id)))
			printk("deep   ");

		if(cru_readl(PLL_CONS(pll_id, 3)) & PLL_BYPASS)
			printk("bypass ");
	} else if(clk == &clk_ddr) {
		rate = clk->recalc(clk);
	}

	if (rate >= MHZ) {
		if (rate % MHZ)
			printk("%ld.%06ld MHz", rate / MHZ, rate % MHZ);
		else
			printk("%ld MHz", rate / MHZ);
	} else if (rate >= KHZ) {
		if (rate % KHZ)
			printk("%ld.%03ld KHz", rate / KHZ, rate % KHZ);
		else
			printk("%ld KHz", rate / KHZ);
	} else {
		printk("%ld Hz", rate);
	}

	printk(" usecount = %d", clk->usecount);

	if (clk->parent)
		printk(" parent = %s", clk->parent->name);

	printk("\n");

	list_for_each_entry(ck, root_clocks, node) {
		if (ck->parent == clk)
			rk_dump_clock(ck, deep + 1, root_clocks);
	}
}

#if 1
struct list_head *get_rk_clocks_head(void);

void rk_dump_clock_info(void)
{
	struct clk* clk;
	list_for_each_entry(clk, get_rk_clocks_head(), node) {
		if (!clk->parent)
		rk_dump_clock(clk, 0,get_rk_clocks_head());
	}
}
#endif

#ifdef CONFIG_PROC_FS

static void dump_clock(struct seq_file *s, struct clk *clk, int deep, const struct list_head *root_clocks)
{
	struct clk *ck;
	int i;
	unsigned long rate = clk->rate;
	//CLKDATA_DBG("dump_clock %s\n",clk->name);
	for (i = 0; i < deep; i++)
		seq_printf(s, "    ");

	seq_printf(s, "%-11s ", clk->name);
#ifndef RK30_CLK_OFFBOARD_TEST
	if (clk->flags & IS_PD) {
		seq_printf(s, "%s ", pmu_power_domain_is_on(clk->gate_idx) ? "on " : "off");
	}
#endif
	if ((clk->mode == gate_mode) && (clk->gate_idx < CLK_GATE_MAX)) {
		int idx = clk->gate_idx;
		u32 v;
		v = cru_readl(CLK_GATE_CLKID_CONS(idx)) & ((0x1) << (idx % 16));
		seq_printf(s, "%s ", v ? "off" : "on ");
	}

	if (clk->pll) {
		u32 pll_mode;
		u32 pll_id = clk->pll->id;
		pll_mode = cru_readl(CRU_MODE_CON)&PLL_MODE_MSK(pll_id);
		if (pll_mode == (PLL_MODE_SLOW(pll_id) & PLL_MODE_MSK(pll_id)))
			seq_printf(s, "slow   ");
		else if (pll_mode == (PLL_MODE_NORM(pll_id) & PLL_MODE_MSK(pll_id)))
			seq_printf(s, "normal ");
		else if (pll_mode == (PLL_MODE_DEEP(pll_id) & PLL_MODE_MSK(pll_id)))
			seq_printf(s, "deep   ");

		if(cru_readl(PLL_CONS(pll_id, 3)) & PLL_BYPASS)
			seq_printf(s, "bypass ");
	} else if(clk == &clk_ddr) {
		rate = clk->recalc(clk);
	}

	if (rate >= MHZ) {
		if (rate % MHZ)
			seq_printf(s, "%ld.%06ld MHz", rate / MHZ, rate % MHZ);
		else
			seq_printf(s, "%ld MHz", rate / MHZ);
	} else if (rate >= KHZ) {
		if (rate % KHZ)
			seq_printf(s, "%ld.%03ld KHz", rate / KHZ, rate % KHZ);
		else
			seq_printf(s, "%ld KHz", rate / KHZ);
	} else {
		seq_printf(s, "%ld Hz", rate);
	}

	seq_printf(s, " usecount = %d", clk->usecount);

	if (clk->parent)
		seq_printf(s, " parent = %s", clk->parent->name);

	seq_printf(s, "\n");

	list_for_each_entry(ck, root_clocks, node) {
		if (ck->parent == clk)
			dump_clock(s, ck, deep + 1, root_clocks);
	}
}

static void dump_regs(struct seq_file *s)
{
	int i = 0;
	seq_printf(s, "\nPLL(id=0 apll,id=1,dpll,id=2,cpll,id=3 cpll)\n");
	seq_printf(s, "\nPLLRegisters:\n");
	for(i = 0; i < END_PLL_ID; i++) {
		seq_printf(s, "pll%d        :cons:%x,%x,%x,%x\n", i,
				cru_readl(PLL_CONS(i, 0)),
				cru_readl(PLL_CONS(i, 1)),
				cru_readl(PLL_CONS(i, 2)),
				cru_readl(PLL_CONS(i, 3))
			  );
	}
	seq_printf(s, "MODE        :%x\n", cru_readl(CRU_MODE_CON));

	for(i = 0; i < CRU_CLKSELS_CON_CNT; i++) {
		seq_printf(s, "CLKSEL%d 	   :%x\n", i, cru_readl(CRU_CLKSELS_CON(i)));
	}
	for(i = 0; i < CRU_CLKGATES_CON_CNT; i++) {
		seq_printf(s, "CLKGATE%d 	  :%x\n", i, cru_readl(CRU_CLKGATES_CON(i)));
	}
	seq_printf(s, "GLB_SRST_FST:%x\n", cru_readl(CRU_GLB_SRST_FST));
	seq_printf(s, "GLB_SRST_SND:%x\n", cru_readl(CRU_GLB_SRST_SND));

	for(i = 0; i < CRU_SOFTRSTS_CON_CNT; i++) {
		seq_printf(s, "CLKGATE%d 	  :%x\n", i, cru_readl(CRU_SOFTRSTS_CON(i)));
	}
	seq_printf(s, "CRU MISC    :%x\n", cru_readl(CRU_MISC_CON));
	seq_printf(s, "GLB_CNT_TH  :%x\n", cru_readl(CRU_GLB_CNT_TH));

}

void rk30_clk_dump_regs(void)
{
	int i = 0;
	printk("\nPLL(id=0 apll,id=1,dpll,id=2,cpll,id=3 cpll)\n");
	printk("\nPLLRegisters:\n");
	for(i = 0; i < END_PLL_ID; i++) {
		printk("pll%d        :cons:%x,%x,%x,%x\n", i,
				cru_readl(PLL_CONS(i, 0)),
				cru_readl(PLL_CONS(i, 1)),
				cru_readl(PLL_CONS(i, 2)),
				cru_readl(PLL_CONS(i, 3))
		      );
	}
	printk("MODE        :%x\n", cru_readl(CRU_MODE_CON));

	for(i = 0; i < CRU_CLKSELS_CON_CNT; i++) {
		printk("CLKSEL%d 	   :%x\n", i, cru_readl(CRU_CLKSELS_CON(i)));
	}
	for(i = 0; i < CRU_CLKGATES_CON_CNT; i++) {
		printk("CLKGATE%d 	  :%x\n", i, cru_readl(CRU_CLKGATES_CON(i)));
	}
	printk("GLB_SRST_FST:%x\n", cru_readl(CRU_GLB_SRST_FST));
	printk("GLB_SRST_SND:%x\n", cru_readl(CRU_GLB_SRST_SND));

	for(i = 0; i < CRU_SOFTRSTS_CON_CNT; i++) {
		printk("SOFTRST%d 	  :%x\n", i, cru_readl(CRU_SOFTRSTS_CON(i)));
	}
	printk("CRU MISC    :%x\n", cru_readl(CRU_MISC_CON));
	printk("GLB_CNT_TH  :%x\n", cru_readl(CRU_GLB_CNT_TH));

}


#ifdef CONFIG_PROC_FS
static void dump_clock(struct seq_file *s, struct clk *clk, int deep, const struct list_head *root_clocks);
struct clk_dump_ops dump_ops = {
	.dump_clk = dump_clock,
	.dump_regs = dump_regs,
};
#endif


#endif /* CONFIG_PROC_FS */




#ifdef RK30_CLK_OFFBOARD_TEST
struct clk *test_get_parent(struct clk *clk) {
	return clk->parent;
}

void i2s_test(void)
{
	struct clk *i2s_clk = &clk_i2s0;

	clk_enable_nolock(i2s_clk);

	clk_set_rate_nolock(i2s_clk, 12288000);
	printk("int %s parent is %s\n", i2s_clk->name, test_get_parent(i2s_clk)->name);
	clk_set_rate_nolock(i2s_clk, 297 * MHZ / 2);
	printk("int%s parent is %s\n", i2s_clk->name, test_get_parent(i2s_clk)->name);
	clk_set_rate_nolock(i2s_clk, 12 * MHZ);
	printk("int%s parent is %s\n", i2s_clk->name, test_get_parent(i2s_clk)->name);

}

void uart_test(void)
{
	struct clk *uart_clk = &clk_uart0;

	clk_enable_nolock(uart_clk);

	clk_set_rate_nolock(uart_clk, 12288000);
	printk("int %s parent is %s\n", uart_clk->name, test_get_parent(uart_clk)->name);
	clk_set_rate_nolock(uart_clk, 297 * MHZ / 2);
	printk("int%s parent is %s\n", uart_clk->name, test_get_parent(uart_clk)->name);
	clk_set_rate_nolock(uart_clk, 12 * MHZ);
	printk("int%s parent is %s\n", uart_clk->name, test_get_parent(uart_clk)->name);

}
void hsadc_test(void)
{
	struct clk *hsadc_clk = &clk_hsadc;

	printk("******************hsadc_test**********************\n");
	clk_enable_nolock(hsadc_clk);

	clk_set_rate_nolock(hsadc_clk, 12288000);
	printk("****end %s parent is %s\n", hsadc_clk->name, test_get_parent(hsadc_clk)->name);


	clk_set_rate_nolock(hsadc_clk, 297 * MHZ / 2);
	printk("****end %s parent is %s\n", hsadc_clk->name, test_get_parent(hsadc_clk)->name);

	clk_set_rate_nolock(hsadc_clk, 300 * MHZ / 2);

	clk_set_rate_nolock(hsadc_clk, 296 * MHZ / 2);

	printk("******************hsadc out clock**********************\n");

	clk_set_parent_nolock(hsadc_clk, &clk_hsadc_ext);
	printk("****end %s parent is %s\n", hsadc_clk->name, test_get_parent(hsadc_clk)->name);
	clk_set_rate_nolock(hsadc_clk, 297 * MHZ / 2);
	printk("****end %s parent is %s\n", hsadc_clk->name, test_get_parent(hsadc_clk)->name);



}

static void __init rk30_clock_test_init(unsigned long ppll_rate)
{
	//arm
	printk("*********arm_pll_clk***********\n");
	clk_set_rate_nolock(&arm_pll_clk, 816 * MHZ);

	printk("*********set clk_core parent***********\n");
	clk_set_parent_nolock(&clk_core, &arm_pll_clk);
	clk_set_rate_nolock(&clk_core, 504 * MHZ);

	//general
	printk("*********general_pll_clk***********\n");
	clk_set_rate_nolock(&general_pll_clk, ppll_rate);

	//code pll
	printk("*********codec_pll_clk***********\n");
	clk_set_rate_nolock(&codec_pll_clk, 600 * MHZ);


	printk("*********periph_clk_set_init***********\n");
	clk_set_parent_nolock(&aclk_periph, &general_pll_clk);
	periph_clk_set_init();

#if 0 //
	clk_set_parent_nolock(&clk_i2s_pll, &codec_pll_clk);
#else
	printk("*********clk i2s***********\n");
	clk_set_parent_nolock(&clk_i2s_pll, &general_pll_clk);
	printk("common %s parent is %s\n", clk_i2s_pll.name, test_get_parent(&clk_i2s_pll)->name);
	i2s_test();
#endif
	// spi
	clk_enable_nolock(&clk_spi0);
	clk_set_rate_nolock(&clk_spi0, 30 * MHZ);
	printk("common %s parent is %s\n", clk_spi0.name, test_get_parent(&clk_spi0)->name);
	//saradc
	clk_enable_nolock(&clk_saradc);
	clk_set_rate_nolock(&clk_saradc, 6 * MHZ);
	printk("common %s parent is %s\n", clk_saradc.name, test_get_parent(&clk_saradc)->name);
	//sdio
	clk_enable_nolock(&clk_sdio);
	clk_set_rate_nolock(&clk_sdio, 50 * MHZ);
	printk("common %s parent is %s\n", clk_sdio.name, test_get_parent(&clk_sdio)->name);
	// uart
	clk_set_parent_nolock(&clk_uart_pll, &general_pll_clk);
	uart_test();
	//mac
	printk("*********mac***********\n");

	clk_set_parent_nolock(&clk_mac_pll_div, &general_pll_clk);
	printk("common %s parent is %s\n", clk_mac_pll_div.name, test_get_parent(&clk_mac_pll_div)->name);

	//clk_set_parent_nolock(&clk_mac_ref, &clk_mac_pll_div);
	clk_set_rate_nolock(&clk_mac_ref, 50 * MHZ);
	printk("common %s parent is %s\n", clk_mac_ref.name, test_get_parent(&clk_mac_ref)->name);

	printk("*********mac mii set***********\n");
	clk_set_parent_nolock(&clk_mac_ref, &rmii_clkin);
	clk_set_rate_nolock(&clk_mac_ref, 20 * MHZ);
	printk("common %s parent is %s\n", clk_mac_ref.name, test_get_parent(&clk_mac_ref)->name);
	//hsadc
	printk("*********hsadc 1***********\n");
	//auto pll
	hsadc_test();
	//lcdc
	clk_enable_nolock(&dclk_lcdc0);

	clk_set_rate_nolock(&dclk_lcdc0, 60 * MHZ);
	clk_set_rate_nolock(&dclk_lcdc0, 27 * MHZ);

	//cif
	clk_enable_nolock(&cif0_out);

	clk_set_parent_nolock(&cif_out_pll, &general_pll_clk);
	printk("common %s parent is %s\n", cif_out_pll.name, test_get_parent(&cif_out_pll)->name);

	clk_set_rate_nolock(&cif0_out, 60 * MHZ);
	printk("common %s parent is %s\n", cif0_out.name, test_get_parent(&cif0_out)->name);

	clk_set_rate_nolock(&cif0_out, 24 * MHZ);
	printk("common %s parent is %s\n", cif0_out.name, test_get_parent(&cif0_out)->name);
	//cif_in
	clk_enable_nolock(&cif0_in);
	clk_set_rate_nolock(&cif0_in, 24 * MHZ);
	//axi lcdc
	clk_enable_nolock(&aclk_lcdc0);
	clk_set_rate_nolock(&aclk_lcdc0, 150 * MHZ);
	printk("common %s parent is %s\n", aclk_lcdc0.name, test_get_parent(&aclk_lcdc0)->name);
	//axi vepu
	clk_enable_nolock(&aclk_vepu);
	clk_set_rate_nolock(&aclk_vepu, 300 * MHZ);
	printk("common %s parent is %s\n", aclk_vepu.name, test_get_parent(&aclk_vepu)->name);

	clk_set_rate_nolock(&hclk_vepu, 300 * MHZ);
	printk("common %s parent is %s\n", hclk_vepu.name, test_get_parent(&hclk_vepu)->name);

	printk("test end\n");

	/* arm pll
	   clk_set_rate_nolock(&arm_pll_clk, armclk);
	   clk_set_rate_nolock(&clk_core,	armclk);//pll:core =1:1
	   */
	//
	//clk_set_rate_nolock(&codec_pll_clk, ppll_rate*2);
	//
	//clk_set_rate_nolock(&aclk_vepu, 300 * MHZ);
	//clk_set_rate_nolock(&clk_gpu, 300 * MHZ);

}





static LIST_HEAD(rk30_clocks);
static DEFINE_MUTEX(rk30_clocks_mutex);

static inline int __rk30clk_get(struct clk *clk)
{
	return 1;
}
void rk30_clkdev_add(struct clk_lookup *cl)
{
	mutex_lock(&rk30_clocks_mutex);
	list_add_tail(&cl->node, &rk30_clocks);
	mutex_unlock(&rk30_clocks_mutex);
}
static struct clk_lookup *rk30_clk_find(const char *dev_id, const char *con_id) {
	struct clk_lookup *p, *cl = NULL;
	int match, best = 0;

	list_for_each_entry(p, &rk30_clocks, node) {
		match = 0;
		if (p->dev_id) {
			if (!dev_id || strcmp(p->dev_id, dev_id))
				continue;
			match += 2;
		}
		if (p->con_id) {
			if (!con_id || strcmp(p->con_id, con_id))
				continue;
			match += 1;
		}

		if (match > best) {
			cl = p;
			if (match != 3)
				best = match;
			else
				break;
		}
	}
	return cl;
}

struct clk *rk30_clk_get_sys(const char *dev_id, const char *con_id) {
	struct clk_lookup *cl;

	mutex_lock(&rk30_clocks_mutex);
	cl = rk30_clk_find(dev_id, con_id);
	if (cl && !__rk30clk_get(cl->clk))
		cl = NULL;
	mutex_unlock(&rk30_clocks_mutex);

	return cl ? cl->clk : ERR_PTR(-ENOENT);
}
//EXPORT_SYMBOL(rk30_clk_get_sys);

struct clk *rk30_clk_get(struct device *dev, const char *con_id) {
	const char *dev_id = dev ? dev_name(dev) : NULL;
	return rk30_clk_get_sys(dev_id, con_id);
}
//EXPORT_SYMBOL(rk30_clk_get);


int rk30_clk_set_rate(struct clk *clk, unsigned long rate);

void rk30_clocks_test(void)
{
	struct clk *test_gpll;
	test_gpll = rk30_clk_get(NULL, "general_pll");
	if(test_gpll) {
		rk30_clk_set_rate(test_gpll, 297 * 2 * MHZ);
		printk("gpll rate=%lu\n", test_gpll->rate);
	}
	//while(1);
}

void __init rk30_clock_init_test(void)
{

	rk30_clock_init(periph_pll_297mhz, codec_pll_360mhz, max_i2s_12288khz);
	//while(1);
}


#endif


