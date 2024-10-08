// SPDX-License-Identifier: GPL-2.0+
/*
 * Motorcomm 8531 PHY driver. (From mainline U-Boot v2023.10)
 *
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <phy.h>
#include <linux/bitfield.h>

#define PHY_ID_YT8531				0x4f51e91b
#define PHY_ID_MASK				GENMASK(31, 0)

/* Extended Register's Address Offset Register */
#define YTPHY_PAGE_SELECT			0x1E

/* Extended Register's Data Register */
#define YTPHY_PAGE_DATA			0x1F

#define YTPHY_SYNCE_CFG_REG			0xA012

#define YTPHY_DTS_OUTPUT_CLK_DIS		0
#define YTPHY_DTS_OUTPUT_CLK_25M		25000000
#define YTPHY_DTS_OUTPUT_CLK_125M		125000000

#define YT8531_SCR_SYNCE_ENABLE		BIT(6)
/* 1b0 output 25m clock   *default*
 * 1b1 output 125m clock
 */
#define YT8531_SCR_CLK_FRE_SEL_125M		BIT(4)
#define YT8531_SCR_CLK_SRC_MASK		GENMASK(3, 1)
#define YT8531_SCR_CLK_SRC_PLL_125M		0
#define YT8531_SCR_CLK_SRC_UTP_RX		1
#define YT8531_SCR_CLK_SRC_SDS_RX		2
#define YT8531_SCR_CLK_SRC_CLOCK_FROM_DIGITAL	3
#define YT8531_SCR_CLK_SRC_REF_25M		4
#define YT8531_SCR_CLK_SRC_SSC_25M		5

/* 1b0 use original tx_clk_rgmii  *default*
 * 1b1 use inverted tx_clk_rgmii.
 */
#define YT8531_RC1R_TX_CLK_SEL_INVERTED	BIT(14)
#define YT8531_RC1R_RX_DELAY_MASK		GENMASK(13, 10)
#define YT8531_RC1R_FE_TX_DELAY_MASK		GENMASK(7, 4)
#define YT8531_RC1R_GE_TX_DELAY_MASK		GENMASK(3, 0)
#define YT8531_RC1R_RGMII_0_000_NS		0
#define YT8531_RC1R_RGMII_0_150_NS		1
#define YT8531_RC1R_RGMII_0_300_NS		2
#define YT8531_RC1R_RGMII_0_450_NS		3
#define YT8531_RC1R_RGMII_0_600_NS		4
#define YT8531_RC1R_RGMII_0_750_NS		5
#define YT8531_RC1R_RGMII_0_900_NS		6
#define YT8531_RC1R_RGMII_1_050_NS		7
#define YT8531_RC1R_RGMII_1_200_NS		8
#define YT8531_RC1R_RGMII_1_350_NS		9
#define YT8531_RC1R_RGMII_1_500_NS		10
#define YT8531_RC1R_RGMII_1_650_NS		11
#define YT8531_RC1R_RGMII_1_800_NS		12
#define YT8531_RC1R_RGMII_1_950_NS		13
#define YT8531_RC1R_RGMII_2_100_NS		14
#define YT8531_RC1R_RGMII_2_250_NS		15

/* Phy gmii clock gating Register */
#define YT8531_CLOCK_GATING_REG		0xC
#define YT8531_CGR_RX_CLK_EN			BIT(12)

/* Specific Status Register */
#define YTPHY_SPECIFIC_STATUS_REG		0x11
#define YTPHY_DUPLEX_MASK			BIT(13)
#define YTPHY_DUPLEX_SHIFT			13
#define YTPHY_SPEED_MODE_MASK			GENMASK(15, 14)
#define YTPHY_SPEED_MODE_SHIFT			14

#define YT8531_EXTREG_SLEEP_CONTROL1_REG	0x27
#define YT8531_ESC1R_SLEEP_SW			BIT(15)
#define YT8531_ESC1R_PLLON_SLP			BIT(14)

#define YT8531_RGMII_CONFIG1_REG		0xA003

#define YT8531_CHIP_CONFIG_REG			0xA001
#define YT8531_CCR_SW_RST			BIT(15)
/* 1b0 disable 1.9ns rxc clock delay  *default*
 * 1b1 enable 1.9ns rxc clock delay
 */
#define YT8531_CCR_RXC_DLY_EN			BIT(8)
#define YT8531_CCR_RXC_DLY_1_900_NS		1900

