ARCH ?= arm
CROSS_COMPILE ?= arm-linux-gnueabihf-
PKG_FILE ?= ./uboot-imx-rel_imx_4.1.15_2.1.0_ga.tar.gz
PKG_URL ?= https://github.com/nxp-imx/uboot-imx/archive/refs/tags/rel_imx_4.1.15_2.1.0_ga.tar.gz
INSTALL_DIR ?= ${HOME}/tftpd/imx6ullevk
DEFCONFIG ?= configs/mx6ull_14x14_evk_nand_defconfig
EXT_TARGETS +=
CUSTOM_FILES += \
    arch/arm/cpu/armv7/start.S \
    arch/arm/cpu/armv7/soc.c \
    include/configs/mx6ullevk.h \
    board/freescale/mx6ullevk/mx6ullevk.c \
    drivers/net/phy/ti.c

include ../wrapper.mk

