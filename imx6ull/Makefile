# SPDX-License-Identifier: GPL-2.0

#
# Makefile wrapper for U-Boot.
#
# Copyright (c) 2024 Man Hung-Coeng <udc577@126.com>
#

ifeq ($(shell [ -s __ver__.mk -a -s u-boot.mk ] && echo 1 || echo 0),0)

LAZY_CODING_URL ?= https://github.com/FooFooDamon/lazy_coding_skills

.PHONY: all help init

all help init:
	@for i in __ver__.mk u-boot.mk; \
	do \
		[ -s $${i} ] || wget -c "${LAZY_CODING_URL}/raw/main/makefile/$${i}"; \
	done
	@echo "~ ~ ~ Initialization finished successfully ~ ~ ~"
	@echo "Re-run your command again to continue your work."

else

include __ver__.mk

PKG_FILE := ./uboot-imx-rel_imx_4.1.15_2.1.0_ga.tar.gz
PKG_URL := https://github.com/nxp-imx/uboot-imx/archive/refs/tags/rel_imx_4.1.15_2.1.0_ga.tar.gz
SRC_PARENT_DIR := ./_tmp_
INSTALL_DIR ?= ${HOME}/tftpd/imx6ullevk
DEFCONFIG := configs/mx6ull_14x14_evk_nand_defconfig
EXTRA_TARGETS :=
CUSTOM_FILES := ${DEFCONFIG} \
    arch/arm/cpu/armv7/start.S \
    arch/arm/cpu/armv7/soc.c \
    include/configs/mx6ullevk.h \
    board/freescale/mx6ullevk/mx6ullevk.c \
    drivers/net/phy/ti.c

include u-boot.mk

endif

