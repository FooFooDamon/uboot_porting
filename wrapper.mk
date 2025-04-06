# SPDX-License-Identifier: GPL-2.0

#
# Makefile wrapper for U-Boot porting project.
#
# Copyright (c) 2024 Man Hung-Coeng <udc577@126.com>
#

override undefine LAZY_CODING_MAKEFILES
LAZY_CODING_MAKEFILES := __ver__.mk u-boot.mk

ifeq ($(shell [ true $(foreach i, ${LAZY_CODING_MAKEFILES}, -a -s ${i}) ] && echo 1 || echo 0),0)

.PHONY: all help prepare

LAZY_CODING_URL ?= https://github.com/FooFooDamon/lazy_coding_skills

all help prepare:
	@for i in ${LAZY_CODING_MAKEFILES}; \
	do \
		mkdir -p $$(dirname $${i}); \
		[ -s $${i} ] || wget -c -O $${i} "${LAZY_CODING_URL}/raw/main/makefile/$$(basename $${i})"; \
	done
	@echo "~ ~ ~ Minimum preparation finished successfully ~ ~ ~"
	@echo "Re-run your command again to continue your work."

else

include $(word 1, ${LAZY_CODING_MAKEFILES})

ARCH ?= arm
CROSS_COMPILE ?= arm-linux-gnueabihf-
PKG_FILE ?= ./uboot-imx-rel_imx_4.1.15_2.1.0_ga.tar.gz
# -- Rule of URL --
# Example: GitHub
# URL prefix: https://github.com/<user>/<repo>
# Package suffix: tar.gz | zip
# Download by tag or release: <prefix>/archive/refs/tags/<tag>.<suffix>
# Download by branch: <prefix>/archive/refs/heads/<branch>.<suffix>
# Download by commit: <prefix>/archive/<full-commit-hash>.<suffix>
# See also: https://docs.github.com/en/repositories/working-with-files/using-files/downloading-source-code-archives
PKG_URL ?= https://github.com/nxp-imx/uboot-imx/archive/refs/tags/rel_imx_4.1.15_2.1.0_ga.tar.gz
INSTALL_DIR ?= ${HOME}/tftpd/imx6ullevk
DEFCONFIG ?= configs/mx6ull_14x14_evk_nand_defconfig
EXT_TARGETS +=
CUSTOM_FILES +=

include $(word 2, ${LAZY_CODING_MAKEFILES})

-include ../../rsync.priv.mk

endif