/* bits in struct ytphy_plat_priv->flag */
#define TX_CLK_ADJ_ENABLED			BIT(0)
#define AUTO_SLEEP_DISABLED			BIT(1)
#define KEEP_PLL_ENABLED			BIT(2)
#define TX_CLK_10_INVERTED			BIT(3)
#define TX_CLK_100_INVERTED			BIT(4)
#define TX_CLK_1000_INVERTED			BIT(5)

struct ytphy_plat_priv {
	u32 rx_delay_ps;
	u32 tx_delay_ps;
	u32 clk_out_frequency;
	u32 flag;
};

/**
 * struct ytphy_cfg_reg_map - map a config value to a register value
 * @cfg: value in device configuration
 * @reg: value in the register
 */
struct ytphy_cfg_reg_map {
	u32 cfg;
	u32 reg;
};

static const struct ytphy_cfg_reg_map ytphy_rgmii_delays[] = {
	/* for tx delay / rx delay with YT8531_CCR_RXC_DLY_EN is not set. */
	{ 0,	YT8531_RC1R_RGMII_0_000_NS },
	{ 150,	YT8531_RC1R_RGMII_0_150_NS },
	{ 300,	YT8531_RC1R_RGMII_0_300_NS },
	{ 450,	YT8531_RC1R_RGMII_0_450_NS },
	{ 600,	YT8531_RC1R_RGMII_0_600_NS },
	{ 750,	YT8531_RC1R_RGMII_0_750_NS },
	{ 900,	YT8531_RC1R_RGMII_0_900_NS },
	{ 1050,	YT8531_RC1R_RGMII_1_050_NS },
	{ 1200,	YT8531_RC1R_RGMII_1_200_NS },
	{ 1350,	YT8531_RC1R_RGMII_1_350_NS },
	{ 1500,	YT8531_RC1R_RGMII_1_500_NS },
	{ 1650,	YT8531_RC1R_RGMII_1_650_NS },
	{ 1800,	YT8531_RC1R_RGMII_1_800_NS },
	{ 1950,	YT8531_RC1R_RGMII_1_950_NS },	/* default tx/rx delay */
	{ 2100,	YT8531_RC1R_RGMII_2_100_NS },
	{ 2250,	YT8531_RC1R_RGMII_2_250_NS },

	/* only for rx delay with YT8531_CCR_RXC_DLY_EN is set. */
	{ 0    + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_000_NS },
	{ 150  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_150_NS },
	{ 300  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_300_NS },
	{ 450  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_450_NS },
	{ 600  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_600_NS },
	{ 750  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_750_NS },
	{ 900  + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_0_900_NS },
	{ 1050 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_050_NS },
	{ 1200 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_200_NS },
	{ 1350 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_350_NS },
	{ 1500 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_500_NS },
	{ 1650 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_650_NS },
	{ 1800 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_800_NS },
	{ 1950 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_1_950_NS },
	{ 2100 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_2_100_NS },
	{ 2250 + YT8531_CCR_RXC_DLY_1_900_NS,	YT8531_RC1R_RGMII_2_250_NS }
};

static u32 ytphy_get_delay_reg_value(struct phy_device *phydev,
				     u32 val,
				     u16 *rxc_dly_en)
{
	int tb_size = ARRAY_SIZE(ytphy_rgmii_delays);
	int tb_size_half = tb_size / 2;
	int i;

	/* when rxc_dly_en is NULL, it is get the delay for tx, only half of
	 * tb_size is valid.
	 */
	if (!rxc_dly_en)
		tb_size = tb_size_half;

	for (i = 0; i < tb_size; i++) {
		if (ytphy_rgmii_delays[i].cfg == val) {
			if (rxc_dly_en && i < tb_size_half)
				*rxc_dly_en = 0;
			return ytphy_rgmii_delays[i].reg;
		}
	}

	pr_warn("Unsupported value %d, using default (%u)\n",
		val, YT8531_RC1R_RGMII_1_950_NS);

	/* when rxc_dly_en is not NULL, it is get the delay for rx.
	 * The rx default in dts and ytphy_rgmii_clk_delay_config is 1950 ps,
	 * so YT8531_CCR_RXC_DLY_EN should not be set.
	 */
	if (rxc_dly_en)
		*rxc_dly_en = 0;

	return YT8531_RC1R_RGMII_1_950_NS;
}

static int ytphy_modify_ext(struct phy_device *phydev, u16 regnum, u16 mask,
			    u16 set)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, YTPHY_PAGE_SELECT, regnum);
	if (ret < 0)
		return ret;

	return phy_modify(phydev, MDIO_DEVAD_NONE, YTPHY_PAGE_DATA, mask, set);
}

