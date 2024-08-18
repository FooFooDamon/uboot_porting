PKG_FILE ?= ./u-boot-orangepi-752ac3f2fdcfe9427ca8868d95025aacd48fc00b.tar.gz
#PKG_URL ?= https://github.com/orangepi-xunlong/u-boot-orangepi/archive/refs/heads/v2017.09-rk3588.tar.gz
PKG_URL ?= https://github.com/orangepi-xunlong/u-boot-orangepi/archive/752ac3f2fdcfe9427ca8868d95025aacd48fc00b.tar.gz
EXT_TARGETS +=
CUSTOM_FILES += tools/rockchip/boot_merger.c \
    drivers/net/phy/Kconfig \
    drivers/net/phy/Makefile \
    include/phy.h \
    drivers/net/phy/phy.c \
    drivers/net/phy/motorcomm.c

include ../common.mk
include ../../wrapper.mk