static int ytphy_rgmii_clk_delay_config(struct phy_device *phydev)
{
	struct ytphy_plat_priv	*priv = phydev->priv;
	u16 rxc_dly_en = YT8531_CCR_RXC_DLY_EN;
	u32 rx_reg, tx_reg;
	u16 mask, val = 0;
	int ret;

	rx_reg = ytphy_get_delay_reg_value(phydev, priv->rx_delay_ps,
					   &rxc_dly_en);
	tx_reg = ytphy_get_delay_reg_value(phydev, priv->tx_delay_ps,
					   NULL);

	switch (phydev->interface) {
	case PHY_INTERFACE_MODE_RGMII:
		rxc_dly_en = 0;
		break;
	case PHY_INTERFACE_MODE_RGMII_RXID:
		val |= FIELD_PREP(YT8531_RC1R_RX_DELAY_MASK, rx_reg);
		break;
	case PHY_INTERFACE_MODE_RGMII_TXID:
		rxc_dly_en = 0;
		val |= FIELD_PREP(YT8531_RC1R_GE_TX_DELAY_MASK, tx_reg);
		break;
	case PHY_INTERFACE_MODE_RGMII_ID:
		val |= FIELD_PREP(YT8531_RC1R_RX_DELAY_MASK, rx_reg) |
		       FIELD_PREP(YT8531_RC1R_GE_TX_DELAY_MASK, tx_reg);
		break;
	default: /* do not support other modes */
		return -EOPNOTSUPP;
	}

	ret = ytphy_modify_ext(phydev, YT8531_CHIP_CONFIG_REG,
			       YT8531_CCR_RXC_DLY_EN, rxc_dly_en);
	if (ret < 0)
		return ret;

	/* Generally, it is not necessary to adjust YT8531_RC1R_FE_TX_DELAY */
	mask = YT8531_RC1R_RX_DELAY_MASK | YT8531_RC1R_GE_TX_DELAY_MASK;
	return ytphy_modify_ext(phydev, YT8531_RGMII_CONFIG1_REG, mask, val);
}

static int yt8531_parse_status(struct phy_device *phydev)
{
	int val;
	int speed, speed_mode;

	val = phy_read(phydev, MDIO_DEVAD_NONE, YTPHY_SPECIFIC_STATUS_REG);
	if (val < 0)
		return val;

	speed_mode = (val & YTPHY_SPEED_MODE_MASK) >> YTPHY_SPEED_MODE_SHIFT;
	switch (speed_mode) {
	case 2:
		speed = SPEED_1000;
		break;
	case 1:
		speed = SPEED_100;
		break;
	default:
		speed = SPEED_10;
		break;
	}

	phydev->speed = speed;
	phydev->duplex = (val & YTPHY_DUPLEX_MASK) >> YTPHY_DUPLEX_SHIFT;

	return 0;
}

static int yt8531_startup(struct phy_device *phydev)
{
	struct ytphy_plat_priv	*priv = phydev->priv;
	u16 val = 0;
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	ret = yt8531_parse_status(phydev);
	if (ret)
		return ret;

	if (phydev->speed < 0)
		return -EINVAL;

	if (!(priv->flag & TX_CLK_ADJ_ENABLED))
		return 0;

	switch (phydev->speed) {
	case SPEED_1000:
		if (priv->flag & TX_CLK_1000_INVERTED)
			val = YT8531_RC1R_TX_CLK_SEL_INVERTED;
		break;
	case SPEED_100:
		if (priv->flag & TX_CLK_100_INVERTED)
			val = YT8531_RC1R_TX_CLK_SEL_INVERTED;
		break;
	case SPEED_10:
		if (priv->flag & TX_CLK_10_INVERTED)
			val = YT8531_RC1R_TX_CLK_SEL_INVERTED;
		break;
	default:
		printf("UNKNOWN SPEED\n");
		return -EINVAL;
	}

	ret = ytphy_modify_ext(phydev, YT8531_RGMII_CONFIG1_REG,
			       YT8531_RC1R_TX_CLK_SEL_INVERTED, val);
	if (ret < 0)
		pr_warn("Modify TX_CLK_SEL err:%d\n", ret);

	return 0;
}

static void ytphy_dt_parse(struct phy_device *phydev)
{
	struct ytphy_plat_priv	*priv = phydev->priv;

	priv->clk_out_frequency = ofnode_read_u32_default(phydev->node,
							  "motorcomm,clk-out-frequency-hz",
							  YTPHY_DTS_OUTPUT_CLK_125M);
	priv->rx_delay_ps = ofnode_read_u32_default(phydev->node,
						    "rx-internal-delay-ps",
						    YT8531_RC1R_RGMII_1_950_NS);
	priv->tx_delay_ps = ofnode_read_u32_default(phydev->node,
						    "tx-internal-delay-ps",
						    YT8531_RC1R_RGMII_1_950_NS);

	if (ofnode_read_bool(phydev->node, "motorcomm,auto-sleep-disabled"))
		priv->flag |= AUTO_SLEEP_DISABLED;

	if (ofnode_read_bool(phydev->node, "motorcomm,keep-pll-enabled"))
		priv->flag |= KEEP_PLL_ENABLED;

	if (ofnode_read_bool(phydev->node, "motorcomm,tx-clk-adj-enabled"))
		priv->flag |= TX_CLK_ADJ_ENABLED;

	if (ofnode_read_bool(phydev->node, "motorcomm,tx-clk-10-inverted"))
		priv->flag |= TX_CLK_10_INVERTED;

	if (ofnode_read_bool(phydev->node, "motorcomm,tx-clk-100-inverted"))
		priv->flag |= TX_CLK_100_INVERTED;

	if (ofnode_read_bool(phydev->node, "motorcomm,tx-clk-1000-inverted"))
		priv->flag |= TX_CLK_1000_INVERTED;
}

static int yt8531_config(struct phy_device *phydev)
{
	struct ytphy_plat_priv	*priv = phydev->priv;
	u16 mask, val;
	int ret;

	ret = genphy_config_aneg(phydev);
	if (ret < 0)
		return ret;

	ytphy_dt_parse(phydev);
	switch (priv->clk_out_frequency) {
	case YTPHY_DTS_OUTPUT_CLK_DIS:
		mask = YT8531_SCR_SYNCE_ENABLE;
		val = 0;
		break;
	case YTPHY_DTS_OUTPUT_CLK_25M:
		mask = YT8531_SCR_SYNCE_ENABLE | YT8531_SCR_CLK_SRC_MASK |
			   YT8531_SCR_CLK_FRE_SEL_125M;
		val = YT8531_SCR_SYNCE_ENABLE |
			  FIELD_PREP(YT8531_SCR_CLK_SRC_MASK,
				     YT8531_SCR_CLK_SRC_REF_25M);
		break;
	case YTPHY_DTS_OUTPUT_CLK_125M:
		mask = YT8531_SCR_SYNCE_ENABLE | YT8531_SCR_CLK_SRC_MASK |
			   YT8531_SCR_CLK_FRE_SEL_125M;
		val = YT8531_SCR_SYNCE_ENABLE | YT8531_SCR_CLK_FRE_SEL_125M |
			  FIELD_PREP(YT8531_SCR_CLK_SRC_MASK,
				     YT8531_SCR_CLK_SRC_PLL_125M);
		break;
	default:
		pr_warn("Freq err:%u\n", priv->clk_out_frequency);
		return -EINVAL;
	}

	ret = ytphy_modify_ext(phydev, YTPHY_SYNCE_CFG_REG, mask,
			       val);
	if (ret < 0)
		return ret;

	ret = ytphy_rgmii_clk_delay_config(phydev);
	if (ret < 0)
		return ret;

	if (priv->flag & AUTO_SLEEP_DISABLED) {
		/* disable auto sleep */
		ret = ytphy_modify_ext(phydev,
				       YT8531_EXTREG_SLEEP_CONTROL1_REG,
				       YT8531_ESC1R_SLEEP_SW, 0);
		if (ret < 0)
			return ret;
	}

	if (priv->flag & KEEP_PLL_ENABLED) {
		/* enable RXC clock when no wire plug */
		ret = ytphy_modify_ext(phydev,
				       YT8531_CLOCK_GATING_REG,
				       YT8531_CGR_RX_CLK_EN, 0);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int yt8531_probe(struct phy_device *phydev)
{
	struct ytphy_plat_priv	*priv;

	priv = calloc(1, sizeof(struct ytphy_plat_priv));
	if (!priv)
		return -ENOMEM;

	phydev->priv = priv;

	return 0;
}

static struct phy_driver yt8531_driver = {
	.name          = "YT8531 Gigabit Ethernet",
	.uid           = PHY_ID_YT8531,
	.mask          = PHY_ID_MASK,
	.features      = PHY_GBIT_FEATURES,
	.probe	       = &yt8531_probe,
	.config        = &yt8531_config,
	.startup       = &yt8531_startup,
	.shutdown      = &genphy_shutdown,
};

int phy_motorcomm_init(void)
{
	phy_register(&yt8531_driver);

	return 0;
}

